// /src7/renderer_vk/src7/vk_backend.c
// Minimal Vulkan 1.4 backend using volk + SDL3 (dynamic rendering).
// Clears the swapchain to RgFrameData.clear_rgba.

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <assert.h>
#include <inttypes.h>
#include <render_api/render_api.h>
#include <renderer/renderer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <volk.h>

#define RG_COUNTOF(x) ((int) (sizeof(x) / sizeof((x)[0])))
enum
{
    RG_FRAMES_IN_FLIGHT = 2
}; // <- global, not in the context

#define RG_CHECK(expr)                                                                                                                     \
    do                                                                                                                                     \
    {                                                                                                                                      \
        VkResult _r = (expr);                                                                                                              \
        if (_r != VK_SUCCESS)                                                                                                              \
        {                                                                                                                                  \
            fprintf(stderr, "Vulkan error %d at %s:%d -> %s\n", _r, __FILE__, __LINE__, #expr);                                            \
            abort();                                                                                                                       \
        }                                                                                                                                  \
    } while (0)

typedef struct RgFrameSync
{
    VkFence in_flight;
    VkSemaphore image_acquired;
    VkCommandBuffer cmd;
} RgFrameSync;

typedef struct RgContext
{
    // App/Window
    SDL_Window* window;
    uint32_t width, height;
    bool validation;

    // Vulkan core
    VkInstance instance;
    VkDebugUtilsMessengerEXT dbg_messenger;
    VkSurfaceKHR surface;
    VkPhysicalDevice phys;
    VkDevice device;

    // Queues
    uint32_t qfam_graphics;
    uint32_t qfam_present;
    VkQueue q_graphics;
    VkQueue q_present;

    // Swapchain & images
    VkSwapchainKHR swapchain;
    VkFormat swap_format;
    VkColorSpaceKHR swap_colorspace;
    VkExtent2D swap_extent;
    VkImage* swap_images;
    VkImageView* swap_views;
    VkImageLayout* swap_layouts;
    uint32_t swap_image_count;

    VkSemaphore* present_complete; // length = swap_image_count

    // Commands & sync
    VkCommandPool cmd_pool;
    RgFrameSync frames[RG_FRAMES_IN_FLIGHT];
    uint32_t frame_index;
    uint32_t image_index;

    bool need_recreate;
} RgContext;

// ---------------- Debug utils ----------------

static VKAPI_ATTR VkBool32 VKAPI_CALL rg_dbg_cb(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT types,
                                                const VkDebugUtilsMessengerCallbackDataEXT* data, void* user_data)
{
    (void) types;
    (void) user_data;
    const char* sev = (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)     ? "ERROR"
                      : (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) ? "WARN "
                      : (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)    ? "INFO "
                                                                                     : "VERB ";
    fprintf(stderr, "[VK/%s] %s\n", sev, data->pMessage);
    return VK_FALSE;
}

static void rg_create_debug_messenger(RgContext* c)
{
    if (!c->validation)
        return;
    VkDebugUtilsMessengerCreateInfoEXT info = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = rg_dbg_cb,
    };
    RG_CHECK(vkCreateDebugUtilsMessengerEXT(c->instance, &info, NULL, &c->dbg_messenger));
}

static void rg_destroy_debug_messenger(RgContext* c)
{
    if (!c->dbg_messenger)
        return;
    vkDestroyDebugUtilsMessengerEXT(c->instance, c->dbg_messenger, NULL);
    c->dbg_messenger = VK_NULL_HANDLE;
}

// --------------- Instance / SDL3 WSI ----------------

