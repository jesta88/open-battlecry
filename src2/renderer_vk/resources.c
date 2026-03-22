#include <renderer/resources.h>
#include <string.h>

#include <volk.h>
#include <vk_mem_alloc.h>

struct WC_Buffer
{
	VkBuffer buffer;
	VkBufferView bufferView;
	VkDescriptorBufferInfo descriptorBuffer;
	VmaAllocation allocation;
};

struct WC_Texture
{
	VkImage image;
	VkImageView imageView;
	VkSampler sampler;
	VkDescriptorImageInfo descriptor;
	VmaAllocation allocation;
};

struct WC_GpuResources
{
	// Descriptor pool and set for bindless resources
	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout bindlessLayout;
	VkDescriptorSet bindlessSet;

	// Global buffers
	VkBuffer meshDataBuffer;
	VmaAllocation meshDataAllocation;
	VkBuffer materialDataBuffer;
	VmaAllocation materialDataAllocation;
	VkBuffer instanceBuffer;
	VmaAllocation instanceAllocation;

	// Vertex and index buffers (single large buffers for all meshes)
	VkBuffer vertexBuffer;
	VmaAllocation vertexAllocation;
	VkBuffer indexBuffer;
	VmaAllocation indexAllocation;

	// Indirect draw buffer
	VkBuffer indirectBuffer;
	VmaAllocation indirectAllocation;

	// Counters
	uint32_t meshCount;
	uint32_t materialCount;
	uint32_t textureCount;
	uint32_t currentVertexOffset;
	uint32_t currentIndexOffset;

	// Vulkan context
	VkDevice device;
	VmaAllocator allocator;
};

static WC_GpuResources s_resources;

static VkResult wc_create_buffer(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage,
								 VkBuffer* buffer, VmaAllocation* allocation)
{
	const VkBufferCreateInfo bufferInfo = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = size, .usage = usage, .sharingMode = VK_SHARING_MODE_EXCLUSIVE};

	const VmaAllocationCreateInfo allocInfo = {.usage = memoryUsage};

	return vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, buffer, allocation, NULL);
}

