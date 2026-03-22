#pragma once

#include <core/types.h>

#define WC_MAX_BINDLESS_RESOURCES 16384
#define WC_MAX_MESHES 4096
#define WC_MAX_MATERIALS 1024

typedef struct VkDevice_T* VkDevice;
typedef struct VmaAllocator_T* VmaAllocator;
typedef struct VkImageView_T* VkImageView;
typedef struct VkSampler_T* VkSampler;

typedef struct WC_Buffer WC_Buffer;
typedef struct WC_Texture WC_Texture;
typedef struct WC_GpuResources WC_GpuResources;

// Mesh data stored in GPU buffers
typedef struct {
	u32 vertexOffset;
	u32 vertexCount;
	u32 indexOffset;
	u32 indexCount;
	u32 materialIndex;
	float boundingSphere[4]; // xyz center, w radius
} WC_GpuMeshData;

// Material data
typedef struct {
	u32 albedoTextureIndex;
	u32 normalTextureIndex;
	u32 metallicRoughnessTextureIndex;
	u32 pad;
} WC_GpuMaterialData;

// Instance data for draw indirect
typedef struct {
	float transform[16]; // 4x4 matrix
	u32 meshIndex;
	u32 instanceID;
	u32 pad[2];
} WC_GpuInstanceData;

int wc_gpu_resource_init(VkDevice device, VmaAllocator allocator);
void wc_gpu_resource_quit();

u32 wc_gpu_resource_add_mesh(
	const float* vertices,
	u32 vertexCount,
	u32 vertexStride,
	const u32* indices,
	u32 indexCount,
	u32 materialIndex,
	const float* boundingSphere
);
u32 wc_gpu_resource_add_texture(VkImageView imageView, VkSampler sampler);

void wc_gpu_resource_update_descriptors(void);