static bool rg_create_instance(RgContext* c)
{
    // SDL3: new signature returns an array of const char* and fills count
    uint32_t sdl_count = 0;
    const char* const* sdl_exts = SDL_Vulkan_GetInstanceExtensions(&sdl_count);
    if (!sdl_exts || sdl_count == 0)
    {
        fprintf(stderr, "SDL_Vulkan_GetInstanceExtensions failed: %s\n", SDL_GetError());
        return false;
    }

    const char* layers_dbg[] = {"VK_LAYER_KHRONOS_validation"};

    VkApplicationInfo app = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                             .pApplicationName = "Recon",
                             .applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
                             .pEngineName = "Recon",
                             .engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0),
                             .apiVersion = VK_API_VERSION_1_4};

    uint32_t ext_count = sdl_count + (c->validation ? 1u : 0u);
    const char** exts = (const char**) SDL_malloc(sizeof(char*) * ext_count);
    for (uint32_t i = 0; i < sdl_count; ++i)
        exts[i] = sdl_exts[i];
    if (c->validation)
        exts[sdl_count] = "VK_EXT_debug_utils";

    // Load global functions before vkCreateInstance
    RG_CHECK(volkInitialize());

    if (!vkCreateInstance)
    {
        fprintf(stderr, "volkInitialize() did not load vkCreateInstance (is vulkan-1.dll present?)\n");
        SDL_free(exts);
        return false;
    }

    VkInstanceCreateInfo ci = {.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                               .pApplicationInfo = &app,
                               .enabledExtensionCount = ext_count,
                               .ppEnabledExtensionNames = exts,
                               .enabledLayerCount = c->validation ? RG_COUNTOF(layers_dbg) : 0u,
                               .ppEnabledLayerNames = c->validation ? layers_dbg : NULL};

    VkResult r = vkCreateInstance(&ci, NULL, &c->instance);
    SDL_free((void*) exts);
    if (r != VK_SUCCESS)
    {
        fprintf(stderr, "vkCreateInstance failed (%d).\n", r);
        return false;
    }

    // Instance-level function pointers (extensions included) via volk
    volkLoadInstance(c->instance);

    // Create WSI surface
    if (!SDL_Vulkan_CreateSurface(c->window, c->instance, NULL, &c->surface))
    {
        fprintf(stderr, "SDL_Vulkan_CreateSurface failed: %s\n", SDL_GetError());
        return false;
    }

    if (c->validation)
        rg_create_debug_messenger(c);
    return true;
}

// --------------- Physical device & device ---------------

static bool rg_pick_physical_device(RgContext* c)
{
    uint32_t count = 0;
    RG_CHECK(vkEnumeratePhysicalDevices(c->instance, &count, NULL));
    if (count == 0)
        return false;

    VkPhysicalDevice* devs = (VkPhysicalDevice*) SDL_malloc(sizeof(VkPhysicalDevice) * count);
    RG_CHECK(vkEnumeratePhysicalDevices(c->instance, &count, devs));

    for (uint32_t i = 0; i < count; ++i)
    {
        VkPhysicalDevice pd = devs[i];

        uint32_t qcount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(pd, &qcount, NULL);
        VkQueueFamilyProperties* qprops = (VkQueueFamilyProperties*) SDL_malloc(sizeof(*qprops) * qcount);
        vkGetPhysicalDeviceQueueFamilyProperties(pd, &qcount, qprops);

        int g = -1, p = -1;
        for (uint32_t q = 0; q < qcount; ++q)
        {
            if (qprops[q].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                g = (int) q;
            VkBool32 present_supported = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(pd, q, c->surface, &present_supported);
            if (present_supported)
                p = (int) q;
            if (g != -1 && p != -1)
                break;
        }
        SDL_free(qprops);

        if (g == -1 || p == -1)
            continue;

        // Ensure swapchain extension exists
        uint32_t dext_count = 0;
        RG_CHECK(vkEnumerateDeviceExtensionProperties(pd, NULL, &dext_count, NULL));
        VkExtensionProperties* exts = (VkExtensionProperties*) SDL_malloc(sizeof(*exts) * dext_count);
        RG_CHECK(vkEnumerateDeviceExtensionProperties(pd, NULL, &dext_count, exts));
        bool has_swapchain = false;
        for (uint32_t e = 0; e < dext_count; ++e)
        {
            if (strcmp(exts[e].extensionName, "VK_KHR_swapchain") == 0)
            {
                has_swapchain = true;
                break;
            }
        }
        SDL_free(exts);
        if (!has_swapchain)
            continue;

        c->phys = pd;
        c->qfam_graphics = (uint32_t) g;
        c->qfam_present = (uint32_t) p;
        SDL_free(devs);
        return true;
    }

    SDL_free(devs);
    return false;
}

static bool rg_create_device_and_queues(RgContext* c)
{
    float prio = 1.0f;
    VkDeviceQueueCreateInfo qci[2];
    memset(qci, 0, sizeof(qci));
    uint32_t qci_count = 0;

    qci[qci_count++] = (VkDeviceQueueCreateInfo) {.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                                                  .queueFamilyIndex = c->qfam_graphics,
                                                  .queueCount = 1,
                                                  .pQueuePriorities = &prio};
    if (c->qfam_present != c->qfam_graphics)
    {
        qci[qci_count++] = (VkDeviceQueueCreateInfo) {.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                                                      .queueFamilyIndex = c->qfam_present,
                                                      .queueCount = 1,
                                                      .pQueuePriorities = &prio};
    }

    const char* dev_exts[] = {"VK_KHR_swapchain"};

    VkPhysicalDeviceSynchronization2Features sync2 = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES,
                                                      .synchronization2 = VK_TRUE};
    VkPhysicalDeviceDynamicRenderingFeatures dyn = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
                                                    .pNext = &sync2,
                                                    .dynamicRendering = VK_TRUE};

    VkDeviceCreateInfo dci = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &dyn,
        .queueCreateInfoCount = qci_count,
        .pQueueCreateInfos = qci,
        .enabledExtensionCount = RG_COUNTOF(dev_exts),
        .ppEnabledExtensionNames = dev_exts,
    };

    RG_CHECK(vkCreateDevice(c->phys, &dci, NULL, &c->device));

    // Device-level dispatch
    volkLoadDevice(c->device);

    vkGetDeviceQueue(c->device, c->qfam_graphics, 0, &c->q_graphics);
    vkGetDeviceQueue(c->device, c->qfam_present, 0, &c->q_present);

    VkCommandPoolCreateInfo pci = {.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                                   .queueFamilyIndex = c->qfam_graphics,
                                   .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT};
    RG_CHECK(vkCreateCommandPool(c->device, &pci, NULL, &c->cmd_pool));
    return true;
}

