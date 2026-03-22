#include <volk.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include "gfx.h"
#include "file_io.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---------------------------------------------------------------------------
// Macros / constants
// ---------------------------------------------------------------------------

#define COUNTOF(x) ((int)(sizeof(x) / sizeof((x)[0])))

#define VK_CHECK(expr)                                                          \
    do                                                                          \
    {                                                                           \
        VkResult _r = (expr);                                                   \
        if (_r != VK_SUCCESS)                                                   \
        {                                                                       \
            fprintf(stderr, "[Vulkan] Error %d at %s:%d -> %s\n",              \
                    _r, __FILE__, __LINE__, #expr);                             \
            abort();                                                            \
        }                                                                       \
    } while (0)

enum
{
    FRAMES_IN_FLIGHT     = 2,
    MAX_SWAPCHAIN_IMAGES = 8,
    MAX_SPRITES          = 4096,
    MAX_TEXTURES         = 4096,
    SPRITE_SIZE          = 48,
    SSBO_SIZE            = MAX_SPRITES * SPRITE_SIZE,
};

// ---------------------------------------------------------------------------
// GPU sprite struct (must match shader)
// ---------------------------------------------------------------------------

typedef struct gpu_sprite
{
    float position[2];
    float size[2];
    float uv_offset[2];
    float uv_scale[2];
    uint32_t texture_index;
    uint32_t color;
    uint32_t _pad[2];
} gpu_sprite;

_Static_assert(sizeof(gpu_sprite) == SPRITE_SIZE, "gpu_sprite must be 48 bytes");

// ---------------------------------------------------------------------------
// Internal state
// ---------------------------------------------------------------------------

typedef struct frame_sync
{
    VkFence in_flight;
    VkSemaphore image_acquired;
    VkCommandBuffer cmd;
} frame_sync;

static struct
{
    // Window
    SDL_Window* window;

    // Vulkan core
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkSurfaceKHR surface;
    VkPhysicalDevice physical_device;
    VkDevice device;

    // Queues
    uint32_t graphics_family;
    uint32_t present_family;
    VkQueue graphics_queue;
    VkQueue present_queue;

    // Swapchain
    VkSwapchainKHR swapchain;
    VkFormat swapchain_format;
    VkExtent2D swapchain_extent;
    VkImage swapchain_images[MAX_SWAPCHAIN_IMAGES];
    VkImageView swapchain_views[MAX_SWAPCHAIN_IMAGES];
    VkSemaphore render_finished[MAX_SWAPCHAIN_IMAGES];
    uint32_t swapchain_image_count;

    // Commands & sync
    VkCommandPool command_pool;
    frame_sync frames[FRAMES_IN_FLIGHT];
    uint32_t frame_index;
    uint32_t image_index;

    // Sprite pipeline
    VkDescriptorPool desc_pool;
    VkDescriptorSetLayout ssbo_set_layout;    // set 0: sprite SSBO
    VkDescriptorSetLayout texture_set_layout; // set 1: bindless textures
    VkPipelineLayout sprite_layout;
    VkPipeline sprite_pipeline;
    VkSampler nearest_sampler;

    // Per-frame sprite SSBO (double-buffered)
    struct
    {
        VkBuffer buffers[FRAMES_IN_FLIGHT];
        VkDeviceMemory memory[FRAMES_IN_FLIGHT];
        void* mapped[FRAMES_IN_FLIGHT];
        VkDescriptorSet desc_sets[FRAMES_IN_FLIGHT]; // set 0 per frame
        gpu_sprite batch[MAX_SPRITES];
        uint32_t count;
    } sprites;

    // Bindless textures
    struct
    {
        VkImage images[MAX_TEXTURES];
        VkDeviceMemory mem[MAX_TEXTURES];
        VkImageView views[MAX_TEXTURES];
        uint32_t count;
        VkDescriptorSet desc_set; // set 1, shared
    } textures;

    // Camera
    float camera_x, camera_y;

    // Input
    gfx_input input;

    // State flags
    bool need_resize;
    bool quit_requested;
    bool frame_active;

    uint32_t width;
    uint32_t height;
    bool validation;
} g;

// ---------------------------------------------------------------------------
// Debug messenger
// ---------------------------------------------------------------------------

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT types,
    const VkDebugUtilsMessengerCallbackDataEXT* data,
    void* user_data)
{
    (void)types;
    (void)user_data;
    const char* sev =
        (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)   ? "ERROR" :
        (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) ? "WARN " :
        (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)    ? "INFO " :
                                                                       "VERB ";
    fprintf(stderr, "[VK/%s] %s\n", sev, data->pMessage);
    return VK_FALSE;
}

// ---------------------------------------------------------------------------
// Instance + surface
// ---------------------------------------------------------------------------

