#include "graphics.h"
#include "../common/log.h"
#include "../shaders/shaders.h"
#include "client.h"

#define VK_NO_PROTOTYPES
#include "../third_party/volk.h"

#define VULKAN_H_
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include "vk_mem_alloc.h"

#include <assert.h>
#include <float.h>
#include <minmax.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum
{
    BUFFERED_FRAME_COUNT = 2,
    MAX_SWAPCHAIN_IMAGES = 3,
    MAX_THREADS = 64,
    MAX_SPRITES = 2048,

    DEFAULT_MAX_SAMPLED_IMAGES = 2048,
    DEFAULT_MAX_STORAGE_BUFFERS = 2,
    DEFAULT_MAX_SAMPLERS = 4,
};

typedef struct
{
    VkBuffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo allocationInfo;
    VkDeviceSize currentOffset;
} WbBuffer;

typedef struct
{
    VkImage image;
    VmaAllocation allocation;
    VmaAllocationInfo allocationInfo;
    VkImageView imageView;
    VkFormat format;
} WbTexture;

typedef struct
{
    VkImage image;
    VmaAllocation allocation;
    VmaAllocationInfo allocationInfo;
    VkImageView imageView;
} WbRenderTarget;

typedef struct
{
    VkDescriptorSetLayout descriptor_set_layout;
    VkDescriptorPool descriptor_pool;
    VkDescriptorSet descriptor_set;
} WbDescriptorHeap;

static VkInstance s_instance;
static VkSurfaceKHR s_surface;
static VkPhysicalDevice s_physical_device;
static VkPhysicalDeviceProperties2 s_physical_device_properties2;
static u32 s_graphics_family_index;
static u32 s_transfer_family_index;
static VkDevice s_device;
static VkQueue s_graphics_queue;
static VkQueue s_transfer_queue;
static VmaAllocator s_allocator;
static VkSwapchainKHR s_swapchain;
static u32 s_swapchain_image_count;
static VkImage s_swapchain_images[MAX_SWAPCHAIN_IMAGES];
static VkImageView s_swapchain_image_views[MAX_SWAPCHAIN_IMAGES];
static VkSemaphore s_image_available_semaphore;
static VkSemaphore s_submit_complete_semaphores[BUFFERED_FRAME_COUNT];
static VkFence s_submit_complete_fences[BUFFERED_FRAME_COUNT];
static VkCommandPool s_command_pools[BUFFERED_FRAME_COUNT];
static VkCommandBuffer s_primary_command_buffers[BUFFERED_FRAME_COUNT];
static VkPipelineCache s_pipeline_cache;

static VkShaderModule s_sprite_vert_shader;
static VkShaderModule s_sprite_frag_shader;
static VkPipelineLayout s_sprite_pipeline_layout;
static VkPipeline s_sprite_pipeline;

static VkDescriptorSetLayout s_storage_buffer_descriptor_set_layout;
static VkDescriptorPool s_storage_buffer_descriptor_pool;
static VkDescriptorSet s_storage_buffer_descriptor_set;

static VkDescriptorSetLayout s_sampled_image_descriptor_set_layout;
static VkDescriptorPool s_sampled_image_descriptor_pool;
static VkDescriptorSet s_sampled_image_descriptor_set;

static VkDescriptorSetLayout s_sampler_descriptor_set_layout;
static VkDescriptorPool s_sampler_descriptor_pool;
static VkDescriptorSet s_sampler_descriptor_set;

static VkSampler s_point_clamp_sampler;
static VkSampler s_point_repeat_sampler;

static VkBuffer s_sprite_buffer;
static VmaAllocation s_sprite_buffer_allocation;

static wb_push_constants push_constants;

static bool s_vsync;
static u32 s_width;
static u32 s_height;
static u32 s_buffered_frame_index;
static u32 s_swapchain_image_index;

static u32 s_sprite_count;

static const VkFormat k_swapchain_format = VK_FORMAT_B8G8R8A8_UNORM;
static const char* k_pipeline_cache_name = "pipeline_cache.bin";

static void createInstance(void);
static void pickPhysicalDevice(void);
static void createSurface(void* hwnd, void* hinstance);
static void createDevice(void);
static void createAllocator(void);
static void createSwapchain(u32 width, u32 height, bool vsync);
static void recreateSwapchain(u32 width, u32 height, bool vsync);
static void createSynchronization(void);
static void createCommandPools(void);
static void createSamplers(void);
static void createDescriptorSets(void);
static void createShaders(void);
static void createGraphicsPipeline(void);

static void loadPipelineCache(const char* fileName);
static void savePipelineCache(const char* fileName);

static void createTexture(u32 width, u32 height, const u8* data, VkFormat format, WbTexture* texture);
static void createRenderTarget(u32 width, u32 height, VkSampleCountFlagBits sampleCount, WbRenderTarget* renderTarget);
static void updateTextureSampler(VkSampler sampler, VkImageView imageView, VkDescriptorSet descriptorSet);