// --------------- Swapchain ---------------

static VkSurfaceFormatKHR pick_surface_format(VkPhysicalDevice pd, VkSurfaceKHR surface)
{
    uint32_t n = 0;
    RG_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(pd, surface, &n, NULL));
    VkSurfaceFormatKHR* a = (VkSurfaceFormatKHR*) SDL_malloc(sizeof(*a) * n);
    RG_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(pd, surface, &n, a));

    VkSurfaceFormatKHR chosen = a[0];
    for (uint32_t i = 0; i < n; ++i)
    {
        if (a[i].format == VK_FORMAT_B8G8R8A8_SRGB)
        {
            chosen = a[i];
            break;
        }
        if (a[i].format == VK_FORMAT_R8G8B8A8_SRGB)
        {
            chosen = a[i];
        }
    }
    SDL_free(a);
    return chosen;
}

static VkPresentModeKHR pick_present_mode(void)
{
    return VK_PRESENT_MODE_FIFO_KHR; // guaranteed
}

static bool rg_create_swapchain(RgContext* c, uint32_t req_w, uint32_t req_h)
{
    VkSurfaceCapabilitiesKHR caps;
    RG_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(c->phys, c->surface, &caps));

    VkSurfaceFormatKHR fmt = pick_surface_format(c->phys, c->surface);
    VkPresentModeKHR pmode = pick_present_mode();

    VkExtent2D extent = caps.currentExtent;
    if (extent.width == UINT32_MAX)
    {
        extent.width = SDL_clamp(req_w, caps.minImageExtent.width, caps.maxImageExtent.width);
        extent.height = SDL_clamp(req_h, caps.minImageExtent.height, caps.maxImageExtent.height);
    }
    if (extent.width == 0 || extent.height == 0)
        return false;

    uint32_t image_count = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && image_count > caps.maxImageCount)
        image_count = caps.maxImageCount;

    VkSwapchainCreateInfoKHR sci = {.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                                    .surface = c->surface,
                                    .minImageCount = image_count,
                                    .imageFormat = fmt.format,
                                    .imageColorSpace = fmt.colorSpace,
                                    .imageExtent = extent,
                                    .imageArrayLayers = 1,
                                    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                    .imageSharingMode = (c->qfam_graphics == c->qfam_present) ? VK_SHARING_MODE_EXCLUSIVE
                                                                                              : VK_SHARING_MODE_CONCURRENT,
                                    .preTransform = caps.currentTransform,
                                    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                                    .presentMode = pmode,
                                    .clipped = VK_TRUE,
                                    .oldSwapchain = VK_NULL_HANDLE};
    uint32_t qIx[2] = {c->qfam_graphics, c->qfam_present};
    if (c->qfam_graphics != c->qfam_present)
    {
        sci.queueFamilyIndexCount = 2;
        sci.pQueueFamilyIndices = qIx;
    }

    RG_CHECK(vkCreateSwapchainKHR(c->device, &sci, NULL, &c->swapchain));
    c->swap_format = fmt.format;
    c->swap_colorspace = fmt.colorSpace;
    c->swap_extent = extent;

    RG_CHECK(vkGetSwapchainImagesKHR(c->device, c->swapchain, &image_count, NULL));
    c->swap_image_count = image_count;
    c->swap_images = (VkImage*) SDL_malloc(sizeof(VkImage) * image_count);
    c->swap_views = (VkImageView*) SDL_malloc(sizeof(VkImageView) * image_count);
    c->swap_layouts = (VkImageLayout*) SDL_malloc(sizeof(VkImageLayout) * image_count);
    RG_CHECK(vkGetSwapchainImagesKHR(c->device, c->swapchain, &image_count, c->swap_images));

    for (uint32_t i = 0; i < image_count; ++i)
    {
        VkImageViewCreateInfo vi = {.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                                    .image = c->swap_images[i],
                                    .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                    .format = c->swap_format,
                                    .components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                                   VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},
                                    .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
        RG_CHECK(vkCreateImageView(c->device, &vi, NULL, &c->swap_views[i]));
        c->swap_layouts[i] = VK_IMAGE_LAYOUT_UNDEFINED;
    }

    // Per-frame resources (on first create)
    if (c->frames[0].cmd == VK_NULL_HANDLE)
    {
        VkCommandBufferAllocateInfo ai = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                                          .commandPool = c->cmd_pool,
                                          .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                                          .commandBufferCount = RG_FRAMES_IN_FLIGHT};
        VkCommandBuffer tmp[RG_FRAMES_IN_FLIGHT] = {VK_NULL_HANDLE};
        RG_CHECK(vkAllocateCommandBuffers(c->device, &ai, tmp));
        for (uint32_t i = 0; i < RG_FRAMES_IN_FLIGHT; ++i)
            c->frames[i].cmd = tmp[i];

        VkFenceCreateInfo fci = {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT};
        VkSemaphoreCreateInfo sciSem = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        for (uint32_t i = 0; i < RG_FRAMES_IN_FLIGHT; ++i)
        {
            RG_CHECK(vkCreateFence(c->device, &fci, NULL, &c->frames[i].in_flight));
            RG_CHECK(vkCreateSemaphore(c->device, &sciSem, NULL, &c->frames[i].image_acquired));
        }
    }

    // (Re)create per-image present semaphores every time we (re)create the swapchain
    if (c->present_complete)
    {
        for (uint32_t i = 0; i < c->swap_image_count; ++i)
            if (c->present_complete[i])
                vkDestroySemaphore(c->device, c->present_complete[i], NULL);
        SDL_free(c->present_complete);
    }
    c->present_complete = (VkSemaphore*) SDL_calloc(image_count, sizeof(VkSemaphore));
    VkSemaphoreCreateInfo sciSem = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    for (uint32_t i = 0; i < image_count; ++i)
    {
        RG_CHECK(vkCreateSemaphore(c->device, &sciSem, NULL, &c->present_complete[i]));
    }

    return true;
}