static bool create_instance(void)
{
    VK_CHECK(volkInitialize());
    if (!vkCreateInstance)
    {
        fprintf(stderr, "[gfx] volkInitialize failed to load vkCreateInstance.\n");
        return false;
    }

    uint32_t sdl_ext_count = 0;
    const char* const* sdl_exts = SDL_Vulkan_GetInstanceExtensions(&sdl_ext_count);
    if (!sdl_exts || sdl_ext_count == 0)
    {
        fprintf(stderr, "[gfx] SDL_Vulkan_GetInstanceExtensions failed: %s\n", SDL_GetError());
        return false;
    }

    uint32_t ext_count = sdl_ext_count + (g.validation ? 1u : 0u);
    const char** exts = malloc(sizeof(char*) * ext_count);
    for (uint32_t i = 0; i < sdl_ext_count; ++i)
        exts[i] = sdl_exts[i];
    if (g.validation)
        exts[sdl_ext_count] = "VK_EXT_debug_utils";

    const char* layers[] = {"VK_LAYER_KHRONOS_validation"};

    VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Open Battlecry",
        .applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 0),
        .pEngineName = "Open Battlecry",
        .engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0),
        .apiVersion = VK_API_VERSION_1_4,
    };

    VkInstanceCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
        .enabledExtensionCount = ext_count,
        .ppEnabledExtensionNames = exts,
        .enabledLayerCount = g.validation ? COUNTOF(layers) : 0,
        .ppEnabledLayerNames = g.validation ? layers : NULL,
    };

    VkResult r = vkCreateInstance(&ci, NULL, &g.instance);
    free(exts);
    if (r != VK_SUCCESS)
    {
        fprintf(stderr, "[gfx] vkCreateInstance failed: %d\n", r);
        return false;
    }

    volkLoadInstance(g.instance);

    if (!SDL_Vulkan_CreateSurface(g.window, g.instance, NULL, &g.surface))
    {
        fprintf(stderr, "[gfx] SDL_Vulkan_CreateSurface failed: %s\n", SDL_GetError());
        return false;
    }

    if (g.validation)
    {
        VkDebugUtilsMessengerCreateInfoEXT dbg_ci = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = debug_callback,
        };
        VK_CHECK(vkCreateDebugUtilsMessengerEXT(g.instance, &dbg_ci, NULL, &g.debug_messenger));
    }

    return true;
}

// ---------------------------------------------------------------------------
// Physical device selection
// ---------------------------------------------------------------------------

static bool pick_physical_device(void)
{
    uint32_t count = 0;
    VK_CHECK(vkEnumeratePhysicalDevices(g.instance, &count, NULL));
    if (count == 0) return false;

    VkPhysicalDevice* devs = malloc(sizeof(VkPhysicalDevice) * count);
    VK_CHECK(vkEnumeratePhysicalDevices(g.instance, &count, devs));

    for (uint32_t i = 0; i < count; ++i)
    {
        VkPhysicalDevice pd = devs[i];

        uint32_t qcount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(pd, &qcount, NULL);
        VkQueueFamilyProperties* qprops = malloc(sizeof(VkQueueFamilyProperties) * qcount);
        vkGetPhysicalDeviceQueueFamilyProperties(pd, &qcount, qprops);

        int gfx = -1, present = -1;
        for (uint32_t q = 0; q < qcount; ++q)
        {
            if (qprops[q].queueFlags & VK_QUEUE_GRAPHICS_BIT) gfx = (int)q;
            VkBool32 supported = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(pd, q, g.surface, &supported);
            if (supported) present = (int)q;
            if (gfx != -1 && present != -1) break;
        }
        free(qprops);
        if (gfx == -1 || present == -1) continue;

        uint32_t ext_count = 0;
        VK_CHECK(vkEnumerateDeviceExtensionProperties(pd, NULL, &ext_count, NULL));
        VkExtensionProperties* ext_props = malloc(sizeof(VkExtensionProperties) * ext_count);
        VK_CHECK(vkEnumerateDeviceExtensionProperties(pd, NULL, &ext_count, ext_props));
        bool has_swapchain = false;
        for (uint32_t e = 0; e < ext_count; ++e)
            if (strcmp(ext_props[e].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) { has_swapchain = true; break; }
        free(ext_props);
        if (!has_swapchain) continue;

        g.physical_device = pd;
        g.graphics_family = (uint32_t)gfx;
        g.present_family = (uint32_t)present;
        free(devs);
        return true;
    }

    free(devs);
    return false;
}

// ---------------------------------------------------------------------------
// Logical device + queues + command pool
// ---------------------------------------------------------------------------

static bool create_device(void)
{
    float prio = 1.0f;
    VkDeviceQueueCreateInfo queue_cis[2];
    uint32_t queue_ci_count = 0;

    queue_cis[queue_ci_count++] = (VkDeviceQueueCreateInfo){
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = g.graphics_family,
        .queueCount = 1,
        .pQueuePriorities = &prio,
    };
    if (g.present_family != g.graphics_family)
    {
        queue_cis[queue_ci_count++] = (VkDeviceQueueCreateInfo){
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = g.present_family,
            .queueCount = 1,
            .pQueuePriorities = &prio,
        };
    }

    const char* dev_exts[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    // Feature chain: Vulkan 1.2 descriptor indexing + 1.3 dynamic rendering + sync2
    VkPhysicalDeviceVulkan12Features vk12 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .descriptorIndexing = VK_TRUE,
        .descriptorBindingPartiallyBound = VK_TRUE,
        .descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
        .descriptorBindingVariableDescriptorCount = VK_TRUE,
        .runtimeDescriptorArray = VK_TRUE,
        .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
    };
    VkPhysicalDeviceSynchronization2Features sync2 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES,
        .pNext = &vk12,
        .synchronization2 = VK_TRUE,
    };
    VkPhysicalDeviceDynamicRenderingFeatures dyn_render = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
        .pNext = &sync2,
        .dynamicRendering = VK_TRUE,
    };

    VkDeviceCreateInfo dci = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &dyn_render,
        .queueCreateInfoCount = queue_ci_count,
        .pQueueCreateInfos = queue_cis,
        .enabledExtensionCount = COUNTOF(dev_exts),
        .ppEnabledExtensionNames = dev_exts,
    };

    VK_CHECK(vkCreateDevice(g.physical_device, &dci, NULL, &g.device));
    volkLoadDevice(g.device);

    vkGetDeviceQueue(g.device, g.graphics_family, 0, &g.graphics_queue);
    vkGetDeviceQueue(g.device, g.present_family, 0, &g.present_queue);

    VkCommandPoolCreateInfo pool_ci = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = g.graphics_family,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    };
    VK_CHECK(vkCreateCommandPool(g.device, &pool_ci, NULL, &g.command_pool));

    return true;
}

