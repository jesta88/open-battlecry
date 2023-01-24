#pragma once

#include "../../engine/std.h"

// Vulkan handles
#define VK_DEFINE_HANDLE(object) typedef struct object##_T* object;
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef struct object##_T *object;

VK_DEFINE_HANDLE(VkInstance)
VK_DEFINE_HANDLE(VkPhysicalDevice)
VK_DEFINE_HANDLE(VkDevice)
VK_DEFINE_HANDLE(VkQueue)
VK_DEFINE_HANDLE(VkCommandBuffer)
VK_DEFINE_HANDLE(VmaAllocator)
VK_DEFINE_HANDLE(VmaAllocation)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkBuffer)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkImage)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSemaphore)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkFence)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkDeviceMemory)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkEvent)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkQueryPool)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkBufferView)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkImageView)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkShaderModule)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkPipelineCache)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkPipelineLayout)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkPipeline)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkRenderPass)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkDescriptorSetLayout)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSampler)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkDescriptorSet)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkDescriptorPool)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkFramebuffer)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkCommandPool)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSurfaceKHR)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSwapchainKHR)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkDebugUtilsMessengerEXT)

#undef VK_DEFINE_HANDLE
#undef VK_DEFINE_NON_DISPATCHABLE_HANDLE

// Constants
enum
{
    GFX_DEFAULT_MAX_SAMPLED_IMAGES = 2048,
    GFX_DEFAULT_MAX_STORAGE_BUFFERS = 2,
    GFX_DEFAULT_MAX_SPRITES = 2048,
};

// Types
// TODO: Use handles instead of pointers
typedef struct gfx_buffer gfx_buffer;
typedef struct gfx_texture gfx_texture;
typedef struct gfx_material gfx_material;
typedef struct gfx_sprite gfx_sprite;

typedef struct gfx_desc
{
    u32 max_sampled_images;
    u32 max_storage_buffers;

    bool vsync;
    float render_scale;
} gfx_desc;

#define GFX_DEFAULT_DESC &(const gfx_desc) {                \
    .max_sampled_images = GFX_DEFAULT_MAX_SAMPLED_IMAGES,   \
    .max_storage_buffers = GFX_DEFAULT_MAX_STORAGE_BUFFERS, \
    .vsync = true,                                          \
    .render_scale = 1.0f }

typedef struct gfx_sprite_pipeline_desc
{
    u32 max_sprites;
} gfx_sprite_pipeline_desc;

#define GFX_DEFAULT_SPRITE_PIPELINE_DESC &(const gfx_sprite_pipeline_desc) {    \
    .max_sprites = GFX_DEFAULT_MAX_SPRITES }

typedef struct gfx_buffer_desc
{
	const u32 size;
	const void* data;
} gfx_buffer_desc;

typedef struct gfx_texture_desc
{
	const u32 width;
	const u32 height;
} gfx_texture_desc;

typedef struct gfx_material_desc
{
	const u32* shader_bytes;
	const u32 shader_byte_count;
} gfx_material_desc;

typedef struct gfx_sprite_desc
{
    gfx_texture* texture;
    s32 x;
    s32 y;
} gfx_sprite_desc;

// General
void gfx_init(const gfx_desc* desc);
void gfx_quit(void);
void gfx_draw(void);
void gfx_resize(u32 width, u32 height, bool vsync);

// Resources
gfx_buffer*   gfx_create_buffer(const gfx_buffer_desc* buffer_desc);
gfx_texture*  gfx_create_texture(const gfx_texture_desc* texture_desc);
gfx_material* gfx_create_material(const gfx_material_desc* material_desc);

void gfx_destroy_buffer(gfx_buffer* buffer);
void gfx_destroy_texture(gfx_texture* texture);
void gfx_destroy_material(gfx_material* material);

// Sprites
void gfx_init_sprite_pipeline(const gfx_sprite_pipeline_desc* sprite_pipeline_desc);
void gfx_quit_sprite_pipeline(void);

gfx_sprite* gfx_add_sprite(const gfx_sprite_desc sprite_desc);
void 		gfx_remove_sprite(gfx_sprite* sprite);

// Vulkan handles
VkInstance gfx_get_vk_instance(void);