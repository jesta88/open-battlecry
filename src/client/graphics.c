#include "graphics.h"
#include "client.h"
#include "../common/log.h"

#define VK_NO_PROTOTYPES

#include "../third_party/volk.h"
#include <assert.h>
#include <minmax.h>

enum
{
	MAX_BUFFERED_FRAMES = 2,
	MAX_SWAPCHAIN_IMAGES = 3,
	MAX_THREADS = 64
};

static VkInstance s_instance;
static VkSurfaceKHR s_surface;
static VkPhysicalDevice s_physical_device;
static u32 s_graphics_family_index;
static u32 s_transfer_family_index;
static VkDevice s_device;
static VkQueue s_graphics_queue;
static VkQueue s_transfer_queue;
static VkSwapchainKHR s_swapchain;
static u32 s_swapchain_image_count;
static VkImage s_swapchain_images[MAX_SWAPCHAIN_IMAGES];
static VkImageView s_swapchain_image_views[MAX_SWAPCHAIN_IMAGES];
static VkSemaphore s_image_available_semaphores[MAX_BUFFERED_FRAMES];
static VkSemaphore s_submit_complete_semaphores[MAX_BUFFERED_FRAMES];
static VkFence s_submit_complete_fences[MAX_BUFFERED_FRAMES];
static VkCommandPool s_command_pools[MAX_BUFFERED_FRAMES];
static VkCommandBuffer s_primary_command_buffers[MAX_BUFFERED_FRAMES];
static VkCommandBuffer s_secondary_command_buffers[MAX_BUFFERED_FRAMES];

static VkPipeline s_terrain_pipeline;
static VkPipeline s_sprite_pipeline;
static VkPipeline s_ui_pipeline;

static VkDescriptorPool s_descriptor_pool;
static VkDescriptorSetLayout s_terrain_set_layout;

static VkSampler s_point_sampler;



static bool s_vsync;
static u32 s_width;
static u32 s_height;
static u32 s_buffered_frame_index;
static u32 s_swapchain_image_index;

static const u32 k_swapchain_format = VK_FORMAT_B8G8R8A8_UNORM;

static u32 getQueueFamilyIndex(VkQueueFlags queue_flags, u32 queue_family_count,
		const VkQueueFamilyProperties* queue_family_properties);

static void createSwapchain();

static void recreateSwapchain();