// ---------------------------------------------------------------------------
// Swapchain
// ---------------------------------------------------------------------------

static bool create_swapchain(uint32_t req_w, uint32_t req_h)
{
    VkSurfaceCapabilitiesKHR caps;
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g.physical_device, g.surface, &caps));

    uint32_t fmt_count = 0;
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(g.physical_device, g.surface, &fmt_count, NULL));
    VkSurfaceFormatKHR* fmts = malloc(sizeof(VkSurfaceFormatKHR) * fmt_count);
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(g.physical_device, g.surface, &fmt_count, fmts));

    VkSurfaceFormatKHR chosen_fmt = fmts[0];
    for (uint32_t i = 0; i < fmt_count; ++i)
    {
        if (fmts[i].format == VK_FORMAT_B8G8R8A8_SRGB) { chosen_fmt = fmts[i]; break; }
        if (fmts[i].format == VK_FORMAT_R8G8B8A8_SRGB) chosen_fmt = fmts[i];
    }
    free(fmts);

    VkExtent2D extent = caps.currentExtent;
    if (extent.width == UINT32_MAX)
    {
        extent.width  = req_w < caps.minImageExtent.width  ? caps.minImageExtent.width  : req_w > caps.maxImageExtent.width  ? caps.maxImageExtent.width  : req_w;
        extent.height = req_h < caps.minImageExtent.height ? caps.minImageExtent.height : req_h > caps.maxImageExtent.height ? caps.maxImageExtent.height : req_h;
    }
    if (extent.width == 0 || extent.height == 0) return false;

    uint32_t image_count = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && image_count > caps.maxImageCount) image_count = caps.maxImageCount;

    VkSwapchainCreateInfoKHR sci = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = g.surface,
        .minImageCount = image_count,
        .imageFormat = chosen_fmt.format,
        .imageColorSpace = chosen_fmt.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = caps.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = VK_TRUE,
    };
    uint32_t families[2] = {g.graphics_family, g.present_family};
    if (g.graphics_family != g.present_family)
    {
        sci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        sci.queueFamilyIndexCount = 2;
        sci.pQueueFamilyIndices = families;
    }

    VK_CHECK(vkCreateSwapchainKHR(g.device, &sci, NULL, &g.swapchain));
    g.swapchain_format = chosen_fmt.format;
    g.swapchain_extent = extent;

    VK_CHECK(vkGetSwapchainImagesKHR(g.device, g.swapchain, &image_count, NULL));
    g.swapchain_image_count = image_count;
    VK_CHECK(vkGetSwapchainImagesKHR(g.device, g.swapchain, &image_count, g.swapchain_images));

    VkSemaphoreCreateInfo sem_ci = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    for (uint32_t i = 0; i < image_count; ++i)
    {
        VkImageViewCreateInfo vci = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = g.swapchain_images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = g.swapchain_format,
            .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
        };
        VK_CHECK(vkCreateImageView(g.device, &vci, NULL, &g.swapchain_views[i]));
        VK_CHECK(vkCreateSemaphore(g.device, &sem_ci, NULL, &g.render_finished[i]));
    }
    return true;
}

static void destroy_swapchain(void)
{
    for (uint32_t i = 0; i < g.swapchain_image_count; ++i)
    {
        if (g.swapchain_views[i]) { vkDestroyImageView(g.device, g.swapchain_views[i], NULL); g.swapchain_views[i] = VK_NULL_HANDLE; }
        if (g.render_finished[i]) { vkDestroySemaphore(g.device, g.render_finished[i], NULL); g.render_finished[i] = VK_NULL_HANDLE; }
    }
    if (g.swapchain) { vkDestroySwapchainKHR(g.device, g.swapchain, NULL); g.swapchain = VK_NULL_HANDLE; }
    g.swapchain_image_count = 0;
}

static bool recreate_swapchain(void)
{
    vkDeviceWaitIdle(g.device);
    destroy_swapchain();
    return create_swapchain(g.width, g.height);
}

// ---------------------------------------------------------------------------
// Sync objects + command buffers
// ---------------------------------------------------------------------------

static bool create_sync_objects(void)
{
    VkCommandBufferAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = g.command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = FRAMES_IN_FLIGHT,
    };
    VkCommandBuffer cmds[FRAMES_IN_FLIGHT];
    VK_CHECK(vkAllocateCommandBuffers(g.device, &alloc_info, cmds));

    VkFenceCreateInfo fence_ci = {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT};
    VkSemaphoreCreateInfo sem_ci = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; ++i)
    {
        g.frames[i].cmd = cmds[i];
        VK_CHECK(vkCreateFence(g.device, &fence_ci, NULL, &g.frames[i].in_flight));
        VK_CHECK(vkCreateSemaphore(g.device, &sem_ci, NULL, &g.frames[i].image_acquired));
    }
    return true;
}