static void rg_destroy_swapchain(RgContext* c)
{
    if (c->present_complete)
    {
        for (uint32_t i = 0; i < c->swap_image_count; ++i)
            if (c->present_complete[i])
                vkDestroySemaphore(c->device, c->present_complete[i], NULL);
        SDL_free(c->present_complete);
        c->present_complete = NULL;
    }

    if (c->swap_views)
    {
        for (uint32_t i = 0; i < c->swap_image_count; ++i)
            if (c->swap_views[i])
                vkDestroyImageView(c->device, c->swap_views[i], NULL);
        SDL_free(c->swap_views);
        c->swap_views = NULL;
    }
    if (c->swap_images)
    {
        SDL_free(c->swap_images);
        c->swap_images = NULL;
    }
    if (c->swap_layouts)
    {
        SDL_free(c->swap_layouts);
        c->swap_layouts = NULL;
    }
    if (c->swapchain)
    {
        vkDestroySwapchainKHR(c->device, c->swapchain, NULL);
        c->swapchain = VK_NULL_HANDLE;
    }
}

static bool rg_recreate_swapchain(RgContext* c, uint32_t w, uint32_t h)
{
    vkDeviceWaitIdle(c->device);
    rg_destroy_swapchain(c);
    return rg_create_swapchain(c, w, h);
}