void wb_graphics_init(const WbGraphicsDesc* graphics_desc)
{
    assert(graphics_desc);

    s_vsync = graphics_desc->vsync;
    s_width = graphics_desc->window_width;
    s_height = graphics_desc->window_height;

    createInstance();
    pickPhysicalDevice();
    createSurface(graphics_desc->hwnd, graphics_desc->hinstance);
    createDevice();
    createAllocator();
    createSwapchain(graphics_desc->window_width, graphics_desc->window_height, graphics_desc->vsync);
    createSynchronization();
    createCommandPools();
    createSamplers();
    createDescriptorSets();
    loadPipelineCache(k_pipeline_cache_name);
    createShaders();
    createGraphicsPipeline();

    const VkBufferCreateInfo buffer_info =
    {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .size = sizeof(wb_sprite) * MAX_SPRITES };
    const VmaAllocationCreateInfo allocation_info =
    {
            .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
    };
    VkResult result = vmaCreateBuffer(s_allocator, &buffer_info, &allocation_info, &s_sprite_buffer, &s_sprite_buffer_allocation, NULL);
    assert(result == VK_SUCCESS);

    wb_sprite sprites[] = {
            {.x = 640.0f - 60,
             .y = 360.0f - 40,
             .width = 120.0f,
             .height = 80.0f,
             .texture_index = 1,
             .color = 0xFFFFFFFF} };

    s_sprite_count = sizeof(sprites) / sizeof(sprites[0]);

    void* data;
    result = vmaMapMemory(s_allocator, s_sprite_buffer_allocation, &data);
    assert(result == VK_SUCCESS);

    memcpy(data, sprites, sizeof(sprites));

    vmaUnmapMemory(s_allocator, s_sprite_buffer_allocation);
}

void wb_graphics_quit(void)
{
    savePipelineCache(k_pipeline_cache_name);
}

void wb_graphics_draw(void)
{
    VkResult result;

    result = vkWaitForFences(s_device, 1, &s_submit_complete_fences[s_buffered_frame_index], VK_TRUE, UINT64_MAX);
    assert(result == VK_SUCCESS);
    result = vkResetFences(s_device, 1, &s_submit_complete_fences[s_buffered_frame_index]);
    assert(result == VK_SUCCESS);

    result = vkAcquireNextImageKHR(s_device, s_swapchain, UINT64_MAX,
                                   s_image_available_semaphore, VK_NULL_HANDLE, &s_swapchain_image_index);
    assert(result == VK_SUCCESS);

    vkResetCommandPool(s_device, s_command_pools[s_buffered_frame_index], 0);

    VkCommandBuffer cmd = s_primary_command_buffers[s_buffered_frame_index];

    result = vkBeginCommandBuffer(cmd, &(const VkCommandBufferBeginInfo){
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    });
    assert(result == VK_SUCCESS);

    vkCmdPipelineBarrier(cmd,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         0, 0, NULL, 0, NULL, 1, &(const VkImageMemoryBarrier){
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .srcAccessMask = VK_ACCESS_NONE,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .image = s_swapchain_images[s_swapchain_image_index],
            .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .levelCount = 1,
                    .layerCount = 1,
        },
    });

    vkCmdBeginRendering(cmd, &(const VkRenderingInfo){
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea = { .extent = {s_width, s_height} },
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &(const VkRenderingAttachmentInfo) {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView = s_swapchain_image_views[s_swapchain_image_index],
                .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = { .color = {.float32 = {0.4f, 0.6f, 0.9f, 1.0f}} },
        },
    });

    vkCmdSetViewport(cmd, 0, 1, &(const VkViewport){
        .width = (float) s_width,
            .height = (float) s_height,
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
    });

    vkCmdSetScissor(cmd, 0, 1, &(const VkRect2D){.extent = { s_width, s_height }});

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, s_sprite_pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, s_sprite_pipeline_layout, 0, 3, (const VkDescriptorSet[]) {
        s_storage_buffer_descriptor_set,
            s_sampled_image_descriptor_set,
            s_sampler_descriptor_set
    }, 0, NULL);

    const VkDescriptorBufferInfo buffer_info = (VkDescriptorBufferInfo){
            .buffer = s_sprite_buffer,
            .offset = 0,
            .range = VK_WHOLE_SIZE,
    };

    const VkWriteDescriptorSet write_buffer_set = (VkWriteDescriptorSet){
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = s_storage_buffer_descriptor_set,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .dstBinding = 0,
            .descriptorCount = 1,
            .pBufferInfo = &buffer_info,
    };
    vkUpdateDescriptorSets(s_device, 1, &write_buffer_set, 0, NULL);

    push_constants.camera_x = 100.0f;
    push_constants.camera_y = 100.0f;
    push_constants.window_width = 1280.0f;
    push_constants.window_height = 720.0f;

    vkCmdPushConstants(cmd, s_sprite_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(wb_push_constants), &push_constants);

    vkCmdDraw(cmd, s_sprite_count * 6, 1, 0, 0);

    vkCmdEndRendering(cmd);

    vkCmdPipelineBarrier(cmd,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                         0, 0, NULL, 0, NULL, 1, &(const VkImageMemoryBarrier){
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dstAccessMask = 0,
            .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .image = s_swapchain_images[s_swapchain_image_index],
            .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .levelCount = 1,
                    .layerCount = 1,
        },
    });

    result = vkEndCommandBuffer(cmd);
    assert(result == VK_SUCCESS);

    result = vkQueueSubmit(s_graphics_queue, 1, &(const VkSubmitInfo){
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &cmd,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &s_image_available_semaphore,
            .pWaitDstStageMask = &(const VkPipelineStageFlags) { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT },
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &s_submit_complete_semaphores[s_buffered_frame_index],
    },
                           s_submit_complete_fences[s_buffered_frame_index]);
    assert(result == VK_SUCCESS);

    result = vkQueuePresentKHR(s_graphics_queue, &(const VkPresentInfoKHR){
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &s_submit_complete_semaphores[s_buffered_frame_index],
            .swapchainCount = 1,
            .pSwapchains = &s_swapchain,
            .pImageIndices = &s_swapchain_image_index,
    });
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        vkDeviceWaitIdle(s_device);
        createSwapchain(s_width, s_height, s_vsync);
    }

    s_buffered_frame_index = (s_buffered_frame_index + 1) % BUFFERED_FRAME_COUNT;
}