// ---------------------------------------------------------------------------
// Shader helpers
// ---------------------------------------------------------------------------

static const char* exe_relative_path(const char* relative)
{
    static char buf[1024];
    const char* base = SDL_GetBasePath();
    snprintf(buf, sizeof(buf), "%s%s", base, relative);
    return buf;
}

static VkShaderModule create_shader_module(const char* path)
{
    const char* full_path = exe_relative_path(path);
    size_t size = 0;
    uint8_t* code = read_file_binary(full_path, &size);
    if (!code) { fprintf(stderr, "[gfx] Failed to load shader: %s\n", full_path); return VK_NULL_HANDLE; }

    VkShaderModuleCreateInfo ci = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = size,
        .pCode = (const uint32_t*)code,
    };
    VkShaderModule module = VK_NULL_HANDLE;
    VK_CHECK(vkCreateShaderModule(g.device, &ci, NULL, &module));
    free(code);
    return module;
}

// ---------------------------------------------------------------------------
// Vulkan memory helpers
// ---------------------------------------------------------------------------

static uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties mem_props;
    vkGetPhysicalDeviceMemoryProperties(g.physical_device, &mem_props);
    for (uint32_t i = 0; i < mem_props.memoryTypeCount; ++i)
        if ((type_filter & (1u << i)) && (mem_props.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    fprintf(stderr, "[gfx] Failed to find suitable memory type\n");
    abort();
}

static VkCommandBuffer begin_oneshot_cmd(void)
{
    VkCommandBufferAllocateInfo ai = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = g.command_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    VkCommandBuffer cmd;
    VK_CHECK(vkAllocateCommandBuffers(g.device, &ai, &cmd));
    VkCommandBufferBeginInfo bi = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};
    VK_CHECK(vkBeginCommandBuffer(cmd, &bi));
    return cmd;
}

static void end_oneshot_cmd(VkCommandBuffer cmd)
{
    VK_CHECK(vkEndCommandBuffer(cmd));
    VkCommandBufferSubmitInfo cmd_info = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO, .commandBuffer = cmd};
    VkSubmitInfo2 submit = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2, .commandBufferInfoCount = 1, .pCommandBufferInfos = &cmd_info};
    VK_CHECK(vkQueueSubmit2(g.graphics_queue, 1, &submit, VK_NULL_HANDLE));
    vkQueueWaitIdle(g.graphics_queue);
    vkFreeCommandBuffers(g.device, g.command_pool, 1, &cmd);
}

// ---------------------------------------------------------------------------
// Sprite pipeline (batched + bindless)
// ---------------------------------------------------------------------------

