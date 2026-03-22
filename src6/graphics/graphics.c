#include "graphics.h"

#include <d3d12.h>

#define VK_NO_PROTOTYPES
#include "../../third_party/volk/volk.h"

#ifndef VULKAN_H
#define VULKAN_H
#endif
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include "vk_mem_alloc.h"

enum
{
	GFX_MAX_FRAMES_IN_FLIGHT = 3
};

struct gfx_context
{
	VkInstance instance;
	VkPhysicalDevice physical_device;
	VkPhysicalDeviceProperties2 physical_device_properties2;
	VkPhysicalDeviceFeatures2 physical_device_features2;
	VkDevice device;
	VmaAllocator allocator;

	u32 graphics_queue_family_index;
	u32 transfer_queue_family_index;
	VkQueue graphics_queue;
	VkQueue transfer_queue;

	VkSurfaceKHR surface;
	VkSwapchainKHR swapchain;
	VkImage swapchain_images[GFX_MAX_FRAMES_IN_FLIGHT];
	VkImageView swapchain_image_views[GFX_MAX_FRAMES_IN_FLIGHT];

	bool vsync;
	VkFormat swapchain_format;
	u32 swapchain_image_count;
	VkExtent2D swapchain_extent;
	VkClearColorValue clear_color;

	VkSemaphore image_available_semaphore;
	VkSemaphore submit_complete_semaphores[GFX_MAX_FRAMES_IN_FLIGHT];
	VkFence submit_complete_fences[GFX_MAX_FRAMES_IN_FLIGHT];

	VkCommandPool graphics_command_pools[GFX_MAX_FRAMES_IN_FLIGHT];
	VkCommandPool transfer_command_pool;

	VkDescriptorSetLayout sampler_descriptor_set_layout;
	VkDescriptorSetLayout sampled_image_descriptor_set_layout;
	VkDescriptorSetLayout storage_buffer_descriptor_set_layout;

	VkDescriptorPool sampler_descriptor_pool;
	VkDescriptorPool sampled_image_descriptor_pool;
	VkDescriptorPool storage_buffer_descriptor_pool;

	VkDescriptorSet sampler_descriptor_set;
	VkDescriptorSet sampled_image_descriptor_set;
	VkDescriptorSet storage_buffer_descriptor_set;

	VkPipelineCache pipeline_cache;

#if defined(DEBUG)
	VkDebugUtilsMessengerEXT debug_utils_messenger;
#endif
};

struct gfx_buffer
{
	VkBuffer buffer;
	VmaAllocation allocation;
	VmaAllocationInfo allocation_info;
	u64 size;
	u8* mapped_data;
	bool mapped;
};

struct gfx_texture
{
	VkImage image;
	VmaAllocation allocation;
	VmaAllocationInfo allocation_info;
	u32 width;
	u32 height;
	u8* mapped_data;
	VkFormat format;
	bool mapped;
};

void gfx_init(const gfx_desc* desc)
{

}

void gfx_quit(void)
{

}

void gfx_draw(void)
{

}

void gfx_resize(u32 width, u32 height, bool vsync)
{

}

gfx_buffer* gfx_create_buffer(const gfx_buffer_desc* buffer_desc)
{
    return NULL;
}

gfx_texture* gfx_create_texture(const gfx_texture_desc* texture_desc)
{
    return NULL;
}

gfx_material* gfx_create_material(const gfx_material_desc* material_desc)
{
    return NULL;
}

void gfx_destroy_buffer(gfx_buffer* buffer)
{

}

void gfx_destroy_texture(gfx_texture* texture)
{

}

void gfx_destroy_material(gfx_material* material)
{

}

void gfx_init_sprite_pipeline(const gfx_sprite_pipeline_desc* sprite_pipeline_desc)
{

}

void gfx_quit_sprite_pipeline(void)
{

}

gfx_sprite* gfx_add_sprite(const gfx_sprite_desc sprite_desc)
{
    return NULL;
}

void gfx_remove_sprite(gfx_sprite* sprite)
{

}