static void createInstance()
{
    VkResult result = volkInitialize();
    assert(result == VK_SUCCESS);

    u32 vk_instance_version = volkGetInstanceVersion();
    wb_log_info("Vulkan instance version: %d.%d.%d", VK_API_VERSION_MAJOR(vk_instance_version),
                VK_API_VERSION_MINOR(vk_instance_version), VK_API_VERSION_PATCH(vk_instance_version));

    const char* instance_extensions[] = { "VK_KHR_surface", "VK_KHR_win32_surface" };

    result = vkCreateInstance(&(const VkInstanceCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &(const VkApplicationInfo) {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                .pApplicationName = "Open WBC",
                .applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 0),
                .pEngineName = "Open WBC",
                .engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0),
                .apiVersion = VK_API_VERSION_1_3,
        },
            .enabledExtensionCount = sizeof(instance_extensions) / sizeof(instance_extensions[0]),
                .ppEnabledExtensionNames = instance_extensions,
    },
                              NULL, & s_instance);
    assert(result == VK_SUCCESS);

    volkLoadInstanceOnly(s_instance);
}

static void pickPhysicalDevice()
{
    VkResult result;
    u32 physical_device_count;
    result = vkEnumeratePhysicalDevices(s_instance, &physical_device_count, NULL);
    assert(result == VK_SUCCESS);

    VkPhysicalDevice physical_devices[4];
    result = vkEnumeratePhysicalDevices(s_instance, &physical_device_count, physical_devices);
    assert(result == VK_SUCCESS);

    // TODO: Select best GPU
    s_physical_device = physical_devices[0];

    s_physical_device_properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    vkGetPhysicalDeviceProperties2(s_physical_device, &s_physical_device_properties2);
    wb_log_info("Physical device: %s", s_physical_device_properties2.properties.deviceName);
}

static void createSurface(void* hwnd, void* hinstance)
{
    VkResult result = vkCreateWin32SurfaceKHR(s_instance, &(const VkWin32SurfaceCreateInfoKHR){.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, .hwnd = hwnd, .hinstance = hinstance}, NULL, & s_surface);
    assert(result == VK_SUCCESS);

    u32 surface_format_count = 0;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(s_physical_device, s_surface, &surface_format_count, NULL);
    assert(result == VK_SUCCESS);

    VkSurfaceFormatKHR surface_formats[8];
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(s_physical_device, s_surface, &surface_format_count,
                                                  surface_formats);
    assert(result == VK_SUCCESS);

    bool surface_format_supported = false;
    for (u32 i = 0; i < surface_format_count; i++)
    {
        if (surface_formats[i].format == k_swapchain_format)
        {
            surface_format_supported = true;
            break;
        }
    }
    assert(surface_format_supported);
}

static u32 getQueueFamilyIndex(VkQueueFlags queue_flags, u32 queue_family_count,
                               const VkQueueFamilyProperties* queue_family_properties)
{
    VkQueueFlags min_flags = ~0u;
    u32 best_family = UINT32_MAX;
    for (u32 i = 0; i < queue_family_count; i++)
    {
        if ((queue_family_properties[i].queueFlags & queue_flags) == queue_flags)
        {
            if (queue_family_properties[i].queueFlags < min_flags)
            {
                min_flags = queue_family_properties[i].queueFlags;
                best_family = i;
            }
        }
    }
    return best_family;
}