static bool create_sprite_pipeline(void)
{
    // Nearest-neighbor sampler (pixel art)
    VkSamplerCreateInfo sampler_ci = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_NEAREST,
        .minFilter = VK_FILTER_NEAREST,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
    };
    VK_CHECK(vkCreateSampler(g.device, &sampler_ci, NULL, &g.nearest_sampler));

    // --- Set 0: Sprite SSBO ---
    VkDescriptorSetLayoutBinding ssbo_binding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    };
    VkDescriptorSetLayoutCreateInfo ssbo_layout_ci = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &ssbo_binding,
    };
    VK_CHECK(vkCreateDescriptorSetLayout(g.device, &ssbo_layout_ci, NULL, &g.ssbo_set_layout));

    // --- Set 1: Bindless textures (combined image sampler, sampler written per-texture) ---
    VkDescriptorSetLayoutBinding tex_binding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = MAX_TEXTURES,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
    };
    VkDescriptorBindingFlags tex_flags = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
                                          VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                                          VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;
    VkDescriptorSetLayoutBindingFlagsCreateInfo tex_flags_ci = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .bindingCount = 1,
        .pBindingFlags = &tex_flags,
    };
    VkDescriptorSetLayoutCreateInfo tex_layout_ci = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = &tex_flags_ci,
        .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
        .bindingCount = 1,
        .pBindings = &tex_binding,
    };
    VK_CHECK(vkCreateDescriptorSetLayout(g.device, &tex_layout_ci, NULL, &g.texture_set_layout));

    // --- Descriptor pool ---
    VkDescriptorPoolSize pool_sizes[] = {
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, FRAMES_IN_FLIGHT},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_TEXTURES},
    };
    VkDescriptorPoolCreateInfo pool_ci = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
        .maxSets = FRAMES_IN_FLIGHT + 1,
        .poolSizeCount = COUNTOF(pool_sizes),
        .pPoolSizes = pool_sizes,
    };
    VK_CHECK(vkCreateDescriptorPool(g.device, &pool_ci, NULL, &g.desc_pool));

    // --- Allocate set 0 descriptors (per-frame SSBO) ---
    VkDescriptorSetLayout ssbo_layouts[FRAMES_IN_FLIGHT];
    for (int i = 0; i < FRAMES_IN_FLIGHT; ++i) ssbo_layouts[i] = g.ssbo_set_layout;
    VkDescriptorSetAllocateInfo ssbo_alloc = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = g.desc_pool,
        .descriptorSetCount = FRAMES_IN_FLIGHT,
        .pSetLayouts = ssbo_layouts,
    };
    VK_CHECK(vkAllocateDescriptorSets(g.device, &ssbo_alloc, g.sprites.desc_sets));

    // --- Allocate set 1 descriptor (bindless textures) ---
    uint32_t variable_count = MAX_TEXTURES;
    VkDescriptorSetVariableDescriptorCountAllocateInfo var_count_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
        .descriptorSetCount = 1,
        .pDescriptorCounts = &variable_count,
    };
    VkDescriptorSetAllocateInfo tex_alloc = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = &var_count_info,
        .descriptorPool = g.desc_pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &g.texture_set_layout,
    };
    VK_CHECK(vkAllocateDescriptorSets(g.device, &tex_alloc, &g.textures.desc_set));

    // --- Create per-frame SSBO buffers (persistent mapped) ---
    for (int i = 0; i < FRAMES_IN_FLIGHT; ++i)
    {
        VkBufferCreateInfo buf_ci = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = SSBO_SIZE,
            .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        };
        VK_CHECK(vkCreateBuffer(g.device, &buf_ci, NULL, &g.sprites.buffers[i]));

        VkMemoryRequirements reqs;
        vkGetBufferMemoryRequirements(g.device, g.sprites.buffers[i], &reqs);
        VkMemoryAllocateInfo mem_ai = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = reqs.size,
            .memoryTypeIndex = find_memory_type(reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
        };
        VK_CHECK(vkAllocateMemory(g.device, &mem_ai, NULL, &g.sprites.memory[i]));
        VK_CHECK(vkBindBufferMemory(g.device, g.sprites.buffers[i], g.sprites.memory[i], 0));
        VK_CHECK(vkMapMemory(g.device, g.sprites.memory[i], 0, SSBO_SIZE, 0, &g.sprites.mapped[i]));

        // Write descriptor
        VkDescriptorBufferInfo buf_info = {.buffer = g.sprites.buffers[i], .offset = 0, .range = SSBO_SIZE};
        VkWriteDescriptorSet write = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = g.sprites.desc_sets[i],
            .dstBinding = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .pBufferInfo = &buf_info,
        };
        vkUpdateDescriptorSets(g.device, 1, &write, 0, NULL);
    }

    // --- Pipeline layout ---
    VkDescriptorSetLayout set_layouts[2] = {g.ssbo_set_layout, g.texture_set_layout};
    VkPushConstantRange push_range = {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = sizeof(float) * 4, // window_size.xy + camera_position.xy
    };
    VkPipelineLayoutCreateInfo layout_ci = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 2,
        .pSetLayouts = set_layouts,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &push_range,
    };
    VK_CHECK(vkCreatePipelineLayout(g.device, &layout_ci, NULL, &g.sprite_layout));

    // --- Graphics pipeline ---
    VkShaderModule vert = create_shader_module("shaders/sprite.vert.spv");
    VkShaderModule frag = create_shader_module("shaders/sprite.frag.spv");
    if (!vert || !frag) return false;

    VkPipelineShaderStageCreateInfo stages[] = {
        {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_VERTEX_BIT, .module = vert, .pName = "main"},
        {.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .module = frag, .pName = "main"},
    };
    VkPipelineVertexInputStateCreateInfo vertex_input = {.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    VkPipelineInputAssemblyStateCreateInfo input_assembly = {.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};
    VkPipelineViewportStateCreateInfo viewport_state = {.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, .viewportCount = 1, .scissorCount = 1};
    VkPipelineRasterizationStateCreateInfo raster = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .polygonMode = VK_POLYGON_MODE_FILL, .cullMode = VK_CULL_MODE_NONE, .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE, .lineWidth = 1.0f,
    };
    VkPipelineMultisampleStateCreateInfo multisample = {.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT};
    VkPipelineColorBlendAttachmentState blend_attachment = {
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA, .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO, .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };
    VkPipelineColorBlendStateCreateInfo color_blend = {.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, .attachmentCount = 1, .pAttachments = &blend_attachment};
    VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamic_state = {.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, .dynamicStateCount = COUNTOF(dynamic_states), .pDynamicStates = dynamic_states};

    VkPipelineRenderingCreateInfo rendering_ci = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &g.swapchain_format,
    };
    VkGraphicsPipelineCreateInfo pipeline_ci = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &rendering_ci,
        .stageCount = COUNTOF(stages), .pStages = stages,
        .pVertexInputState = &vertex_input, .pInputAssemblyState = &input_assembly,
        .pViewportState = &viewport_state, .pRasterizationState = &raster,
        .pMultisampleState = &multisample, .pColorBlendState = &color_blend,
        .pDynamicState = &dynamic_state, .layout = g.sprite_layout,
    };
    VK_CHECK(vkCreateGraphicsPipelines(g.device, VK_NULL_HANDLE, 1, &pipeline_ci, NULL, &g.sprite_pipeline));
    vkDestroyShaderModule(g.device, vert, NULL);
    vkDestroyShaderModule(g.device, frag, NULL);

    return true;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