// --------------------- RgApi impl ---------------------

static RgResult rg_create(const RgCreateInfo* ci, RgHandle** out_handle)
{
    if (!ci || !ci->sdl_window || !out_handle)
        return RG_ERR_BAD_ARG;

    RgContext* c = (RgContext*) SDL_calloc(1, sizeof(RgContext));
    c->window = ci->sdl_window;
    c->width = ci->width;
    c->height = ci->height;
    c->validation = ci->enable_validation;

    if (!rg_create_instance(c))
    {
        SDL_free(c);
        return RG_ERR_BACKEND;
    }
    if (!rg_pick_physical_device(c))
    {
        SDL_free(c);
        return RG_ERR_BACKEND;
    }
    if (!rg_create_device_and_queues(c))
    {
        SDL_free(c);
        return RG_ERR_BACKEND;
    }
    if (!rg_create_swapchain(c, c->width, c->height))
    {
        SDL_free(c);
        return RG_ERR_BACKEND;
    }

    c->frame_index = 0;
    c->image_index = 0;
    c->need_recreate = false;

    *out_handle = (RgHandle*) c;
    return RG_SUCCESS;
}

static void rg_destroy(RgHandle* handle)
{
    if (!handle)
        return;
    RgContext* c = (RgContext*) handle;

    vkDeviceWaitIdle(c->device);

    for (uint32_t i = 0; i < RG_FRAMES_IN_FLIGHT; ++i)
    {
        if (c->frames[i].in_flight)
            vkDestroyFence(c->device, c->frames[i].in_flight, NULL);
        if (c->frames[i].image_acquired)
            vkDestroySemaphore(c->device, c->frames[i].image_acquired, NULL);
    }

    rg_destroy_swapchain(c);

    if (c->cmd_pool)
        vkDestroyCommandPool(c->device, c->cmd_pool, NULL);
    if (c->device)
        vkDestroyDevice(c->device, NULL);

    if (c->surface)
        vkDestroySurfaceKHR(c->instance, c->surface, NULL);
    rg_destroy_debug_messenger(c);
    if (c->instance)
        vkDestroyInstance(c->instance, NULL);

    SDL_free(c);
}

static RgResult rg_resize(RgHandle* handle, uint32_t width, uint32_t height)
{
    RgContext* c = (RgContext*) handle;
    if (!c)
        return RG_ERR_BAD_ARG;
    if (width == 0 || height == 0)
        return RG_SUCCESS; // minimized
    c->width = width;
    c->height = height;
    c->need_recreate = true;
    return RG_SUCCESS;
}

static RgResult rg_begin_frame(RgHandle* handle)
{
    RgContext* c = (RgContext*) handle;
    if (!c)
        return RG_ERR_BAD_ARG;

    if (c->need_recreate)
    {
        if (!rg_recreate_swapchain(c, c->width, c->height))
            return RG_ERR_BACKEND;
        c->need_recreate = false;
    }

    RgFrameSync* f = &c->frames[c->frame_index];

    // Wait for GPU to finish with this frame, then reset fence for reuse
    RG_CHECK(vkWaitForFences(c->device, 1, &f->in_flight, VK_TRUE, UINT64_MAX));
    RG_CHECK(vkResetFences(c->device, 1, &f->in_flight));

    // Acquire image; if OUT_OF_DATE, mark for recreate and re-signal the fence
    VkResult ar = vkAcquireNextImageKHR(c->device, c->swapchain, UINT64_MAX, f->image_acquired, VK_NULL_HANDLE, &c->image_index);
    if (ar == VK_ERROR_OUT_OF_DATE_KHR)
    {
        c->need_recreate = true;

        // Re-signal the fence so the next begin_frame won't block forever.
        VkSubmitInfo2 empty = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2};
        RG_CHECK(vkQueueSubmit2(c->q_graphics, 1, &empty, f->in_flight));
        return RG_SUCCESS;
    }
    else if (ar != VK_SUCCESS && ar != VK_SUBOPTIMAL_KHR)
    {
        fprintf(stderr, "vkAcquireNextImageKHR failed: %d\n", ar);
        return RG_ERR_BACKEND;
    }

    RG_CHECK(vkResetCommandBuffer(f->cmd, 0));
    VkCommandBufferBeginInfo bi = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                                   .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};
    RG_CHECK(vkBeginCommandBuffer(f->cmd, &bi));

    return RG_SUCCESS;
}