static void createDevice()
{
    VkResult result;
    u32 queue_family_count;
    vkGetPhysicalDeviceQueueFamilyProperties(s_physical_device, &queue_family_count, NULL);
    VkQueueFamilyProperties queue_family_properties[8];
    vkGetPhysicalDeviceQueueFamilyProperties(s_physical_device, &queue_family_count, queue_family_properties);

    s_graphics_family_index = getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT, queue_family_count,
                                                  queue_family_properties);
    s_transfer_family_index = getQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT, queue_family_count,
                                                  queue_family_properties);

    const char* device_extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkPhysicalDeviceDescriptorIndexingFeatures descriptor_indexing_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT,
    };
    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
            .pNext = &descriptor_indexing_features,
    };
    VkPhysicalDeviceFeatures2 supported_features = {
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = &dynamic_rendering_features };
    vkGetPhysicalDeviceFeatures2(s_physical_device, &supported_features);

    if (descriptor_indexing_features.runtimeDescriptorArray == VK_FALSE)
    {
        wb_log_error("GPU doesn't support Descriptor Indexing feature: runtimeDescriptorArray.");
        return;
    }
    if (descriptor_indexing_features.descriptorBindingPartiallyBound == VK_FALSE)
    {
        wb_log_error("GPU doesn't support Descriptor Indexing feature: descriptorBindingPartiallyBound.");
        return;
    }
    if (dynamic_rendering_features.dynamicRendering == VK_FALSE)
    {
        wb_log_error("GPU doesn't support Dynamic Rendering feature: dynamicRendering.");
        return;
    }

    descriptor_indexing_features = (VkPhysicalDeviceDescriptorIndexingFeatures){
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT,
            .descriptorBindingPartiallyBound = VK_TRUE,
            .runtimeDescriptorArray = VK_TRUE,
            .descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
            .descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE,
    };

    dynamic_rendering_features = (VkPhysicalDeviceDynamicRenderingFeatures){
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
            .pNext = &descriptor_indexing_features,
            .dynamicRendering = VK_TRUE,
    };
    VkPhysicalDeviceFeatures2 features2 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = &dynamic_rendering_features,
            .features = {
                    .textureCompressionBC = VK_TRUE,
            },
    };

    result = vkCreateDevice(s_physical_device, &(const VkDeviceCreateInfo){
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &features2,
            .queueCreateInfoCount = 2,
            .pQueueCreateInfos = (const VkDeviceQueueCreateInfo[]){ {
                                                                           .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                                                                           .pQueuePriorities = (const float[]){1.0f},
                                                                           .queueCount = 1,
                                                                           .queueFamilyIndex = s_graphics_family_index,
                                                                   },
                                                                   {
                                                                           .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                                                                           .pQueuePriorities = (const float[]){1.0f},
                                                                           .queueCount = 1,
                                                                           .queueFamilyIndex = s_transfer_family_index,
                                                                   } },
                                                                   .enabledExtensionCount = 1,
                                                                   .ppEnabledExtensionNames = device_extensions,
    },
    NULL, & s_device);
    assert(result == VK_SUCCESS);

    volkLoadDevice(s_device);

    vkGetDeviceQueue(s_device, s_graphics_family_index, 0, &s_graphics_queue);
    vkGetDeviceQueue(s_device, s_transfer_family_index, 0, &s_transfer_queue);
}

static void createAllocator()
{
    VkResult result = vmaCreateAllocator(&(const VmaAllocatorCreateInfo) {
        .physicalDevice = s_physical_device,
            .device = s_device,
            .instance = s_instance,
            .vulkanApiVersion = VK_API_VERSION_1_3,
            .pVulkanFunctions = &(const VmaVulkanFunctions) {
            .vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties,
                .vkAllocateMemory = vkAllocateMemory,
                .vkBindBufferMemory = vkBindBufferMemory,
                .vkBindBufferMemory2KHR = vkBindBufferMemory2,
                .vkBindImageMemory = vkBindImageMemory,
                .vkBindImageMemory2KHR = vkBindImageMemory2,
                .vkCmdCopyBuffer = vkCmdCopyBuffer,
                .vkCreateBuffer = vkCreateBuffer,
                .vkCreateImage = vkCreateImage,
                .vkDestroyBuffer = vkDestroyBuffer,
                .vkDestroyImage = vkDestroyImage,
                .vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges,
                .vkFreeMemory = vkFreeMemory,
                .vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements,
                .vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2,
                .vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements,
                .vkGetDeviceImageMemoryRequirements = vkGetDeviceImageMemoryRequirements,
                .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
                .vkGetImageMemoryRequirements = vkGetImageMemoryRequirements,
                .vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2,
                .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
                .vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties,
                .vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2,
                .vkInvalidateMappedMemoryRanges = vkFlushMappedMemoryRanges,
                .vkMapMemory = vkMapMemory,
                .vkUnmapMemory = vkUnmapMemory,
        },
    },
                                         & s_allocator);
    assert(result == VK_SUCCESS);
}