bool gfx_init(const gfx_config* config)
{
    memset(&g, 0, sizeof(g));
    g.width = config->window_width;
    g.height = config->window_height;
#ifdef WBC_DEBUG
    g.validation = config->enable_validation;
#else
    g.validation = false;
    (void)config->enable_validation;
#endif

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        fprintf(stderr, "[gfx] SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

    g.window = SDL_CreateWindow(config->window_title, (int)config->window_width, (int)config->window_height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    if (!g.window) { fprintf(stderr, "[gfx] SDL_CreateWindow failed: %s\n", SDL_GetError()); return false; }

    if (!create_instance())      return false;
    if (!pick_physical_device()) return false;
    if (!create_device())        return false;
    if (!create_swapchain(g.width, g.height)) return false;
    if (!create_sync_objects())  return false;
    if (!create_sprite_pipeline()) return false;

    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(g.physical_device, &props);
    fprintf(stderr, "[gfx] Initialized: %s (Vulkan 1.4, %ux%u, bindless=%u)\n",
            props.deviceName, g.swapchain_extent.width, g.swapchain_extent.height, MAX_TEXTURES);

    return true;
}

void gfx_shutdown(void)
{
    if (g.device) vkDeviceWaitIdle(g.device);

    // Sprite pipeline
    for (int i = 0; i < FRAMES_IN_FLIGHT; ++i)
    {
        if (g.sprites.mapped[i]) vkUnmapMemory(g.device, g.sprites.memory[i]);
        if (g.sprites.buffers[i]) vkDestroyBuffer(g.device, g.sprites.buffers[i], NULL);
        if (g.sprites.memory[i]) vkFreeMemory(g.device, g.sprites.memory[i], NULL);
    }
    // Textures
    for (uint32_t i = 0; i < g.textures.count; ++i)
    {
        if (g.textures.views[i]) vkDestroyImageView(g.device, g.textures.views[i], NULL);
        if (g.textures.images[i]) vkDestroyImage(g.device, g.textures.images[i], NULL);
        if (g.textures.mem[i]) vkFreeMemory(g.device, g.textures.mem[i], NULL);
    }
    if (g.desc_pool) vkDestroyDescriptorPool(g.device, g.desc_pool, NULL);
    if (g.nearest_sampler) vkDestroySampler(g.device, g.nearest_sampler, NULL);
    if (g.sprite_pipeline) vkDestroyPipeline(g.device, g.sprite_pipeline, NULL);
    if (g.sprite_layout) vkDestroyPipelineLayout(g.device, g.sprite_layout, NULL);
    if (g.texture_set_layout) vkDestroyDescriptorSetLayout(g.device, g.texture_set_layout, NULL);
    if (g.ssbo_set_layout) vkDestroyDescriptorSetLayout(g.device, g.ssbo_set_layout, NULL);

    // Frame sync
    for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; ++i)
    {
        if (g.frames[i].in_flight) vkDestroyFence(g.device, g.frames[i].in_flight, NULL);
        if (g.frames[i].image_acquired) vkDestroySemaphore(g.device, g.frames[i].image_acquired, NULL);
    }
    destroy_swapchain();
    if (g.command_pool) vkDestroyCommandPool(g.device, g.command_pool, NULL);
    if (g.device) vkDestroyDevice(g.device, NULL);
    if (g.surface) vkDestroySurfaceKHR(g.instance, g.surface, NULL);
    if (g.debug_messenger) vkDestroyDebugUtilsMessengerEXT(g.instance, g.debug_messenger, NULL);
    if (g.instance) vkDestroyInstance(g.instance, NULL);
    if (g.window) SDL_DestroyWindow(g.window);
    SDL_Quit();
    memset(&g, 0, sizeof(g));
}

bool gfx_poll_events(void)
{
    // Reset per-frame input flags
    g.input.mouse_left_pressed = false;
    g.input.mouse_right_pressed = false;
    g.input.mouse_left_released = false;
    g.input.mouse_right_released = false;

    SDL_Event ev;
    while (SDL_PollEvent(&ev))
    {
        switch (ev.type)
        {
            case SDL_EVENT_QUIT:
                g.quit_requested = true;
                break;
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                g.width = (uint32_t)ev.window.data1;
                g.height = (uint32_t)ev.window.data2;
                g.need_resize = true;
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (ev.button.button == SDL_BUTTON_LEFT)  { g.input.mouse_left_pressed = true; g.input.mouse_left_held = true; }
                if (ev.button.button == SDL_BUTTON_RIGHT) { g.input.mouse_right_pressed = true; g.input.mouse_right_held = true; }
                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (ev.button.button == SDL_BUTTON_LEFT)  { g.input.mouse_left_released = true; g.input.mouse_left_held = false; }
                if (ev.button.button == SDL_BUTTON_RIGHT) { g.input.mouse_right_released = true; g.input.mouse_right_held = false; }
                break;
        }
    }

    // Update mouse position
    float mx, my;
    SDL_GetMouseState(&mx, &my);
    g.input.mouse_x = mx;
    g.input.mouse_y = my;

    return !g.quit_requested;
}

bool gfx_begin_frame(void)
{
    g.frame_active = false;

    if (g.need_resize)
    {
        if (g.width == 0 || g.height == 0) return false;
        if (!recreate_swapchain()) return false;
        g.need_resize = false;
    }

    frame_sync* f = &g.frames[g.frame_index];

    VK_CHECK(vkWaitForFences(g.device, 1, &f->in_flight, VK_TRUE, UINT64_MAX));
    VK_CHECK(vkResetFences(g.device, 1, &f->in_flight));

    VkResult ar = vkAcquireNextImageKHR(g.device, g.swapchain, UINT64_MAX, f->image_acquired, VK_NULL_HANDLE, &g.image_index);
    if (ar == VK_ERROR_OUT_OF_DATE_KHR)
    {
        g.need_resize = true;
        VkSubmitInfo2 empty = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2};
        VK_CHECK(vkQueueSubmit2(g.graphics_queue, 1, &empty, f->in_flight));
        return false;
    }
    if (ar != VK_SUCCESS && ar != VK_SUBOPTIMAL_KHR)
    {
        fprintf(stderr, "[gfx] vkAcquireNextImageKHR failed: %d\n", ar);
        VkSubmitInfo2 empty = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2};
        VK_CHECK(vkQueueSubmit2(g.graphics_queue, 1, &empty, f->in_flight));
        return false;
    }

    VK_CHECK(vkResetCommandBuffer(f->cmd, 0));
    VkCommandBufferBeginInfo begin_info = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};
    VK_CHECK(vkBeginCommandBuffer(f->cmd, &begin_info));

    // Transition: UNDEFINED -> COLOR_ATTACHMENT
    VkImageMemoryBarrier2 barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_NONE, .srcAccessMask = 0,
        .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED, .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .image = g.swapchain_images[g.image_index], .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
    };
    VkDependencyInfo dep = {.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &barrier};
    vkCmdPipelineBarrier2(f->cmd, &dep);

    VkClearValue clear = {.color = {.float32 = {0.392f, 0.584f, 0.929f, 1.0f}}};
    VkRenderingAttachmentInfo color_att = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = g.swapchain_views[g.image_index], .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, .storeOp = VK_ATTACHMENT_STORE_OP_STORE, .clearValue = clear,
    };
    VkRenderingInfo ri = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {{0, 0}, g.swapchain_extent}, .layerCount = 1, .colorAttachmentCount = 1, .pColorAttachments = &color_att,
    };
    vkCmdBeginRendering(f->cmd, &ri);

    g.frame_active = true;
    return true;
}