static RgResult rg_draw(RgHandle* handle, const RgFrameData* frame)
{
    RgContext* c = (RgContext*) handle;
    if (!c)
        return RG_ERR_BAD_ARG;
    const float* clear = frame ? frame->clear_rgba : (float[4]) {0, 0, 0, 1};

    RgFrameSync* f = &c->frames[c->frame_index];
    VkImage image = c->swap_images[c->image_index];
    VkImageView view = c->swap_views[c->image_index];

    // To COLOR_ATTACHMENT_OPTIMAL
    VkImageMemoryBarrier2 pre = {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                                 .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
                                 .srcAccessMask = 0,
                                 .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                 .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                                 .oldLayout = c->swap_layouts[c->image_index],
                                 .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                 .image = image,
                                 .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
    VkDependencyInfo dep_pre = {.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &pre};
    vkCmdPipelineBarrier2(f->cmd, &dep_pre);

    VkClearValue clearVal = {.color = {.float32 = {clear[0], clear[1], clear[2], clear[3]}}};
    VkRenderingAttachmentInfo color_att = {.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                                           .imageView = view,
                                           .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                           .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                           .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                           .clearValue = clearVal};
    VkRenderingInfo ri = {.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
                          .renderArea = {{0, 0}, c->swap_extent},
                          .layerCount = 1,
                          .colorAttachmentCount = 1,
                          .pColorAttachments = &color_att};
    vkCmdBeginRendering(f->cmd, &ri);
    vkCmdEndRendering(f->cmd);

    // To PRESENT
    VkImageMemoryBarrier2 post = {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                                  .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                  .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                                  .dstStageMask = VK_PIPELINE_STAGE_2_NONE,
                                  .dstAccessMask = 0,
                                  .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                  .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                  .image = image,
                                  .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
    VkDependencyInfo dep_post = {.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO, .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &post};
    vkCmdPipelineBarrier2(f->cmd, &dep_post);

    c->swap_layouts[c->image_index] = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    return RG_SUCCESS;
}

static RgResult rg_end_frame(RgHandle* handle)
{
    RgContext* c = (RgContext*) handle;
    if (!c)
        return RG_ERR_BAD_ARG;

    RgFrameSync* f = &c->frames[c->frame_index];

    RG_CHECK(vkEndCommandBuffer(f->cmd));

    VkCommandBufferSubmitInfo cbsi = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO, .commandBuffer = f->cmd};
    VkSemaphoreSubmitInfo wait_sems = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                                       .semaphore = f->image_acquired,
                                       .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphoreSubmitInfo signal_sems = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                                         .semaphore = c->present_complete[c->image_index],
                                         .stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT};
    VkSubmitInfo2 si2 = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
                         .waitSemaphoreInfoCount = 1,
                         .pWaitSemaphoreInfos = &wait_sems,
                         .commandBufferInfoCount = 1,
                         .pCommandBufferInfos = &cbsi,
                         .signalSemaphoreInfoCount = 1,
                         .pSignalSemaphoreInfos = &signal_sems};
    RG_CHECK(vkQueueSubmit2(c->q_graphics, 1, &si2, f->in_flight));

    VkPresentInfoKHR pi = {.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                           .waitSemaphoreCount = 1,
                           .pWaitSemaphores = &c->present_complete[c->image_index],
                           .swapchainCount = 1,
                           .pSwapchains = &c->swapchain,
                           .pImageIndices = &c->image_index};
    VkResult pr = vkQueuePresentKHR(c->q_present, &pi);
    if (pr == VK_ERROR_OUT_OF_DATE_KHR || pr == VK_SUBOPTIMAL_KHR)
    {
        c->need_recreate = true;
    }
    else if (pr != VK_SUCCESS)
    {
        fprintf(stderr, "vkQueuePresentKHR failed: %d\n", pr);
        return RG_ERR_BACKEND;
    }

    c->frame_index = (c->frame_index + 1) % RG_FRAMES_IN_FLIGHT;
    return RG_SUCCESS;
}

static const RgApi g_rg_api = {.api_version = RG_API_VERSION,
                               .create = rg_create,
                               .destroy = rg_destroy,
                               .resize = rg_resize,
                               .begin_frame = rg_begin_frame,
                               .draw = rg_draw,
                               .end_frame = rg_end_frame};

const RgApi* recon_renderer_vk_api(void)
{
    return &g_rg_api;
}