static void createSwapchain(u32 width, u32 height, bool vsync)
{
    //    VkSurfaceFullScreenExclusiveInfoEXT fullscreen_exclusive_info = {
    //        .sType = VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT,
    //        .fullScreenExclusive = VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT
    //    };

    VkResult result;

    // Get surface properties
    VkBool32 supports_present;
    result = vkGetPhysicalDeviceSurfaceSupportKHR(s_physical_device, s_graphics_family_index, s_surface,
                                                  &supports_present);
    assert(result == VK_SUCCESS);
    assert(supports_present == VK_TRUE);

    VkSurfaceCapabilitiesKHR surface_capabilities;
    result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(s_physical_device, s_surface, &surface_capabilities);
    assert(result == VK_SUCCESS);

    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
    {
        u32 present_mode_count;
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(s_physical_device, s_surface, &present_mode_count, NULL);
        assert(result == VK_SUCCESS);

        VkPresentModeKHR present_modes[8];
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(s_physical_device, s_surface, &present_mode_count,
                                                           present_modes);
        assert(result == VK_SUCCESS);

        VkPresentModeKHR preferred_present_mode = vsync
            ? VK_PRESENT_MODE_MAILBOX_KHR
            : VK_PRESENT_MODE_IMMEDIATE_KHR;
        for (u32 i = 0; i < present_mode_count; i++)
        {
            if (present_modes[i] == preferred_present_mode)
            {
                present_mode = preferred_present_mode;
                break;
            }
        }
    }

    VkExtent2D extent = { 0 };
    if (surface_capabilities.currentExtent.width == UINT32_MAX ||
        surface_capabilities.currentExtent.height == UINT32_MAX)
    {
        wb_client_get_window_size(&width, &height);
        extent.width = width;
        extent.height = height;
    }
    else
    {
        extent = surface_capabilities.currentExtent;
    }
    extent.width = max(surface_capabilities.minImageExtent.width,
                       min(surface_capabilities.maxImageExtent.width, extent.width));
    extent.height = max(surface_capabilities.minImageExtent.height,
                        min(surface_capabilities.maxImageExtent.height, extent.height));

    VkSwapchainKHR old_swapchain = s_swapchain;

    result = vkCreateSwapchainKHR(s_device, &(const VkSwapchainCreateInfoKHR){
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext = NULL,// TODO: Handle exclusive fullscreen
            .surface = s_surface,
            .minImageCount = vsync ? 3 : 2,
            .imageFormat = k_swapchain_format,
            .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
            .imageExtent = extent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = present_mode,
            .clipped = VK_TRUE,
            .oldSwapchain = old_swapchain,
    },
                                  NULL, & s_swapchain);
    assert(result == VK_SUCCESS);

    if (old_swapchain != VK_NULL_HANDLE)
    {
        for (u32 i = 0; i < s_swapchain_image_count; i++)
        {
            vkDestroyImageView(s_device, s_swapchain_image_views[i], NULL);
        }

        vkDestroySwapchainKHR(s_device, old_swapchain, NULL);
    }

    result = vkGetSwapchainImagesKHR(s_device, s_swapchain, &s_swapchain_image_count, NULL);
    assert(result == VK_SUCCESS);
    result = vkGetSwapchainImagesKHR(s_device, s_swapchain, &s_swapchain_image_count, s_swapchain_images);
    assert(result == VK_SUCCESS);

    wb_log_info("Vulkan swapchain created with size %dx%d and %d images.", extent.width, extent.height,
                s_swapchain_image_count);

    for (u32 i = 0; i < s_swapchain_image_count; i++)
    {
        result = vkCreateImageView(s_device, &(const VkImageViewCreateInfo){.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, .format = k_swapchain_format, .image = s_swapchain_images[i], .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .subresourceRange.levelCount = 1, .subresourceRange.layerCount = 1, .viewType = VK_IMAGE_VIEW_TYPE_2D}, NULL, & s_swapchain_image_views[i]);
        assert(result == VK_SUCCESS);
    }
}

static void recreateSwapchain(u32 width, u32 height, bool vsync)
{
    VkResult result = vkDeviceWaitIdle(s_device);
    assert(result == VK_SUCCESS);

    createSwapchain(width, height, vsync);
}

static void createSynchronization()
{
    VkResult result;
    const VkFenceCreateInfo fence_info = {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT };
    const VkSemaphoreCreateInfo semaphore_info = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    result = vkCreateSemaphore(s_device, &semaphore_info, NULL, &s_image_available_semaphore);
    assert(result == VK_SUCCESS);

    for (uint32_t i = 0; i < BUFFERED_FRAME_COUNT; ++i)
    {
        result = vkCreateFence(s_device, &fence_info, NULL, &s_submit_complete_fences[i]);
        assert(result == VK_SUCCESS);
        result = vkCreateSemaphore(s_device, &semaphore_info, NULL, &s_submit_complete_semaphores[i]);
        assert(result == VK_SUCCESS);
    }
}

static void createCommandPools()
{
    VkResult result;
    for (u32 i = 0; i < BUFFERED_FRAME_COUNT; i++)
    {
        result = vkCreateCommandPool(s_device, &(const VkCommandPoolCreateInfo){
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .queueFamilyIndex = s_graphics_family_index,
                .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
        },
                                     NULL, & s_command_pools[i]);
        assert(result == VK_SUCCESS);

        result = vkAllocateCommandBuffers(s_device, &(const VkCommandBufferAllocateInfo){
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = s_command_pools[i],
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1,
        },
                                          & s_primary_command_buffers[i]);
        assert(result == VK_SUCCESS);
    }
}