void gfx_end_frame(void)
{
    if (!g.frame_active) return;
    frame_sync* f = &g.frames[g.frame_index];

    // Flush sprite batch
    if (g.sprites.count > 0)
    {
        memcpy(g.sprites.mapped[g.frame_index], g.sprites.batch, g.sprites.count * SPRITE_SIZE);

        vkCmdBindPipeline(f->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g.sprite_pipeline);

        VkViewport viewport = {.width = (float)g.swapchain_extent.width, .height = (float)g.swapchain_extent.height, .maxDepth = 1.0f};
        vkCmdSetViewport(f->cmd, 0, 1, &viewport);
        VkRect2D scissor = {.extent = g.swapchain_extent};
        vkCmdSetScissor(f->cmd, 0, 1, &scissor);

        VkDescriptorSet sets[2] = {g.sprites.desc_sets[g.frame_index], g.textures.desc_set};
        vkCmdBindDescriptorSets(f->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g.sprite_layout, 0, 2, sets, 0, NULL);

        float push[4] = {(float)g.swapchain_extent.width, (float)g.swapchain_extent.height, g.camera_x, g.camera_y};
        vkCmdPushConstants(f->cmd, g.sprite_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push), push);

        vkCmdDraw(f->cmd, 6 * g.sprites.count, 1, 0, 0);
        g.sprites.count = 0;
    }

    vkCmdEndRendering(f->cmd);

    // Transition: COLOR_ATTACHMENT -> PRESENT_SRC
    VkImageMemoryBarrier2 barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_NONE, .dstAccessMask = 0,
        .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .image = g.swapchain_images[g.image_index], .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
    };
    VkDependencyInfo dep = {.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &barrier};
    vkCmdPipelineBarrier2(f->cmd, &dep);

    VK_CHECK(vkEndCommandBuffer(f->cmd));

    VkSemaphoreSubmitInfo wait_sem = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, .semaphore = f->image_acquired, .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphoreSubmitInfo signal_sem = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO, .semaphore = g.render_finished[g.image_index], .stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT};
    VkCommandBufferSubmitInfo cmd_info = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO, .commandBuffer = f->cmd};
    VkSubmitInfo2 submit = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .waitSemaphoreInfoCount = 1, .pWaitSemaphoreInfos = &wait_sem,
        .commandBufferInfoCount = 1, .pCommandBufferInfos = &cmd_info,
        .signalSemaphoreInfoCount = 1, .pSignalSemaphoreInfos = &signal_sem,
    };
    VK_CHECK(vkQueueSubmit2(g.graphics_queue, 1, &submit, f->in_flight));

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1, .pWaitSemaphores = &g.render_finished[g.image_index],
        .swapchainCount = 1, .pSwapchains = &g.swapchain, .pImageIndices = &g.image_index,
    };
    VkResult pr = vkQueuePresentKHR(g.present_queue, &present_info);
    if (pr == VK_ERROR_OUT_OF_DATE_KHR || pr == VK_SUBOPTIMAL_KHR) g.need_resize = true;
    else if (pr != VK_SUCCESS) fprintf(stderr, "[gfx] vkQueuePresentKHR failed: %d\n", pr);

    g.frame_index = (g.frame_index + 1) % FRAMES_IN_FLIGHT;
    g.frame_active = false;
}

void gfx_get_extent(uint32_t* w, uint32_t* h)
{
    *w = g.swapchain_extent.width;
    *h = g.swapchain_extent.height;
}

void gfx_set_camera(float x, float y)
{
    g.camera_x = x;
    g.camera_y = y;
}