void wbInitGraphics(const WbGraphicsDesc* graphics_desc)
{
	assert(graphics_desc);

	VkResult result = volkInitialize();
	assert(result == VK_SUCCESS);

	{
		u32 vk_instance_version = volkGetInstanceVersion();
		wbLogInfo("Vulkan instance version: %d.%d.%d", VK_API_VERSION_MAJOR(vk_instance_version),
				VK_API_VERSION_MINOR(vk_instance_version), VK_API_VERSION_PATCH(vk_instance_version));
	}

	const char* instance_layers[] = { "VK_LAYER_KHRONOS_validation" };
	const char* instance_extensions[] = { "VK_KHR_surface", "VK_KHR_win32_surface" };

	result = vkCreateInstance(&(const VkInstanceCreateInfo){
					.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
					.pApplicationInfo = &(const VkApplicationInfo){
							.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
							.pApplicationName = "Open WBC",
							.applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 0),
							.pEngineName = "Open WBC",
							.engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0),
							.apiVersion = VK_API_VERSION_1_3,
					},
					.enabledLayerCount = graphics_desc->enable_validation ? 1 : 0,
					.ppEnabledLayerNames = instance_layers,
					.enabledExtensionCount = 2,
					.ppEnabledExtensionNames = instance_extensions,
			},
			NULL, &s_instance);
	assert(result == VK_SUCCESS);

	volkLoadInstanceOnly(s_instance);

	{
		u32 physical_device_count;
		result = vkEnumeratePhysicalDevices(s_instance, &physical_device_count, NULL);
		assert(result == VK_SUCCESS);

		VkPhysicalDevice physical_devices[4];
		result = vkEnumeratePhysicalDevices(s_instance, &physical_device_count, physical_devices);
		assert(result == VK_SUCCESS);

		s_physical_device = physical_devices[0];

		VkPhysicalDeviceProperties physical_device_properties;
		vkGetPhysicalDeviceProperties(s_physical_device, &physical_device_properties);
		wbLogInfo("Physical device: %s", physical_device_properties.deviceName);
	}

	{
		result = vkCreateWin32SurfaceKHR(s_instance, &(const VkWin32SurfaceCreateInfoKHR){
				.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
				.hwnd = graphics_desc->hwnd,
				.hinstance = graphics_desc->hinstance
		}, NULL, &s_surface);
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

	{
		u32 queue_family_count;
		vkGetPhysicalDeviceQueueFamilyProperties(s_physical_device, &queue_family_count, NULL);
		VkQueueFamilyProperties queue_family_properties[8];
		vkGetPhysicalDeviceQueueFamilyProperties(s_physical_device, &queue_family_count, queue_family_properties);

		s_graphics_family_index = getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT, queue_family_count,
				queue_family_properties);
		s_transfer_family_index = getQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT, queue_family_count,
				queue_family_properties);
	}

	{
		const char* device_extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		VkPhysicalDeviceFeatures2 supported_features = {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
		vkGetPhysicalDeviceFeatures2(s_physical_device, &supported_features);

		VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_features = {
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
				.dynamicRendering = VK_TRUE
		};
		VkPhysicalDeviceFeatures2 requested_features = {
				.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
				.pNext = &dynamic_rendering_features,
		};

		result = vkCreateDevice(s_physical_device, &(const VkDeviceCreateInfo){
				.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
				.pNext = &requested_features,
				.queueCreateInfoCount = 2,
				.pQueueCreateInfos = (const VkDeviceQueueCreateInfo[]){
						{
								.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
								.pQueuePriorities = (const float[]){ 1.0f },
								.queueCount = 1,
								.queueFamilyIndex = s_graphics_family_index,
						},
						{
								.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
								.pQueuePriorities = (const float[]){ 1.0f },
								.queueCount = 1,
								.queueFamilyIndex = s_transfer_family_index,
						}
				},
				.enabledExtensionCount = 1,
				.ppEnabledExtensionNames = device_extensions,
		}, NULL, &s_device);
		assert(result == VK_SUCCESS);
	}

	volkLoadDevice(s_device);

	vkGetDeviceQueue(s_device, s_graphics_family_index, 0, &s_graphics_queue);
	vkGetDeviceQueue(s_device, s_transfer_family_index, 0, &s_transfer_queue);

	s_vsync = graphics_desc->vsync;
	s_width = graphics_desc->window_width;
	s_height = graphics_desc->window_height;
	createSwapchain();

	{
		VkFenceCreateInfo fence_info = {
				.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
				.flags = VK_FENCE_CREATE_SIGNALED_BIT
		};
		VkSemaphoreCreateInfo semaphore_info = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		for (uint32_t i = 0; i < MAX_BUFFERED_FRAMES; ++i)
		{
			result = vkCreateFence(s_device, &fence_info, NULL, &s_submit_complete_fences[i]);
			assert(result == VK_SUCCESS);
			result = vkCreateSemaphore(s_device, &semaphore_info, NULL, &s_submit_complete_semaphores[i]);
			assert(result == VK_SUCCESS);
			result = vkCreateSemaphore(s_device, &semaphore_info, NULL, &s_image_available_semaphores[i]);
			assert(result == VK_SUCCESS);
		}
	}

	for (u32 i = 0; i < MAX_BUFFERED_FRAMES; i++)
	{
		result = vkCreateCommandPool(s_device, &(const VkCommandPoolCreateInfo){
				.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
				.queueFamilyIndex = s_graphics_family_index,
				.flags = 0,
		}, NULL, &s_command_pools[i]);
		assert(result == VK_SUCCESS);

		result = vkAllocateCommandBuffers(s_device, &(const VkCommandBufferAllocateInfo){
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.commandPool = s_command_pools[i],
				.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = MAX_BUFFERED_FRAMES
		}, &s_primary_command_buffers[i]);
		assert(result == VK_SUCCESS);
	}
}

void wbFreeGraphics(void)
{
}