static void createSamplers()
{
    VkResult result;
    VkSamplerCreateInfo sampler_info = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .minFilter = VK_FILTER_NEAREST,
            .magFilter = VK_FILTER_NEAREST,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
            .minLod = 0.0f,
            .maxLod = FLT_MAX,
    };

    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    result = vkCreateSampler(s_device, &sampler_info, NULL, &s_point_clamp_sampler);
    assert(result == VK_SUCCESS);

    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    result = vkCreateSampler(s_device, &sampler_info, NULL, &s_point_repeat_sampler);
    assert(result == VK_SUCCESS);
}

static void createDescriptorSets()
{
    VkResult result;

    result = vkCreateDescriptorPool(s_device, &(const VkDescriptorPoolCreateInfo){
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
            .maxSets = 1,
            .poolSizeCount = 1,
            .pPoolSizes = &(const VkDescriptorPoolSize) { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, DEFAULT_MAX_STORAGE_BUFFERS },
    },
                                    NULL, & s_storage_buffer_descriptor_pool);
    assert(result == VK_SUCCESS);

    result = vkCreateDescriptorPool(s_device, &(const VkDescriptorPoolCreateInfo){
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
            .maxSets = 1,
            .poolSizeCount = 1,
            .pPoolSizes = &(const VkDescriptorPoolSize) { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, DEFAULT_MAX_SAMPLED_IMAGES },
    },
                                    NULL, & s_sampled_image_descriptor_pool);
    assert(result == VK_SUCCESS);

    result = vkCreateDescriptorPool(s_device, &(const VkDescriptorPoolCreateInfo){
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
            .maxSets = 1,
            .poolSizeCount = 1,
            .pPoolSizes = &(const VkDescriptorPoolSize) { VK_DESCRIPTOR_TYPE_SAMPLER, 2 },
    },
                                    NULL, & s_sampler_descriptor_pool);
    assert(result == VK_SUCCESS);

    VkDescriptorSetLayoutBindingFlagsCreateInfo binding_flags_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .bindingCount = 1,
            .pBindingFlags = &(const VkDescriptorBindingFlags) { VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT } };

    VkDescriptorSetLayoutCreateInfo set_layout_create_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = &binding_flags_info,
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
            .bindingCount = 1,
    };

    set_layout_create_info.pBindings = &(const VkDescriptorSetLayoutBinding) {
        .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = DEFAULT_MAX_STORAGE_BUFFERS,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
    };
    result = vkCreateDescriptorSetLayout(s_device, &set_layout_create_info, NULL, &s_storage_buffer_descriptor_set_layout);
    assert(result == VK_SUCCESS);

    set_layout_create_info.pBindings = &(const VkDescriptorSetLayoutBinding) {
        .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = DEFAULT_MAX_SAMPLED_IMAGES,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
    };
    result = vkCreateDescriptorSetLayout(s_device, &set_layout_create_info, NULL, &s_sampled_image_descriptor_set_layout);
    assert(result == VK_SUCCESS);

    set_layout_create_info.pNext = NULL;
    set_layout_create_info.pBindings = &(const VkDescriptorSetLayoutBinding) {
        .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = 2,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = (const VkSampler[]){ s_point_clamp_sampler, s_point_repeat_sampler },
    };
    result = vkCreateDescriptorSetLayout(s_device, &set_layout_create_info, NULL, &s_sampler_descriptor_set_layout);
    assert(result == VK_SUCCESS);

    VkDescriptorSetAllocateInfo set_allocate_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorSetCount = 1,
    };

    set_allocate_info.descriptorPool = s_storage_buffer_descriptor_pool;
    set_allocate_info.pSetLayouts = &s_storage_buffer_descriptor_set_layout;
    result = vkAllocateDescriptorSets(s_device, &set_allocate_info, &s_storage_buffer_descriptor_set);
    assert(result == VK_SUCCESS);

    set_allocate_info.descriptorPool = s_sampled_image_descriptor_pool;
    set_allocate_info.pSetLayouts = &s_sampled_image_descriptor_set_layout;
    result = vkAllocateDescriptorSets(s_device, &set_allocate_info, &s_sampled_image_descriptor_set);
    assert(result == VK_SUCCESS);

    set_allocate_info.descriptorPool = s_sampler_descriptor_pool;
    set_allocate_info.pSetLayouts = &s_sampler_descriptor_set_layout;
    result = vkAllocateDescriptorSets(s_device, &set_allocate_info, &s_sampler_descriptor_set);
    assert(result == VK_SUCCESS);
}

