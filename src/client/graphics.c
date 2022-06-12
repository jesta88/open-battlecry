#include "graphics.h"
#include "../common/log.h"

#define VK_NO_PROTOTYPES

#include "../third_party/volk.h"
#include <assert.h>

static VkInstance s_instance;
static VkPhysicalDevice s_physical_device;

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
}

void wbFreeGraphics(void)
{
}

void wbBeginFrame(void)
{
}

void wbEndFrame(void)
{
}