int wc_gpu_resource_init(VkDevice device, VmaAllocator allocator)
{
	s_resources.device = device;
	s_resources.allocator = allocator;
	s_resources.meshCount = 0;
	s_resources.materialCount = 0;
	s_resources.textureCount = 0;
	s_resources.currentVertexOffset = 0;
	s_resources.currentIndexOffset = 0;

	// Create descriptor pool for bindless resources
	VkDescriptorPoolSize poolSizes[] = {
		{
			.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 10 // Various buffers
		},
		{.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = WC_MAX_BINDLESS_RESOURCES}};

	VkDescriptorPoolCreateInfo poolInfo = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
										   .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
										   .maxSets = 1,
										   .poolSizeCount = 2,
										   .pPoolSizes = poolSizes};

	VkResult result = vkCreateDescriptorPool(device, &poolInfo, NULL, &s_resources.descriptorPool);
	if (result != VK_SUCCESS)
		return result;

	// Create bindless descriptor set layout
	VkDescriptorSetLayoutBinding bindings[] = {// Mesh data buffer
											   {.binding = 0,
												.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
												.descriptorCount = 1,
												.stageFlags = VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_FRAGMENT_BIT},
											   // Material data buffer
											   {.binding = 1,
												.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
												.descriptorCount = 1,
												.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT},
											   // Instance data buffer
											   {.binding = 2,
												.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
												.descriptorCount = 1,
												.stageFlags = VK_SHADER_STAGE_MESH_BIT_EXT},
											   // Vertex buffer
											   {.binding = 3,
												.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
												.descriptorCount = 1,
												.stageFlags = VK_SHADER_STAGE_MESH_BIT_EXT},
											   // Index buffer
											   {.binding = 4,
												.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
												.descriptorCount = 1,
												.stageFlags = VK_SHADER_STAGE_MESH_BIT_EXT},
											   // Bindless textures
											   {.binding = 5,
												.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
												.descriptorCount = WC_MAX_BINDLESS_RESOURCES,
												.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT}};

	VkDescriptorBindingFlags bindingFlags[] = {0,
											   0,
											   0,
											   0,
											   0,
											   VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
												   VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
												   VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT};

	VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
		.bindingCount = 6,
		.pBindingFlags = bindingFlags};

	VkDescriptorSetLayoutCreateInfo layoutInfo = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
												  .pNext = &bindingFlagsInfo,
												  .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
												  .bindingCount = 6,
												  .pBindings = bindings};

	result = vkCreateDescriptorSetLayout(device, &layoutInfo, NULL, &s_resources.bindlessLayout);
	if (result != VK_SUCCESS)
		return result;

	// Allocate descriptor set
	VkDescriptorSetVariableDescriptorCountAllocateInfo variableInfo = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
		.descriptorSetCount = 1,
		.pDescriptorCounts = &(uint32_t){WC_MAX_BINDLESS_RESOURCES}};

	VkDescriptorSetAllocateInfo allocInfo = {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
											 .pNext = &variableInfo,
											 .descriptorPool = s_resources.descriptorPool,
											 .descriptorSetCount = 1,
											 .pSetLayouts = &s_resources.bindlessLayout};

	result = vkAllocateDescriptorSets(device, &allocInfo, &s_resources.bindlessSet);
	if (result != VK_SUCCESS)
		return result;

	// Create buffers
	const VkDeviceSize meshDataSize = sizeof(WC_GpuMeshData) * WC_MAX_MESHES;
	const VkDeviceSize materialDataSize = sizeof(WC_GpuMaterialData) * WC_MAX_MATERIALS;
	const VkDeviceSize instanceSize = sizeof(WC_GpuInstanceData) * 100000; // Support 100k instances
	const VkDeviceSize vertexSize = 1024 * 1024 * 1024;					   // 1GB for vertices
	const VkDeviceSize indexSize = 512 * 1024 * 1024;					   // 512MB for indices
	const VkDeviceSize indirectSize = sizeof(VkDrawIndexedIndirectCommand) * 100000;

	result = wc_create_buffer(allocator, meshDataSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
							  VMA_MEMORY_USAGE_GPU_ONLY, &s_resources.meshDataBuffer, &s_resources.meshDataAllocation);
	if (result != VK_SUCCESS)
		return result;

	result = wc_create_buffer(allocator, materialDataSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
							  VMA_MEMORY_USAGE_GPU_ONLY, &s_resources.materialDataBuffer, &s_resources.materialDataAllocation);
	if (result != VK_SUCCESS)
		return result;

	result = wc_create_buffer(allocator, instanceSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
							  VMA_MEMORY_USAGE_GPU_ONLY, &s_resources.instanceBuffer, &s_resources.instanceAllocation);
	if (result != VK_SUCCESS)
		return result;

	result = wc_create_buffer(allocator, vertexSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
							  VMA_MEMORY_USAGE_GPU_ONLY, &s_resources.vertexBuffer, &s_resources.vertexAllocation);
	if (result != VK_SUCCESS)
		return result;

	result =
		wc_create_buffer(allocator, indexSize,
						 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
						 VMA_MEMORY_USAGE_GPU_ONLY, &s_resources.indexBuffer, &s_resources.indexAllocation);
	if (result != VK_SUCCESS)
		return result;

	result = wc_create_buffer(allocator, indirectSize,
							  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |
								  VK_BUFFER_USAGE_TRANSFER_DST_BIT,
							  VMA_MEMORY_USAGE_GPU_ONLY, &s_resources.indirectBuffer, &s_resources.indirectAllocation);
	if (result != VK_SUCCESS)
		return result;

	// Update initial descriptors for buffers
	wc_gpu_resource_update_descriptors();

	return VK_SUCCESS;
}

void wc_gpu_resource_quit()
{
	vmaDestroyBuffer(s_resources.allocator, s_resources.indirectBuffer, s_resources.indirectAllocation);
	vmaDestroyBuffer(s_resources.allocator, s_resources.indexBuffer, s_resources.indexAllocation);
	vmaDestroyBuffer(s_resources.allocator, s_resources.vertexBuffer, s_resources.vertexAllocation);
	vmaDestroyBuffer(s_resources.allocator, s_resources.instanceBuffer, s_resources.instanceAllocation);
	vmaDestroyBuffer(s_resources.allocator, s_resources.materialDataBuffer, s_resources.materialDataAllocation);
	vmaDestroyBuffer(s_resources.allocator, s_resources.meshDataBuffer, s_resources.meshDataAllocation);
	vkDestroyDescriptorSetLayout(s_resources.device, s_resources.bindlessLayout, NULL);
	vkDestroyDescriptorPool(s_resources.device, s_resources.descriptorPool, NULL);
}