static void createShaders()
{
    VkResult result;
    result = vkCreateShaderModule(s_device, &(const VkShaderModuleCreateInfo){
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = sprite_vert_spv_size,
            .pCode = (u32*) sprite_vert_spv,
    },
                                  NULL, & s_sprite_vert_shader);
    assert(result == VK_SUCCESS);

    result = vkCreateShaderModule(s_device, &(const VkShaderModuleCreateInfo){
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = sprite_frag_spv_size,
            .pCode = (u32*) sprite_frag_spv,
    },
                                  NULL, & s_sprite_frag_shader);
    assert(result == VK_SUCCESS);

    const VkPipelineLayoutCreateInfo pipeline_layout_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 3,
            .pSetLayouts = (const VkDescriptorSetLayout[]){s_storage_buffer_descriptor_set_layout, s_sampled_image_descriptor_set_layout, s_sampler_descriptor_set_layout},
            .pushConstantRangeCount = 1,
            .pPushConstantRanges = &(const VkPushConstantRange) {
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                    .offset = 0,
                    .size = sizeof(wb_push_constants),
            },
    };
    result = vkCreatePipelineLayout(s_device, &pipeline_layout_info, NULL, &s_sprite_pipeline_layout);
    assert(result == VK_SUCCESS);
}

static void createGraphicsPipeline()
{
    VkResult result;

    const VkDynamicState dynamic_states[] = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
            VK_DYNAMIC_STATE_BLEND_CONSTANTS };

    const VkPipelineRenderingCreateInfo rendering_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &k_swapchain_format,
    };

    const VkGraphicsPipelineCreateInfo pipeline_info = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &rendering_info,
            .stageCount = 2,
            .pStages = (const VkPipelineShaderStageCreateInfo[]){
                    {
                            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                            .stage = VK_SHADER_STAGE_VERTEX_BIT,
                            .module = s_sprite_vert_shader,
                            .pName = "main",
                    },
                    {
                            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                            .module = s_sprite_frag_shader,
                            .pName = "main",
                    },
            },
            .pRasterizationState = &(const VkPipelineRasterizationStateCreateInfo) {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                    .cullMode = VK_CULL_MODE_NONE,
                    .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
                    .lineWidth = 1.0f,
                    .polygonMode = VK_POLYGON_MODE_FILL,
                    .depthClampEnable = VK_FALSE,
                        .depthBiasEnable = VK_FALSE,
                    .rasterizerDiscardEnable = VK_FALSE,
            },
            .pViewportState = &(const VkPipelineViewportStateCreateInfo) { .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, .scissorCount = 1, .pScissors = &(VkRect2D) { .extent = {U16_MAX, U16_MAX} }, .viewportCount = 1, .pViewports = &(VkViewport) { .width = U16_MAX, .height = U16_MAX, .minDepth = 0.0f, .maxDepth = 1.0f } },
            .pInputAssemblyState = &(const VkPipelineInputAssemblyStateCreateInfo) { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST },
            .pDynamicState = &(const VkPipelineDynamicStateCreateInfo) {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                    .dynamicStateCount = sizeof(dynamic_states) / sizeof(dynamic_states[0]),
                    .pDynamicStates = dynamic_states,
            },
            .pColorBlendState = &(const VkPipelineColorBlendStateCreateInfo) {
.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, .attachmentCount = 1, .pAttachments = &(const VkPipelineColorBlendAttachmentState) {
                                                                                                 .colorWriteMask = 0xF,
                                                                                                 .blendEnable = VK_FALSE,
                                                                                         }
},
.pVertexInputState = &(const VkPipelineVertexInputStateCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,

},
.layout = s_sprite_pipeline_layout };

    result = vkCreateGraphicsPipelines(s_device, s_pipeline_cache, 1, &pipeline_info, NULL, &s_sprite_pipeline);
    assert(result == VK_SUCCESS);
}

static void loadPipelineCache(const char* fileName)
{
    u32 cache_size = 0;
    void* cache_data = NULL;

    FILE* file = fopen(fileName, "rb");
    if (file == NULL)
    {
        file = fopen(fileName, "wb");
        assert(file);
    }
    else
    {
        fseek(file, 0L, SEEK_END);
        cache_size = ftell(file);
        if (cache_size > 0)
        {
            rewind(file);
            cache_data = malloc(cache_size);
            assert(cache_data);

            size_t num_read = fread(cache_data, cache_size, 1, file);

            // Verify cache data
            struct
            {
                u32 length;
                u32 version;
                u32 vendor_id;
                u32 device_id;
                u8 cache_uuid[VK_UUID_SIZE];
            } cache_header = { 0 };

#pragma warning(suppress : 6385)
            memcpy(&cache_header, (u8*) cache_data, sizeof(cache_header));
            if (cache_header.length <= 0 ||
                cache_header.version != VK_PIPELINE_CACHE_HEADER_VERSION_ONE ||
                cache_header.vendor_id != s_physical_device_properties2.properties.vendorID ||
                cache_header.device_id != s_physical_device_properties2.properties.deviceID ||
                memcmp(cache_header.cache_uuid, s_physical_device_properties2.properties.pipelineCacheUUID, sizeof(cache_header.cache_uuid)) != 0)
            {
                free(cache_data);
                cache_size = 0;
                cache_data = NULL;
            }
        }
        fclose(file);
    }

    const VkPipelineCacheCreateInfo cache_info = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
            .initialDataSize = cache_size,
            .pInitialData = cache_data,
    };
    VkResult result = vkCreatePipelineCache(s_device, &cache_info, NULL, &s_pipeline_cache);
    assert(result == VK_SUCCESS);

    if (cache_size > 0)
    {
        free(cache_data);
    }
    wb_log_info("Loaded Vulkan pipeline cache of %u bytes", cache_size);
}