void wbBeginFrame(void)
{
	VkResult result;

	result = vkWaitForFences(s_device, 1, &s_submit_complete_fences[s_buffered_frame_index], VK_TRUE, UINT64_MAX);
	assert(result == VK_SUCCESS);
	result = vkResetFences(s_device, 1, &s_submit_complete_fences[s_buffered_frame_index]);
	assert(result == VK_SUCCESS);

	result = vkAcquireNextImageKHR(s_device, s_swapchain, UINT64_MAX, s_image_available_semaphores[s_buffered_frame_index], VK_NULL_HANDLE, &s_swapchain_image_index);
	assert(result == VK_SUCCESS);

	vkResetCommandPool(s_device, s_command_pools[s_buffered_frame_index], 0);

	result = vkBeginCommandBuffer(s_primary_command_buffers[s_buffered_frame_index], &(const VkCommandBufferBeginInfo){
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	});
	assert(result == VK_SUCCESS);

	vkCmdPipelineBarrier(s_primary_command_buffers[s_buffered_frame_index],
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

	vkCmdBeginRendering(s_primary_command_buffers[s_buffered_frame_index], &(const VkRenderingInfo){
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
			.renderArea = { .extent = { s_width, s_height }},
			.layerCount = 1,
			.colorAttachmentCount = 1,
			.pColorAttachments = &(const VkRenderingAttachmentInfo){
					.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
					.imageView = s_swapchain_image_views[s_swapchain_image_index],
					.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
					.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
					.clearValue = { .color = { .float32 = { 0.4f, 0.6f, 0.9f, 1.0f }}},
			},
	});

	vkCmdSetViewport(s_primary_command_buffers[s_buffered_frame_index], 0, 1, &(const VkViewport) {
		.width = (float) s_width,
		.height = (float) s_height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	});

	vkCmdSetScissor(s_primary_command_buffers[s_buffered_frame_index], 0, 1, &(const VkRect2D) {
		.extent = {s_width, s_height}
	});
}

void wbEndFrame(void)
{
	VkResult result;

	vkCmdEndRendering(s_primary_command_buffers[s_buffered_frame_index]);

	vkCmdPipelineBarrier(s_primary_command_buffers[s_buffered_frame_index],
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

	result = vkEndCommandBuffer(s_primary_command_buffers[s_buffered_frame_index]);
	assert(result == VK_SUCCESS);

	result = vkQueueSubmit(s_graphics_queue, 1, &(const VkSubmitInfo) {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &s_primary_command_buffers[s_buffered_frame_index],
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &s_image_available_semaphores[s_buffered_frame_index],
		.pWaitDstStageMask = &(const VkPipelineStageFlags) {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &s_submit_complete_semaphores[s_buffered_frame_index],
	}, s_submit_complete_fences[s_buffered_frame_index]);
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
		recreateSwapchain();
	}

	s_buffered_frame_index = (s_buffered_frame_index + 1) % MAX_BUFFERED_FRAMES;
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

static void createSwapchain()
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

		VkPresentModeKHR preferred_present_mode = s_vsync
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

	VkExtent2D extent;
	if (surface_capabilities.currentExtent.width == UINT32_MAX ||
		surface_capabilities.currentExtent.height == UINT32_MAX)
	{
		wbGetWindowSize(&s_width, &s_height);
		extent.width = s_width;
		extent.height = s_height;
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
			.pNext = NULL, // TODO: Handle exclusive fullscreen
			.surface = s_surface,
			.minImageCount = s_vsync ? 3 : 2,
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
	}, NULL, &s_swapchain);
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

	wbLogInfo("Vulkan swapchain created with size %dx%d and %d images.", extent.width, extent.height,
			s_swapchain_image_count);

	for (u32 i = 0; i < s_swapchain_image_count; i++)
	{
		result = vkCreateImageView(s_device, &(const VkImageViewCreateInfo){
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.format = k_swapchain_format,
				.image = s_swapchain_images[i],
				.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.subresourceRange.levelCount = 1,
				.subresourceRange.layerCount = 1,
				.viewType = VK_IMAGE_VIEW_TYPE_2D
		}, NULL, &s_swapchain_image_views[i]);
		assert(result == VK_SUCCESS);
	}
}

static void recreateSwapchain()
{
	vkDeviceWaitIdle(s_device);
	createSwapchain();
}