uint32_t wc_gpu_resource_add_mesh(const float* vertices, uint32_t vertexCount, uint32_t vertexStride, const uint32_t* indices,
								  uint32_t indexCount, uint32_t materialIndex, const float* boundingSphere)
{
	if (s_resources.meshCount >= WC_MAX_MESHES)
	{
		return UINT32_MAX;
	}

	uint32_t meshIndex = s_resources.meshCount++;

	// Create staging buffer and copy vertex/index data
	VkDeviceSize vertexSize = vertexCount * vertexStride;
	VkDeviceSize indexSize = indexCount * sizeof(uint32_t);

	VkBuffer stagingBuffer;
	VmaAllocation stagingAllocation;
	wc_create_buffer(s_resources.allocator, vertexSize + indexSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU,
					 &stagingBuffer, &stagingAllocation);

	// Map and copy data
	void* data;
	vmaMapMemory(s_resources.allocator, stagingAllocation, &data);
	memcpy(data, vertices, vertexSize);
	memcpy((uint8_t*)data + vertexSize, indices, indexSize);
	vmaUnmapMemory(s_resources.allocator, stagingAllocation);

	// Copy to GPU buffers (you'll need to record and submit command buffer)
	// This is simplified - in practice you'd batch these transfers

	// Update mesh data
	WC_GpuMeshData meshData = {.vertexOffset = s_resources.currentVertexOffset,
							   .vertexCount = vertexCount,
							   .indexOffset = s_resources.currentIndexOffset,
							   .indexCount = indexCount,
							   .materialIndex = materialIndex};
	memcpy(meshData.boundingSphere, boundingSphere, sizeof(float) * 4);

	// Upload mesh data (simplified - needs staging)
	void* meshDataPtr;
	vmaMapMemory(s_resources.allocator, s_resources.meshDataAllocation, &meshDataPtr);
	memcpy((WC_GpuMeshData*)meshDataPtr + meshIndex, &meshData, sizeof(WC_GpuMeshData));
	vmaUnmapMemory(s_resources.allocator, s_resources.meshDataAllocation);

	s_resources.currentVertexOffset += vertexCount;
	s_resources.currentIndexOffset += indexCount;

	vmaDestroyBuffer(s_resources.allocator, stagingBuffer, stagingAllocation);

	return meshIndex;
}

uint32_t wc_gpu_resource_add_texture(VkImageView imageView, VkSampler sampler)
{
	if (s_resources.textureCount >= WC_MAX_BINDLESS_RESOURCES)
	{
		return UINT32_MAX;
	}

	uint32_t textureIndex = s_resources.textureCount++;

	VkDescriptorImageInfo imageInfo = {
		.sampler = sampler, .imageView = imageView, .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

	VkWriteDescriptorSet write = {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
								  .dstSet = s_resources.bindlessSet,
								  .dstBinding = 5,
								  .dstArrayElement = textureIndex,
								  .descriptorCount = 1,
								  .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
								  .pImageInfo = &imageInfo};

	vkUpdateDescriptorSets(s_resources.device, 1, &write, 0, NULL);

	return textureIndex;
}

void wc_gpu_resource_update_descriptors()
{
	VkDescriptorBufferInfo bufferInfos[5] = {// Mesh data
											 {.buffer = s_resources.meshDataBuffer, .offset = 0, .range = VK_WHOLE_SIZE},
											 // Material data
											 {.buffer = s_resources.materialDataBuffer, .offset = 0, .range = VK_WHOLE_SIZE},
											 // Instance data
											 {.buffer = s_resources.instanceBuffer, .offset = 0, .range = VK_WHOLE_SIZE},
											 // Vertex buffer
											 {.buffer = s_resources.vertexBuffer, .offset = 0, .range = VK_WHOLE_SIZE},
											 // Index buffer
											 {.buffer = s_resources.indexBuffer, .offset = 0, .range = VK_WHOLE_SIZE}};

	VkWriteDescriptorSet writes[5];
	for (uint32_t i = 0; i < 5; i++)
	{
		writes[i] = (VkWriteDescriptorSet){.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
										   .dstSet = s_resources.bindlessSet,
										   .dstBinding = i,
										   .dstArrayElement = 0,
										   .descriptorCount = 1,
										   .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
										   .pBufferInfo = &bufferInfos[i]};
	}

	vkUpdateDescriptorSets(s_resources.device, 5, writes, 0, NULL);
}