void gfx_get_camera(float* x, float* y)
{
    *x = g.camera_x;
    *y = g.camera_y;
}

const gfx_input* gfx_get_input(void)
{
    return &g.input;
}

// ---------------------------------------------------------------------------
// Texture creation (bindless)
// ---------------------------------------------------------------------------

uint32_t gfx_create_texture(uint32_t width, uint32_t height, const uint8_t* rgba_pixels)
{
    if (g.textures.count >= MAX_TEXTURES)
    {
        fprintf(stderr, "[gfx] Texture limit reached (%d)\n", MAX_TEXTURES);
        return UINT32_MAX;
    }

    VkDeviceSize image_size = (VkDeviceSize)width * height * 4;
    uint32_t idx = g.textures.count;

    // Staging buffer
    VkBufferCreateInfo staging_buf_ci = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = image_size, .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT};
    VkBuffer staging_buf;
    VK_CHECK(vkCreateBuffer(g.device, &staging_buf_ci, NULL, &staging_buf));

    VkMemoryRequirements staging_reqs;
    vkGetBufferMemoryRequirements(g.device, staging_buf, &staging_reqs);
    VkMemoryAllocateInfo staging_alloc = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = staging_reqs.size,
        .memoryTypeIndex = find_memory_type(staging_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
    };
    VkDeviceMemory staging_mem;
    VK_CHECK(vkAllocateMemory(g.device, &staging_alloc, NULL, &staging_mem));
    VK_CHECK(vkBindBufferMemory(g.device, staging_buf, staging_mem, 0));

    void* mapped;
    VK_CHECK(vkMapMemory(g.device, staging_mem, 0, image_size, 0, &mapped));
    memcpy(mapped, rgba_pixels, image_size);
    vkUnmapMemory(g.device, staging_mem);

    // GPU image (UNORM for paletted pixel art)
    VkImageCreateInfo image_ci = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .extent = {width, height, 1},
        .mipLevels = 1, .arrayLayers = 1, .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };
    VK_CHECK(vkCreateImage(g.device, &image_ci, NULL, &g.textures.images[idx]));

    VkMemoryRequirements img_reqs;
    vkGetImageMemoryRequirements(g.device, g.textures.images[idx], &img_reqs);
    VkMemoryAllocateInfo img_alloc = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = img_reqs.size,
        .memoryTypeIndex = find_memory_type(img_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
    };
    VK_CHECK(vkAllocateMemory(g.device, &img_alloc, NULL, &g.textures.mem[idx]));
    VK_CHECK(vkBindImageMemory(g.device, g.textures.images[idx], g.textures.mem[idx], 0));

    // Upload
    VkCommandBuffer cmd = begin_oneshot_cmd();

    VkImageMemoryBarrier2 to_transfer = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_NONE, .srcAccessMask = 0,
        .dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT, .dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED, .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .image = g.textures.images[idx], .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
    };
    VkDependencyInfo dep1 = {.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &to_transfer};
    vkCmdPipelineBarrier2(cmd, &dep1);

    VkBufferImageCopy2 region = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
        .imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
        .imageExtent = {width, height, 1},
    };
    VkCopyBufferToImageInfo2 copy_info = {
        .sType = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2,
        .srcBuffer = staging_buf, .dstImage = g.textures.images[idx],
        .dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .regionCount = 1, .pRegions = &region,
    };
    vkCmdCopyBufferToImage2(cmd, &copy_info);

    VkImageMemoryBarrier2 to_shader = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT, .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, .dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .image = g.textures.images[idx], .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
    };
    VkDependencyInfo dep2 = {.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &to_shader};
    vkCmdPipelineBarrier2(cmd, &dep2);

    end_oneshot_cmd(cmd);

    vkDestroyBuffer(g.device, staging_buf, NULL);
    vkFreeMemory(g.device, staging_mem, NULL);

    // Image view
    VkImageViewCreateInfo view_ci = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = g.textures.images[idx], .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_UNORM,
        .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
    };
    VK_CHECK(vkCreateImageView(g.device, &view_ci, NULL, &g.textures.views[idx]));

    // Write bindless descriptor
    VkDescriptorImageInfo img_info = {
        .sampler = g.nearest_sampler,
        .imageView = g.textures.views[idx],
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
    VkWriteDescriptorSet write = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = g.textures.desc_set,
        .dstBinding = 0,
        .dstArrayElement = idx,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &img_info,
    };
    vkUpdateDescriptorSets(g.device, 1, &write, 0, NULL);

    g.textures.count++;
    return idx;
}

// ---------------------------------------------------------------------------
// Sprite batching
// ---------------------------------------------------------------------------

void gfx_draw_sprite(float x, float y, float w, float h, uint32_t texture_index, uint32_t color)
{
    gfx_draw_sprite_region(x, y, w, h, texture_index, color, 0.0f, 0.0f, 1.0f, 1.0f);
}

void gfx_draw_sprite_region(float x, float y, float w, float h,
                            uint32_t texture_index, uint32_t color,
                            float uv_x, float uv_y, float uv_w, float uv_h)
{
    if (g.sprites.count >= MAX_SPRITES) return;
    g.sprites.batch[g.sprites.count++] = (gpu_sprite){
        .position = {x, y},
        .size = {w, h},
        .uv_offset = {uv_x, uv_y},
        .uv_scale = {uv_w, uv_h},
        .texture_index = texture_index,
        .color = color,
    };
}