static void savePipelineCache(const char* fileName)
{
    if (s_pipeline_cache == VK_NULL_HANDLE)
    {
        return;
    }

    u64 cache_size = 0;
    VkResult result = vkGetPipelineCacheData(s_device, s_pipeline_cache, &cache_size, NULL);
    assert(result == VK_SUCCESS);

    if (cache_size > 0)
    {
        u8* cache_data = malloc(cache_size);
        assert(cache_data);
        result = vkGetPipelineCacheData(s_device, s_pipeline_cache, &cache_size, cache_data);
        assert(result == VK_SUCCESS);

        FILE* file = fopen(fileName, "wb");
        assert(file);

        fwrite(cache_data, cache_size, 1, file);

        wb_log_info("Saving pipeline cache of size %d bytes", (u32) cache_size);

        free(cache_data);
        fclose(file);
    }
    // TODO: Write empty file if invalid cache data
    vkDestroyPipelineCache(s_device, s_pipeline_cache, NULL);
}

static void createTexture(u32 width, u32 height, const u8* data, VkFormat format, WbTexture* texture)
{
    assert(texture);

    // TODO: Create staging buffer and map data

    VkResult result = vmaCreateImage(s_allocator, &(const VkImageCreateInfo){
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .extent.width = width,
            .extent.height = height,
            .extent.depth = 1,
            .mipLevels = 1,
            .arrayLayers = 1,
            .format = format,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .samples = VK_SAMPLE_COUNT_1_BIT,
    },
                                     & (const VmaAllocationCreateInfo) {
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
    },
            & texture->image, & texture->allocation, & texture->allocationInfo);
    assert(result == VK_SUCCESS);

    result = vkCreateImageView(s_device, &(const VkImageViewCreateInfo){.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, .image = texture->image, .viewType = VK_IMAGE_VIEW_TYPE_2D, .format = format, .components.r = VK_COMPONENT_SWIZZLE_IDENTITY, .components.g = VK_COMPONENT_SWIZZLE_IDENTITY, .components.b = VK_COMPONENT_SWIZZLE_IDENTITY, .components.a = VK_COMPONENT_SWIZZLE_IDENTITY, .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .subresourceRange.baseArrayLayer = 0, .subresourceRange.baseMipLevel = 0, .subresourceRange.layerCount = 1, .subresourceRange.levelCount = 1}, NULL, & texture->imageView);
    assert(result == VK_SUCCESS);
}

static void createRenderTarget(u32 width, u32 height, VkSampleCountFlagBits sampleCount, WbRenderTarget* renderTarget)
{
    assert(renderTarget);

    VkResult result = vmaCreateImage(s_allocator, &(const VkImageCreateInfo){
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .extent.width = width,
            .extent.height = height,
            .extent.depth = 1,
            .mipLevels = 1,
            .arrayLayers = 1,
            .format = k_swapchain_format,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .samples = sampleCount,
    },
                                     & (const VmaAllocationCreateInfo) {
        .usage = VMA_MEMORY_USAGE_AUTO,
            .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
            .priority = 1.0f,
    },
            & renderTarget->image, & renderTarget->allocation, & renderTarget->allocationInfo);
    assert(result == VK_SUCCESS);

    result = vkCreateImageView(s_device, &(const VkImageViewCreateInfo){.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, .image = renderTarget->image, .viewType = VK_IMAGE_VIEW_TYPE_2D, .format = k_swapchain_format, .components.r = VK_COMPONENT_SWIZZLE_IDENTITY, .components.g = VK_COMPONENT_SWIZZLE_IDENTITY, .components.b = VK_COMPONENT_SWIZZLE_IDENTITY, .components.a = VK_COMPONENT_SWIZZLE_IDENTITY, .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .subresourceRange.baseArrayLayer = 0, .subresourceRange.baseMipLevel = 0, .subresourceRange.layerCount = 1, .subresourceRange.levelCount = 1}, NULL, & renderTarget->imageView);
    assert(result == VK_SUCCESS);
}

static void updateTextureSampler(VkSampler sampler, VkImageView imageView, VkDescriptorSet descriptorSet)
{
    assert(sampler != VK_NULL_HANDLE);

    vkUpdateDescriptorSets(s_device, 1, &(const VkWriteDescriptorSet){
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptorSet,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &(const VkDescriptorImageInfo) { .sampler = sampler, .imageView = imageView, .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL },
    },
                           0, NULL);
}