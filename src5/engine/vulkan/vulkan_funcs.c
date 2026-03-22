#include "vulkan_funcs.h"

#ifdef WK_PLATFORM_WINDOWS
typedef const char* LPCSTR;
typedef struct HINSTANCE__* HINSTANCE;
typedef HINSTANCE HMODULE;
typedef __int64(__stdcall* FARPROC)(void);

__declspec(dllimport) HMODULE __stdcall LoadLibraryA(LPCSTR);
__declspec(dllimport) FARPROC __stdcall GetProcAddress(HMODULE, LPCSTR);
__declspec(dllimport) int __stdcall FreeLibrary(HMODULE);
#else
#include <dlfcn.h>
#endif

static void* vulkan_module = NULL;

bool wk_load_vulkan(void)
{
#ifdef WK_PLATFORM_WINDOWS
    vulkan_module = LoadLibraryA("vulkan-1.dll");
    if (!vulkan_module)
        return false;

    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)GetProcAddress(vulkan_module, "vkGetInstanceProcAddr");
#elif defined(WK_PLATFORM_MACOS)
    vulkan_module = dlopen("libvulkan.dylib", RTLD_NOW | RTLD_LOCAL);
    if (!vulkan_module)
        vulkan_module = dlopen("libvulkan.1.dylib", RTLD_NOW | RTLD_LOCAL);
    if (!vulkan_module)
        vulkan_module = dlopen("libMoltenVK.dylib", RTLD_NOW | RTLD_LOCAL);
    if (!vulkan_module)
        return VK_ERROR_INITIALIZATION_FAILED;

    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dlsym(vulkan_module, "vkGetInstanceProcAddr");
#else
    vulkan_module = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
    if (!vulkan_module)
        vulkan_module = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
    if (!vulkan_module)
        return VK_ERROR_INITIALIZATION_FAILED;

    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dlsym(vulkan_module, "vkGetInstanceProcAddr");
#endif

    vkCreateInstance = (PFN_vkCreateInstance)vkGetInstanceProcAddr(NULL, "vkCreateInstance");
    vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties)vkGetInstanceProcAddr(NULL, "vkEnumerateInstanceExtensionProperties");
    vkEnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties)vkGetInstanceProcAddr(NULL, "vkEnumerateInstanceLayerProperties");
    vkEnumerateInstanceVersion = (PFN_vkEnumerateInstanceVersion)vkGetInstanceProcAddr(NULL, "vkEnumerateInstanceVersion");

    return true;
}

void wk_unload_vulkan(void)
{
#if defined(WK_PLATFORM_WINDOWS)
    FreeLibrary(vulkan_module);
#else
    dlclose(vulkan_module);
#endif
}

void wk_load_vulkan_instance_funcs(VkInstance instance)
{
#if defined(VK_VERSION_1_0)
    vkCreateDevice = (PFN_vkCreateDevice)vkGetInstanceProcAddr(instance, "vkCreateDevice");
    vkDestroyInstance = (PFN_vkDestroyInstance)vkGetInstanceProcAddr(instance, "vkDestroyInstance");
    vkEnumerateDeviceExtensionProperties = (PFN_vkEnumerateDeviceExtensionProperties)vkGetInstanceProcAddr(instance, "vkEnumerateDeviceExtensionProperties");
    vkEnumerateDeviceLayerProperties = (PFN_vkEnumerateDeviceLayerProperties)vkGetInstanceProcAddr(instance, "vkEnumerateDeviceLayerProperties");
    vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)vkGetInstanceProcAddr(instance, "vkEnumeratePhysicalDevices");
    vkGetDeviceProcAddr = (PFN_vkGetDeviceProcAddr)vkGetInstanceProcAddr(instance, "vkGetDeviceProcAddr");
    vkGetPhysicalDeviceFeatures = (PFN_vkGetPhysicalDeviceFeatures)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceFeatures");
    vkGetPhysicalDeviceFormatProperties = (PFN_vkGetPhysicalDeviceFormatProperties)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceFormatProperties");
    vkGetPhysicalDeviceImageFormatProperties = (PFN_vkGetPhysicalDeviceImageFormatProperties)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceImageFormatProperties");
    vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceMemoryProperties");
    vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties");
    vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceQueueFamilyProperties");
    vkGetPhysicalDeviceSparseImageFormatProperties = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSparseImageFormatProperties");
#endif /* defined(VK_VERSION_1_0) */
#if defined(VK_VERSION_1_1)
    vkEnumeratePhysicalDeviceGroups = (PFN_vkEnumeratePhysicalDeviceGroups)vkGetInstanceProcAddr(instance, "vkEnumeratePhysicalDeviceGroups");
    vkGetPhysicalDeviceExternalBufferProperties = (PFN_vkGetPhysicalDeviceExternalBufferProperties)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceExternalBufferProperties");
    vkGetPhysicalDeviceExternalFenceProperties = (PFN_vkGetPhysicalDeviceExternalFenceProperties)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceExternalFenceProperties");
    vkGetPhysicalDeviceExternalSemaphoreProperties = (PFN_vkGetPhysicalDeviceExternalSemaphoreProperties)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceExternalSemaphoreProperties");
    vkGetPhysicalDeviceFeatures2 = (PFN_vkGetPhysicalDeviceFeatures2)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceFeatures2");
    vkGetPhysicalDeviceFormatProperties2 = (PFN_vkGetPhysicalDeviceFormatProperties2)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceFormatProperties2");
    vkGetPhysicalDeviceImageFormatProperties2 = (PFN_vkGetPhysicalDeviceImageFormatProperties2)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceImageFormatProperties2");
    vkGetPhysicalDeviceMemoryProperties2 = (PFN_vkGetPhysicalDeviceMemoryProperties2)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceMemoryProperties2");
    vkGetPhysicalDeviceProperties2 = (PFN_vkGetPhysicalDeviceProperties2)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties2");
    vkGetPhysicalDeviceQueueFamilyProperties2 = (PFN_vkGetPhysicalDeviceQueueFamilyProperties2)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceQueueFamilyProperties2");
    vkGetPhysicalDeviceSparseImageFormatProperties2 = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSparseImageFormatProperties2");
#endif /* defined(VK_VERSION_1_1) */
#if defined(VK_VERSION_1_3)
    vkGetPhysicalDeviceToolProperties = (PFN_vkGetPhysicalDeviceToolProperties)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceToolProperties");
#endif /* defined(VK_VERSION_1_3) */
#if defined(VK_EXT_acquire_drm_display)
    vkAcquireDrmDisplayEXT = (PFN_vkAcquireDrmDisplayEXT)vkGetInstanceProcAddr(instance, "vkAcquireDrmDisplayEXT");
    vkGetDrmDisplayEXT = (PFN_vkGetDrmDisplayEXT)vkGetInstanceProcAddr(instance, "vkGetDrmDisplayEXT");
#endif /* defined(VK_EXT_acquire_drm_display) */
#if defined(VK_EXT_acquire_xlib_display)
    vkAcquireXlibDisplayEXT = (PFN_vkAcquireXlibDisplayEXT)vkGetInstanceProcAddr(instance, "vkAcquireXlibDisplayEXT");
    vkGetRandROutputDisplayEXT = (PFN_vkGetRandROutputDisplayEXT)vkGetInstanceProcAddr(instance, "vkGetRandROutputDisplayEXT");
#endif /* defined(VK_EXT_acquire_xlib_display) */
#if defined(VK_EXT_calibrated_timestamps)
    vkGetPhysicalDeviceCalibrateableTimeDomainsEXT = (PFN_vkGetPhysicalDeviceCalibrateableTimeDomainsEXT)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceCalibrateableTimeDomainsEXT");
#endif /* defined(VK_EXT_calibrated_timestamps) */
#if defined(VK_EXT_debug_report)
    vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
    vkDebugReportMessageEXT = (PFN_vkDebugReportMessageEXT)vkGetInstanceProcAddr(instance, "vkDebugReportMessageEXT");
    vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
#endif /* defined(VK_EXT_debug_report) */
#if defined(VK_EXT_debug_utils)
    vkCmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT");
    vkCmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT");
    vkCmdInsertDebugUtilsLabelEXT = (PFN_vkCmdInsertDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkCmdInsertDebugUtilsLabelEXT");
    vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    vkQueueBeginDebugUtilsLabelEXT = (PFN_vkQueueBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkQueueBeginDebugUtilsLabelEXT");
    vkQueueEndDebugUtilsLabelEXT = (PFN_vkQueueEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkQueueEndDebugUtilsLabelEXT");
    vkQueueInsertDebugUtilsLabelEXT = (PFN_vkQueueInsertDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkQueueInsertDebugUtilsLabelEXT");
    vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");
    vkSetDebugUtilsObjectTagEXT = (PFN_vkSetDebugUtilsObjectTagEXT)vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectTagEXT");
    vkSubmitDebugUtilsMessageEXT = (PFN_vkSubmitDebugUtilsMessageEXT)vkGetInstanceProcAddr(instance, "vkSubmitDebugUtilsMessageEXT");
#endif /* defined(VK_EXT_debug_utils) */
#if defined(VK_EXT_direct_mode_display)
    vkReleaseDisplayEXT = (PFN_vkReleaseDisplayEXT)vkGetInstanceProcAddr(instance, "vkReleaseDisplayEXT");
#endif /* defined(VK_EXT_direct_mode_display) */
#if defined(VK_EXT_directfb_surface)
    vkCreateDirectFBSurfaceEXT = (PFN_vkCreateDirectFBSurfaceEXT)vkGetInstanceProcAddr(instance, "vkCreateDirectFBSurfaceEXT");
    vkGetPhysicalDeviceDirectFBPresentationSupportEXT = (PFN_vkGetPhysicalDeviceDirectFBPresentationSupportEXT)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceDirectFBPresentationSupportEXT");
#endif /* defined(VK_EXT_directfb_surface) */
#if defined(VK_EXT_display_surface_counter)
    vkGetPhysicalDeviceSurfaceCapabilities2EXT = (PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceCapabilities2EXT");
#endif /* defined(VK_EXT_display_surface_counter) */
#if defined(VK_EXT_full_screen_exclusive)
    vkGetPhysicalDeviceSurfacePresentModes2EXT = (PFN_vkGetPhysicalDeviceSurfacePresentModes2EXT)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfacePresentModes2EXT");
#endif /* defined(VK_EXT_full_screen_exclusive) */
#if defined(VK_EXT_headless_surface)
    vkCreateHeadlessSurfaceEXT = (PFN_vkCreateHeadlessSurfaceEXT)vkGetInstanceProcAddr(instance, "vkCreateHeadlessSurfaceEXT");
#endif /* defined(VK_EXT_headless_surface) */
#if defined(VK_EXT_metal_surface)
    vkCreateMetalSurfaceEXT = (PFN_vkCreateMetalSurfaceEXT)vkGetInstanceProcAddr(instance, "vkCreateMetalSurfaceEXT");
#endif /* defined(VK_EXT_metal_surface) */
#if defined(VK_EXT_sample_locations)
    vkGetPhysicalDeviceMultisamplePropertiesEXT = (PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceMultisamplePropertiesEXT");
#endif /* defined(VK_EXT_sample_locations) */
#if defined(VK_EXT_tooling_info)
    vkGetPhysicalDeviceToolPropertiesEXT = (PFN_vkGetPhysicalDeviceToolPropertiesEXT)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceToolPropertiesEXT");
#endif /* defined(VK_EXT_tooling_info) */
#if defined(VK_FUCHSIA_imagepipe_surface)
    vkCreateImagePipeSurfaceFUCHSIA = (PFN_vkCreateImagePipeSurfaceFUCHSIA)vkGetInstanceProcAddr(instance, "vkCreateImagePipeSurfaceFUCHSIA");
#endif /* defined(VK_FUCHSIA_imagepipe_surface) */
#if defined(VK_GGP_stream_descriptor_surface)
    vkCreateStreamDescriptorSurfaceGGP = (PFN_vkCreateStreamDescriptorSurfaceGGP)vkGetInstanceProcAddr(instance, "vkCreateStreamDescriptorSurfaceGGP");
#endif /* defined(VK_GGP_stream_descriptor_surface) */
#if defined(VK_KHR_android_surface)
    vkCreateAndroidSurfaceKHR = (PFN_vkCreateAndroidSurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateAndroidSurfaceKHR");
#endif /* defined(VK_KHR_android_surface) */
#if defined(VK_KHR_cooperative_matrix)
    vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR = (PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR");
#endif /* defined(VK_KHR_cooperative_matrix) */
#if defined(VK_KHR_device_group_creation)
    vkEnumeratePhysicalDeviceGroupsKHR = (PFN_vkEnumeratePhysicalDeviceGroupsKHR)vkGetInstanceProcAddr(instance, "vkEnumeratePhysicalDeviceGroupsKHR");
#endif /* defined(VK_KHR_device_group_creation) */
#if defined(VK_KHR_display)
    vkCreateDisplayModeKHR = (PFN_vkCreateDisplayModeKHR)vkGetInstanceProcAddr(instance, "vkCreateDisplayModeKHR");
    vkCreateDisplayPlaneSurfaceKHR = (PFN_vkCreateDisplayPlaneSurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateDisplayPlaneSurfaceKHR");
    vkGetDisplayModePropertiesKHR = (PFN_vkGetDisplayModePropertiesKHR)vkGetInstanceProcAddr(instance, "vkGetDisplayModePropertiesKHR");
    vkGetDisplayPlaneCapabilitiesKHR = (PFN_vkGetDisplayPlaneCapabilitiesKHR)vkGetInstanceProcAddr(instance, "vkGetDisplayPlaneCapabilitiesKHR");
    vkGetDisplayPlaneSupportedDisplaysKHR = (PFN_vkGetDisplayPlaneSupportedDisplaysKHR)vkGetInstanceProcAddr(instance, "vkGetDisplayPlaneSupportedDisplaysKHR");
    vkGetPhysicalDeviceDisplayPlanePropertiesKHR = (PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
    vkGetPhysicalDeviceDisplayPropertiesKHR = (PFN_vkGetPhysicalDeviceDisplayPropertiesKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceDisplayPropertiesKHR");
#endif /* defined(VK_KHR_display) */
#if defined(VK_KHR_external_fence_capabilities)
    vkGetPhysicalDeviceExternalFencePropertiesKHR = (PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceExternalFencePropertiesKHR");
#endif /* defined(VK_KHR_external_fence_capabilities) */
#if defined(VK_KHR_external_memory_capabilities)
    vkGetPhysicalDeviceExternalBufferPropertiesKHR = (PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceExternalBufferPropertiesKHR");
#endif /* defined(VK_KHR_external_memory_capabilities) */
#if defined(VK_KHR_external_semaphore_capabilities)
    vkGetPhysicalDeviceExternalSemaphorePropertiesKHR = (PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceExternalSemaphorePropertiesKHR");
#endif /* defined(VK_KHR_external_semaphore_capabilities) */
#if defined(VK_KHR_fragment_shading_rate)
    vkGetPhysicalDeviceFragmentShadingRatesKHR = (PFN_vkGetPhysicalDeviceFragmentShadingRatesKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceFragmentShadingRatesKHR");
#endif /* defined(VK_KHR_fragment_shading_rate) */
#if defined(VK_KHR_get_display_properties2)
    vkGetDisplayModeProperties2KHR = (PFN_vkGetDisplayModeProperties2KHR)vkGetInstanceProcAddr(instance, "vkGetDisplayModeProperties2KHR");
    vkGetDisplayPlaneCapabilities2KHR = (PFN_vkGetDisplayPlaneCapabilities2KHR)vkGetInstanceProcAddr(instance, "vkGetDisplayPlaneCapabilities2KHR");
    vkGetPhysicalDeviceDisplayPlaneProperties2KHR = (PFN_vkGetPhysicalDeviceDisplayPlaneProperties2KHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceDisplayPlaneProperties2KHR");
    vkGetPhysicalDeviceDisplayProperties2KHR = (PFN_vkGetPhysicalDeviceDisplayProperties2KHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceDisplayProperties2KHR");
#endif /* defined(VK_KHR_get_display_properties2) */
#if defined(VK_KHR_get_physical_device_properties2)
    vkGetPhysicalDeviceFeatures2KHR = (PFN_vkGetPhysicalDeviceFeatures2KHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceFeatures2KHR");
    vkGetPhysicalDeviceFormatProperties2KHR = (PFN_vkGetPhysicalDeviceFormatProperties2KHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceFormatProperties2KHR");
    vkGetPhysicalDeviceImageFormatProperties2KHR = (PFN_vkGetPhysicalDeviceImageFormatProperties2KHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceImageFormatProperties2KHR");
    vkGetPhysicalDeviceMemoryProperties2KHR = (PFN_vkGetPhysicalDeviceMemoryProperties2KHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceMemoryProperties2KHR");
    vkGetPhysicalDeviceProperties2KHR = (PFN_vkGetPhysicalDeviceProperties2KHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties2KHR");
    vkGetPhysicalDeviceQueueFamilyProperties2KHR = (PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceQueueFamilyProperties2KHR");
    vkGetPhysicalDeviceSparseImageFormatProperties2KHR = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSparseImageFormatProperties2KHR");
#endif /* defined(VK_KHR_get_physical_device_properties2) */
#if defined(VK_KHR_get_surface_capabilities2)
    vkGetPhysicalDeviceSurfaceCapabilities2KHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceCapabilities2KHR");
    vkGetPhysicalDeviceSurfaceFormats2KHR = (PFN_vkGetPhysicalDeviceSurfaceFormats2KHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceFormats2KHR");
#endif /* defined(VK_KHR_get_surface_capabilities2) */
#if defined(VK_KHR_performance_query)
    vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR = (PFN_vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR)vkGetInstanceProcAddr(instance, "vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR");
    vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR = (PFN_vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR");
#endif /* defined(VK_KHR_performance_query) */
#if defined(VK_KHR_surface)
    vkDestroySurfaceKHR = (PFN_vkDestroySurfaceKHR)vkGetInstanceProcAddr(instance, "vkDestroySurfaceKHR");
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
    vkGetPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceFormatsKHR");
    vkGetPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfacePresentModesKHR");
    vkGetPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceSupportKHR");
#endif /* defined(VK_KHR_surface) */
#if defined(VK_KHR_video_encode_queue)
    vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR = (PFN_vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR");
#endif /* defined(VK_KHR_video_encode_queue) */
#if defined(VK_KHR_video_queue)
    vkGetPhysicalDeviceVideoCapabilitiesKHR = (PFN_vkGetPhysicalDeviceVideoCapabilitiesKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceVideoCapabilitiesKHR");
    vkGetPhysicalDeviceVideoFormatPropertiesKHR = (PFN_vkGetPhysicalDeviceVideoFormatPropertiesKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceVideoFormatPropertiesKHR");
#endif /* defined(VK_KHR_video_queue) */
#if defined(VK_KHR_wayland_surface)
    vkCreateWaylandSurfaceKHR = (PFN_vkCreateWaylandSurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateWaylandSurfaceKHR");
    vkGetPhysicalDeviceWaylandPresentationSupportKHR = (PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceWaylandPresentationSupportKHR");
#endif /* defined(VK_KHR_wayland_surface) */
#if defined(VK_KHR_win32_surface)
    vkCreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");
    vkGetPhysicalDeviceWin32PresentationSupportKHR = (PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceWin32PresentationSupportKHR");
#endif /* defined(VK_KHR_win32_surface) */
#if defined(VK_KHR_xcb_surface)
    vkCreateXcbSurfaceKHR = (PFN_vkCreateXcbSurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateXcbSurfaceKHR");
    vkGetPhysicalDeviceXcbPresentationSupportKHR = (PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceXcbPresentationSupportKHR");
#endif /* defined(VK_KHR_xcb_surface) */
#if defined(VK_KHR_xlib_surface)
    vkCreateXlibSurfaceKHR = (PFN_vkCreateXlibSurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateXlibSurfaceKHR");
    vkGetPhysicalDeviceXlibPresentationSupportKHR = (PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceXlibPresentationSupportKHR");
#endif /* defined(VK_KHR_xlib_surface) */
#if defined(VK_MVK_ios_surface)
    vkCreateIOSSurfaceMVK = (PFN_vkCreateIOSSurfaceMVK)vkGetInstanceProcAddr(instance, "vkCreateIOSSurfaceMVK");
#endif /* defined(VK_MVK_ios_surface) */
#if defined(VK_MVK_macos_surface)
    vkCreateMacOSSurfaceMVK = (PFN_vkCreateMacOSSurfaceMVK)vkGetInstanceProcAddr(instance, "vkCreateMacOSSurfaceMVK");
#endif /* defined(VK_MVK_macos_surface) */
#if defined(VK_NN_vi_surface)
    vkCreateViSurfaceNN = (PFN_vkCreateViSurfaceNN)vkGetInstanceProcAddr(instance, "vkCreateViSurfaceNN");
#endif /* defined(VK_NN_vi_surface) */
#if defined(VK_NV_acquire_winrt_display)
    vkAcquireWinrtDisplayNV = (PFN_vkAcquireWinrtDisplayNV)vkGetInstanceProcAddr(instance, "vkAcquireWinrtDisplayNV");
    vkGetWinrtDisplayNV = (PFN_vkGetWinrtDisplayNV)vkGetInstanceProcAddr(instance, "vkGetWinrtDisplayNV");
#endif /* defined(VK_NV_acquire_winrt_display) */
#if defined(VK_NV_cooperative_matrix)
    vkGetPhysicalDeviceCooperativeMatrixPropertiesNV = (PFN_vkGetPhysicalDeviceCooperativeMatrixPropertiesNV)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceCooperativeMatrixPropertiesNV");
#endif /* defined(VK_NV_cooperative_matrix) */
#if defined(VK_NV_coverage_reduction_mode)
    vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV = (PFN_vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV");
#endif /* defined(VK_NV_coverage_reduction_mode) */
#if defined(VK_NV_external_memory_capabilities)
    vkGetPhysicalDeviceExternalImageFormatPropertiesNV = (PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceExternalImageFormatPropertiesNV");
#endif /* defined(VK_NV_external_memory_capabilities) */
#if defined(VK_NV_optical_flow)
    vkGetPhysicalDeviceOpticalFlowImageFormatsNV = (PFN_vkGetPhysicalDeviceOpticalFlowImageFormatsNV)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceOpticalFlowImageFormatsNV");
#endif /* defined(VK_NV_optical_flow) */
#if defined(VK_QNX_screen_surface)
    vkCreateScreenSurfaceQNX = (PFN_vkCreateScreenSurfaceQNX)vkGetInstanceProcAddr(instance, "vkCreateScreenSurfaceQNX");
    vkGetPhysicalDeviceScreenPresentationSupportQNX = (PFN_vkGetPhysicalDeviceScreenPresentationSupportQNX)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceScreenPresentationSupportQNX");
#endif /* defined(VK_QNX_screen_surface) */
#if (defined(VK_KHR_device_group) && defined(VK_KHR_surface)) || (defined(VK_KHR_swapchain) && defined(VK_VERSION_1_1))
    vkGetPhysicalDevicePresentRectanglesKHR = (PFN_vkGetPhysicalDevicePresentRectanglesKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDevicePresentRectanglesKHR");
#endif /* (defined(VK_KHR_device_group) && defined(VK_KHR_surface)) || (defined(VK_KHR_swapchain) && defined(VK_VERSION_1_1)) */
}

void wk_load_vulkan_device_funcs(VkDevice device)
{
#if defined(VK_VERSION_1_0)
    vkAllocateCommandBuffers = (PFN_vkAllocateCommandBuffers)vkGetDeviceProcAddr(device, "vkAllocateCommandBuffers");
    vkAllocateDescriptorSets = (PFN_vkAllocateDescriptorSets)vkGetDeviceProcAddr(device, "vkAllocateDescriptorSets");
    vkAllocateMemory = (PFN_vkAllocateMemory)vkGetDeviceProcAddr(device, "vkAllocateMemory");
    vkBeginCommandBuffer = (PFN_vkBeginCommandBuffer)vkGetDeviceProcAddr(device, "vkBeginCommandBuffer");
    vkBindBufferMemory = (PFN_vkBindBufferMemory)vkGetDeviceProcAddr(device, "vkBindBufferMemory");
    vkBindImageMemory = (PFN_vkBindImageMemory)vkGetDeviceProcAddr(device, "vkBindImageMemory");
    vkCmdBeginQuery = (PFN_vkCmdBeginQuery)vkGetDeviceProcAddr(device, "vkCmdBeginQuery");
    vkCmdBeginRenderPass = (PFN_vkCmdBeginRenderPass)vkGetDeviceProcAddr(device, "vkCmdBeginRenderPass");
    vkCmdBindDescriptorSets = (PFN_vkCmdBindDescriptorSets)vkGetDeviceProcAddr(device, "vkCmdBindDescriptorSets");
    vkCmdBindIndexBuffer = (PFN_vkCmdBindIndexBuffer)vkGetDeviceProcAddr(device, "vkCmdBindIndexBuffer");
    vkCmdBindPipeline = (PFN_vkCmdBindPipeline)vkGetDeviceProcAddr(device, "vkCmdBindPipeline");
    vkCmdBindVertexBuffers = (PFN_vkCmdBindVertexBuffers)vkGetDeviceProcAddr(device, "vkCmdBindVertexBuffers");
    vkCmdBlitImage = (PFN_vkCmdBlitImage)vkGetDeviceProcAddr(device, "vkCmdBlitImage");
    vkCmdClearAttachments = (PFN_vkCmdClearAttachments)vkGetDeviceProcAddr(device, "vkCmdClearAttachments");
    vkCmdClearColorImage = (PFN_vkCmdClearColorImage)vkGetDeviceProcAddr(device, "vkCmdClearColorImage");
    vkCmdClearDepthStencilImage = (PFN_vkCmdClearDepthStencilImage)vkGetDeviceProcAddr(device, "vkCmdClearDepthStencilImage");
    vkCmdCopyBuffer = (PFN_vkCmdCopyBuffer)vkGetDeviceProcAddr(device, "vkCmdCopyBuffer");
    vkCmdCopyBufferToImage = (PFN_vkCmdCopyBufferToImage)vkGetDeviceProcAddr(device, "vkCmdCopyBufferToImage");
    vkCmdCopyImage = (PFN_vkCmdCopyImage)vkGetDeviceProcAddr(device, "vkCmdCopyImage");
    vkCmdCopyImageToBuffer = (PFN_vkCmdCopyImageToBuffer)vkGetDeviceProcAddr(device, "vkCmdCopyImageToBuffer");
    vkCmdCopyQueryPoolResults = (PFN_vkCmdCopyQueryPoolResults)vkGetDeviceProcAddr(device, "vkCmdCopyQueryPoolResults");
    vkCmdDispatch = (PFN_vkCmdDispatch)vkGetDeviceProcAddr(device, "vkCmdDispatch");
    vkCmdDispatchIndirect = (PFN_vkCmdDispatchIndirect)vkGetDeviceProcAddr(device, "vkCmdDispatchIndirect");
    vkCmdDraw = (PFN_vkCmdDraw)vkGetDeviceProcAddr(device, "vkCmdDraw");
    vkCmdDrawIndexed = (PFN_vkCmdDrawIndexed)vkGetDeviceProcAddr(device, "vkCmdDrawIndexed");
    vkCmdDrawIndexedIndirect = (PFN_vkCmdDrawIndexedIndirect)vkGetDeviceProcAddr(device, "vkCmdDrawIndexedIndirect");
    vkCmdDrawIndirect = (PFN_vkCmdDrawIndirect)vkGetDeviceProcAddr(device, "vkCmdDrawIndirect");
    vkCmdEndQuery = (PFN_vkCmdEndQuery)vkGetDeviceProcAddr(device, "vkCmdEndQuery");
    vkCmdEndRenderPass = (PFN_vkCmdEndRenderPass)vkGetDeviceProcAddr(device, "vkCmdEndRenderPass");
    vkCmdExecuteCommands = (PFN_vkCmdExecuteCommands)vkGetDeviceProcAddr(device, "vkCmdExecuteCommands");
    vkCmdFillBuffer = (PFN_vkCmdFillBuffer)vkGetDeviceProcAddr(device, "vkCmdFillBuffer");
    vkCmdNextSubpass = (PFN_vkCmdNextSubpass)vkGetDeviceProcAddr(device, "vkCmdNextSubpass");
    vkCmdPipelineBarrier = (PFN_vkCmdPipelineBarrier)vkGetDeviceProcAddr(device, "vkCmdPipelineBarrier");
    vkCmdPushConstants = (PFN_vkCmdPushConstants)vkGetDeviceProcAddr(device, "vkCmdPushConstants");
    vkCmdResetEvent = (PFN_vkCmdResetEvent)vkGetDeviceProcAddr(device, "vkCmdResetEvent");
    vkCmdResetQueryPool = (PFN_vkCmdResetQueryPool)vkGetDeviceProcAddr(device, "vkCmdResetQueryPool");
    vkCmdResolveImage = (PFN_vkCmdResolveImage)vkGetDeviceProcAddr(device, "vkCmdResolveImage");
    vkCmdSetBlendConstants = (PFN_vkCmdSetBlendConstants)vkGetDeviceProcAddr(device, "vkCmdSetBlendConstants");
    vkCmdSetDepthBias = (PFN_vkCmdSetDepthBias)vkGetDeviceProcAddr(device, "vkCmdSetDepthBias");
    vkCmdSetDepthBounds = (PFN_vkCmdSetDepthBounds)vkGetDeviceProcAddr(device, "vkCmdSetDepthBounds");
    vkCmdSetEvent = (PFN_vkCmdSetEvent)vkGetDeviceProcAddr(device, "vkCmdSetEvent");
    vkCmdSetLineWidth = (PFN_vkCmdSetLineWidth)vkGetDeviceProcAddr(device, "vkCmdSetLineWidth");
    vkCmdSetScissor = (PFN_vkCmdSetScissor)vkGetDeviceProcAddr(device, "vkCmdSetScissor");
    vkCmdSetStencilCompareMask = (PFN_vkCmdSetStencilCompareMask)vkGetDeviceProcAddr(device, "vkCmdSetStencilCompareMask");
    vkCmdSetStencilReference = (PFN_vkCmdSetStencilReference)vkGetDeviceProcAddr(device, "vkCmdSetStencilReference");
    vkCmdSetStencilWriteMask = (PFN_vkCmdSetStencilWriteMask)vkGetDeviceProcAddr(device, "vkCmdSetStencilWriteMask");
    vkCmdSetViewport = (PFN_vkCmdSetViewport)vkGetDeviceProcAddr(device, "vkCmdSetViewport");
    vkCmdUpdateBuffer = (PFN_vkCmdUpdateBuffer)vkGetDeviceProcAddr(device, "vkCmdUpdateBuffer");
    vkCmdWaitEvents = (PFN_vkCmdWaitEvents)vkGetDeviceProcAddr(device, "vkCmdWaitEvents");
    vkCmdWriteTimestamp = (PFN_vkCmdWriteTimestamp)vkGetDeviceProcAddr(device, "vkCmdWriteTimestamp");
    vkCreateBuffer = (PFN_vkCreateBuffer)vkGetDeviceProcAddr(device, "vkCreateBuffer");
    vkCreateBufferView = (PFN_vkCreateBufferView)vkGetDeviceProcAddr(device, "vkCreateBufferView");
    vkCreateCommandPool = (PFN_vkCreateCommandPool)vkGetDeviceProcAddr(device, "vkCreateCommandPool");
    vkCreateComputePipelines = (PFN_vkCreateComputePipelines)vkGetDeviceProcAddr(device, "vkCreateComputePipelines");
    vkCreateDescriptorPool = (PFN_vkCreateDescriptorPool)vkGetDeviceProcAddr(device, "vkCreateDescriptorPool");
    vkCreateDescriptorSetLayout = (PFN_vkCreateDescriptorSetLayout)vkGetDeviceProcAddr(device, "vkCreateDescriptorSetLayout");
    vkCreateEvent = (PFN_vkCreateEvent)vkGetDeviceProcAddr(device, "vkCreateEvent");
    vkCreateFence = (PFN_vkCreateFence)vkGetDeviceProcAddr(device, "vkCreateFence");
    vkCreateFramebuffer = (PFN_vkCreateFramebuffer)vkGetDeviceProcAddr(device, "vkCreateFramebuffer");
    vkCreateGraphicsPipelines = (PFN_vkCreateGraphicsPipelines)vkGetDeviceProcAddr(device, "vkCreateGraphicsPipelines");
    vkCreateImage = (PFN_vkCreateImage)vkGetDeviceProcAddr(device, "vkCreateImage");
    vkCreateImageView = (PFN_vkCreateImageView)vkGetDeviceProcAddr(device, "vkCreateImageView");
    vkCreatePipelineCache = (PFN_vkCreatePipelineCache)vkGetDeviceProcAddr(device, "vkCreatePipelineCache");
    vkCreatePipelineLayout = (PFN_vkCreatePipelineLayout)vkGetDeviceProcAddr(device, "vkCreatePipelineLayout");
    vkCreateQueryPool = (PFN_vkCreateQueryPool)vkGetDeviceProcAddr(device, "vkCreateQueryPool");
    vkCreateRenderPass = (PFN_vkCreateRenderPass)vkGetDeviceProcAddr(device, "vkCreateRenderPass");
    vkCreateSampler = (PFN_vkCreateSampler)vkGetDeviceProcAddr(device, "vkCreateSampler");
    vkCreateSemaphore = (PFN_vkCreateSemaphore)vkGetDeviceProcAddr(device, "vkCreateSemaphore");
    vkCreateShaderModule = (PFN_vkCreateShaderModule)vkGetDeviceProcAddr(device, "vkCreateShaderModule");
    vkDestroyBuffer = (PFN_vkDestroyBuffer)vkGetDeviceProcAddr(device, "vkDestroyBuffer");
    vkDestroyBufferView = (PFN_vkDestroyBufferView)vkGetDeviceProcAddr(device, "vkDestroyBufferView");
    vkDestroyCommandPool = (PFN_vkDestroyCommandPool)vkGetDeviceProcAddr(device, "vkDestroyCommandPool");
    vkDestroyDescriptorPool = (PFN_vkDestroyDescriptorPool)vkGetDeviceProcAddr(device, "vkDestroyDescriptorPool");
    vkDestroyDescriptorSetLayout = (PFN_vkDestroyDescriptorSetLayout)vkGetDeviceProcAddr(device, "vkDestroyDescriptorSetLayout");
    vkDestroyDevice = (PFN_vkDestroyDevice)vkGetDeviceProcAddr(device, "vkDestroyDevice");
    vkDestroyEvent = (PFN_vkDestroyEvent)vkGetDeviceProcAddr(device, "vkDestroyEvent");
    vkDestroyFence = (PFN_vkDestroyFence)vkGetDeviceProcAddr(device, "vkDestroyFence");
    vkDestroyFramebuffer = (PFN_vkDestroyFramebuffer)vkGetDeviceProcAddr(device, "vkDestroyFramebuffer");
    vkDestroyImage = (PFN_vkDestroyImage)vkGetDeviceProcAddr(device, "vkDestroyImage");
    vkDestroyImageView = (PFN_vkDestroyImageView)vkGetDeviceProcAddr(device, "vkDestroyImageView");
    vkDestroyPipeline = (PFN_vkDestroyPipeline)vkGetDeviceProcAddr(device, "vkDestroyPipeline");
    vkDestroyPipelineCache = (PFN_vkDestroyPipelineCache)vkGetDeviceProcAddr(device, "vkDestroyPipelineCache");
    vkDestroyPipelineLayout = (PFN_vkDestroyPipelineLayout)vkGetDeviceProcAddr(device, "vkDestroyPipelineLayout");
    vkDestroyQueryPool = (PFN_vkDestroyQueryPool)vkGetDeviceProcAddr(device, "vkDestroyQueryPool");
    vkDestroyRenderPass = (PFN_vkDestroyRenderPass)vkGetDeviceProcAddr(device, "vkDestroyRenderPass");
    vkDestroySampler = (PFN_vkDestroySampler)vkGetDeviceProcAddr(device, "vkDestroySampler");
    vkDestroySemaphore = (PFN_vkDestroySemaphore)vkGetDeviceProcAddr(device, "vkDestroySemaphore");
    vkDestroyShaderModule = (PFN_vkDestroyShaderModule)vkGetDeviceProcAddr(device, "vkDestroyShaderModule");
    vkDeviceWaitIdle = (PFN_vkDeviceWaitIdle)vkGetDeviceProcAddr(device, "vkDeviceWaitIdle");
    vkEndCommandBuffer = (PFN_vkEndCommandBuffer)vkGetDeviceProcAddr(device, "vkEndCommandBuffer");
    vkFlushMappedMemoryRanges = (PFN_vkFlushMappedMemoryRanges)vkGetDeviceProcAddr(device, "vkFlushMappedMemoryRanges");
    vkFreeCommandBuffers = (PFN_vkFreeCommandBuffers)vkGetDeviceProcAddr(device, "vkFreeCommandBuffers");
    vkFreeDescriptorSets = (PFN_vkFreeDescriptorSets)vkGetDeviceProcAddr(device, "vkFreeDescriptorSets");
    vkFreeMemory = (PFN_vkFreeMemory)vkGetDeviceProcAddr(device, "vkFreeMemory");
    vkGetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements)vkGetDeviceProcAddr(device, "vkGetBufferMemoryRequirements");
    vkGetDeviceMemoryCommitment = (PFN_vkGetDeviceMemoryCommitment)vkGetDeviceProcAddr(device, "vkGetDeviceMemoryCommitment");
    vkGetDeviceQueue = (PFN_vkGetDeviceQueue)vkGetDeviceProcAddr(device, "vkGetDeviceQueue");
    vkGetEventStatus = (PFN_vkGetEventStatus)vkGetDeviceProcAddr(device, "vkGetEventStatus");
    vkGetFenceStatus = (PFN_vkGetFenceStatus)vkGetDeviceProcAddr(device, "vkGetFenceStatus");
    vkGetImageMemoryRequirements = (PFN_vkGetImageMemoryRequirements)vkGetDeviceProcAddr(device, "vkGetImageMemoryRequirements");
    vkGetImageSparseMemoryRequirements = (PFN_vkGetImageSparseMemoryRequirements)vkGetDeviceProcAddr(device, "vkGetImageSparseMemoryRequirements");
    vkGetImageSubresourceLayout = (PFN_vkGetImageSubresourceLayout)vkGetDeviceProcAddr(device, "vkGetImageSubresourceLayout");
    vkGetPipelineCacheData = (PFN_vkGetPipelineCacheData)vkGetDeviceProcAddr(device, "vkGetPipelineCacheData");
    vkGetQueryPoolResults = (PFN_vkGetQueryPoolResults)vkGetDeviceProcAddr(device, "vkGetQueryPoolResults");
    vkGetRenderAreaGranularity = (PFN_vkGetRenderAreaGranularity)vkGetDeviceProcAddr(device, "vkGetRenderAreaGranularity");
    vkInvalidateMappedMemoryRanges = (PFN_vkInvalidateMappedMemoryRanges)vkGetDeviceProcAddr(device, "vkInvalidateMappedMemoryRanges");
    vkMapMemory = (PFN_vkMapMemory)vkGetDeviceProcAddr(device, "vkMapMemory");
    vkMergePipelineCaches = (PFN_vkMergePipelineCaches)vkGetDeviceProcAddr(device, "vkMergePipelineCaches");
    vkQueueBindSparse = (PFN_vkQueueBindSparse)vkGetDeviceProcAddr(device, "vkQueueBindSparse");
    vkQueueSubmit = (PFN_vkQueueSubmit)vkGetDeviceProcAddr(device, "vkQueueSubmit");
    vkQueueWaitIdle = (PFN_vkQueueWaitIdle)vkGetDeviceProcAddr(device, "vkQueueWaitIdle");
    vkResetCommandBuffer = (PFN_vkResetCommandBuffer)vkGetDeviceProcAddr(device, "vkResetCommandBuffer");
    vkResetCommandPool = (PFN_vkResetCommandPool)vkGetDeviceProcAddr(device, "vkResetCommandPool");
    vkResetDescriptorPool = (PFN_vkResetDescriptorPool)vkGetDeviceProcAddr(device, "vkResetDescriptorPool");
    vkResetEvent = (PFN_vkResetEvent)vkGetDeviceProcAddr(device, "vkResetEvent");
    vkResetFences = (PFN_vkResetFences)vkGetDeviceProcAddr(device, "vkResetFences");
    vkSetEvent = (PFN_vkSetEvent)vkGetDeviceProcAddr(device, "vkSetEvent");
    vkUnmapMemory = (PFN_vkUnmapMemory)vkGetDeviceProcAddr(device, "vkUnmapMemory");
    vkUpdateDescriptorSets = (PFN_vkUpdateDescriptorSets)vkGetDeviceProcAddr(device, "vkUpdateDescriptorSets");
    vkWaitForFences = (PFN_vkWaitForFences)vkGetDeviceProcAddr(device, "vkWaitForFences");
#endif /* defined(VK_VERSION_1_0) */
#if defined(VK_VERSION_1_1)
    vkBindBufferMemory2 = (PFN_vkBindBufferMemory2)vkGetDeviceProcAddr(device, "vkBindBufferMemory2");
    vkBindImageMemory2 = (PFN_vkBindImageMemory2)vkGetDeviceProcAddr(device, "vkBindImageMemory2");
    vkCmdDispatchBase = (PFN_vkCmdDispatchBase)vkGetDeviceProcAddr(device, "vkCmdDispatchBase");
    vkCmdSetDeviceMask = (PFN_vkCmdSetDeviceMask)vkGetDeviceProcAddr(device, "vkCmdSetDeviceMask");
    vkCreateDescriptorUpdateTemplate = (PFN_vkCreateDescriptorUpdateTemplate)vkGetDeviceProcAddr(device, "vkCreateDescriptorUpdateTemplate");
    vkCreateSamplerYcbcrConversion = (PFN_vkCreateSamplerYcbcrConversion)vkGetDeviceProcAddr(device, "vkCreateSamplerYcbcrConversion");
    vkDestroyDescriptorUpdateTemplate = (PFN_vkDestroyDescriptorUpdateTemplate)vkGetDeviceProcAddr(device, "vkDestroyDescriptorUpdateTemplate");
    vkDestroySamplerYcbcrConversion = (PFN_vkDestroySamplerYcbcrConversion)vkGetDeviceProcAddr(device, "vkDestroySamplerYcbcrConversion");
    vkGetBufferMemoryRequirements2 = (PFN_vkGetBufferMemoryRequirements2)vkGetDeviceProcAddr(device, "vkGetBufferMemoryRequirements2");
    vkGetDescriptorSetLayoutSupport = (PFN_vkGetDescriptorSetLayoutSupport)vkGetDeviceProcAddr(device, "vkGetDescriptorSetLayoutSupport");
    vkGetDeviceGroupPeerMemoryFeatures = (PFN_vkGetDeviceGroupPeerMemoryFeatures)vkGetDeviceProcAddr(device, "vkGetDeviceGroupPeerMemoryFeatures");
    vkGetDeviceQueue2 = (PFN_vkGetDeviceQueue2)vkGetDeviceProcAddr(device, "vkGetDeviceQueue2");
    vkGetImageMemoryRequirements2 = (PFN_vkGetImageMemoryRequirements2)vkGetDeviceProcAddr(device, "vkGetImageMemoryRequirements2");
    vkGetImageSparseMemoryRequirements2 = (PFN_vkGetImageSparseMemoryRequirements2)vkGetDeviceProcAddr(device, "vkGetImageSparseMemoryRequirements2");
    vkTrimCommandPool = (PFN_vkTrimCommandPool)vkGetDeviceProcAddr(device, "vkTrimCommandPool");
    vkUpdateDescriptorSetWithTemplate = (PFN_vkUpdateDescriptorSetWithTemplate)vkGetDeviceProcAddr(device, "vkUpdateDescriptorSetWithTemplate");
#endif /* defined(VK_VERSION_1_1) */
#if defined(VK_VERSION_1_2)
    vkCmdBeginRenderPass2 = (PFN_vkCmdBeginRenderPass2)vkGetDeviceProcAddr(device, "vkCmdBeginRenderPass2");
    vkCmdDrawIndexedIndirectCount = (PFN_vkCmdDrawIndexedIndirectCount)vkGetDeviceProcAddr(device, "vkCmdDrawIndexedIndirectCount");
    vkCmdDrawIndirectCount = (PFN_vkCmdDrawIndirectCount)vkGetDeviceProcAddr(device, "vkCmdDrawIndirectCount");
    vkCmdEndRenderPass2 = (PFN_vkCmdEndRenderPass2)vkGetDeviceProcAddr(device, "vkCmdEndRenderPass2");
    vkCmdNextSubpass2 = (PFN_vkCmdNextSubpass2)vkGetDeviceProcAddr(device, "vkCmdNextSubpass2");
    vkCreateRenderPass2 = (PFN_vkCreateRenderPass2)vkGetDeviceProcAddr(device, "vkCreateRenderPass2");
    vkGetBufferDeviceAddress = (PFN_vkGetBufferDeviceAddress)vkGetDeviceProcAddr(device, "vkGetBufferDeviceAddress");
    vkGetBufferOpaqueCaptureAddress = (PFN_vkGetBufferOpaqueCaptureAddress)vkGetDeviceProcAddr(device, "vkGetBufferOpaqueCaptureAddress");
    vkGetDeviceMemoryOpaqueCaptureAddress = (PFN_vkGetDeviceMemoryOpaqueCaptureAddress)vkGetDeviceProcAddr(device, "vkGetDeviceMemoryOpaqueCaptureAddress");
    vkGetSemaphoreCounterValue = (PFN_vkGetSemaphoreCounterValue)vkGetDeviceProcAddr(device, "vkGetSemaphoreCounterValue");
    vkResetQueryPool = (PFN_vkResetQueryPool)vkGetDeviceProcAddr(device, "vkResetQueryPool");
    vkSignalSemaphore = (PFN_vkSignalSemaphore)vkGetDeviceProcAddr(device, "vkSignalSemaphore");
    vkWaitSemaphores = (PFN_vkWaitSemaphores)vkGetDeviceProcAddr(device, "vkWaitSemaphores");
#endif /* defined(VK_VERSION_1_2) */
#if defined(VK_VERSION_1_3)
    vkCmdBeginRendering = (PFN_vkCmdBeginRendering)vkGetDeviceProcAddr(device, "vkCmdBeginRendering");
    vkCmdBindVertexBuffers2 = (PFN_vkCmdBindVertexBuffers2)vkGetDeviceProcAddr(device, "vkCmdBindVertexBuffers2");
    vkCmdBlitImage2 = (PFN_vkCmdBlitImage2)vkGetDeviceProcAddr(device, "vkCmdBlitImage2");
    vkCmdCopyBuffer2 = (PFN_vkCmdCopyBuffer2)vkGetDeviceProcAddr(device, "vkCmdCopyBuffer2");
    vkCmdCopyBufferToImage2 = (PFN_vkCmdCopyBufferToImage2)vkGetDeviceProcAddr(device, "vkCmdCopyBufferToImage2");
    vkCmdCopyImage2 = (PFN_vkCmdCopyImage2)vkGetDeviceProcAddr(device, "vkCmdCopyImage2");
    vkCmdCopyImageToBuffer2 = (PFN_vkCmdCopyImageToBuffer2)vkGetDeviceProcAddr(device, "vkCmdCopyImageToBuffer2");
    vkCmdEndRendering = (PFN_vkCmdEndRendering)vkGetDeviceProcAddr(device, "vkCmdEndRendering");
    vkCmdPipelineBarrier2 = (PFN_vkCmdPipelineBarrier2)vkGetDeviceProcAddr(device, "vkCmdPipelineBarrier2");
    vkCmdResetEvent2 = (PFN_vkCmdResetEvent2)vkGetDeviceProcAddr(device, "vkCmdResetEvent2");
    vkCmdResolveImage2 = (PFN_vkCmdResolveImage2)vkGetDeviceProcAddr(device, "vkCmdResolveImage2");
    vkCmdSetCullMode = (PFN_vkCmdSetCullMode)vkGetDeviceProcAddr(device, "vkCmdSetCullMode");
    vkCmdSetDepthBiasEnable = (PFN_vkCmdSetDepthBiasEnable)vkGetDeviceProcAddr(device, "vkCmdSetDepthBiasEnable");
    vkCmdSetDepthBoundsTestEnable = (PFN_vkCmdSetDepthBoundsTestEnable)vkGetDeviceProcAddr(device, "vkCmdSetDepthBoundsTestEnable");
    vkCmdSetDepthCompareOp = (PFN_vkCmdSetDepthCompareOp)vkGetDeviceProcAddr(device, "vkCmdSetDepthCompareOp");
    vkCmdSetDepthTestEnable = (PFN_vkCmdSetDepthTestEnable)vkGetDeviceProcAddr(device, "vkCmdSetDepthTestEnable");
    vkCmdSetDepthWriteEnable = (PFN_vkCmdSetDepthWriteEnable)vkGetDeviceProcAddr(device, "vkCmdSetDepthWriteEnable");
    vkCmdSetEvent2 = (PFN_vkCmdSetEvent2)vkGetDeviceProcAddr(device, "vkCmdSetEvent2");
    vkCmdSetFrontFace = (PFN_vkCmdSetFrontFace)vkGetDeviceProcAddr(device, "vkCmdSetFrontFace");
    vkCmdSetPrimitiveRestartEnable = (PFN_vkCmdSetPrimitiveRestartEnable)vkGetDeviceProcAddr(device, "vkCmdSetPrimitiveRestartEnable");
    vkCmdSetPrimitiveTopology = (PFN_vkCmdSetPrimitiveTopology)vkGetDeviceProcAddr(device, "vkCmdSetPrimitiveTopology");
    vkCmdSetRasterizerDiscardEnable = (PFN_vkCmdSetRasterizerDiscardEnable)vkGetDeviceProcAddr(device, "vkCmdSetRasterizerDiscardEnable");
    vkCmdSetScissorWithCount = (PFN_vkCmdSetScissorWithCount)vkGetDeviceProcAddr(device, "vkCmdSetScissorWithCount");
    vkCmdSetStencilOp = (PFN_vkCmdSetStencilOp)vkGetDeviceProcAddr(device, "vkCmdSetStencilOp");
    vkCmdSetStencilTestEnable = (PFN_vkCmdSetStencilTestEnable)vkGetDeviceProcAddr(device, "vkCmdSetStencilTestEnable");
    vkCmdSetViewportWithCount = (PFN_vkCmdSetViewportWithCount)vkGetDeviceProcAddr(device, "vkCmdSetViewportWithCount");
    vkCmdWaitEvents2 = (PFN_vkCmdWaitEvents2)vkGetDeviceProcAddr(device, "vkCmdWaitEvents2");
    vkCmdWriteTimestamp2 = (PFN_vkCmdWriteTimestamp2)vkGetDeviceProcAddr(device, "vkCmdWriteTimestamp2");
    vkCreatePrivateDataSlot = (PFN_vkCreatePrivateDataSlot)vkGetDeviceProcAddr(device, "vkCreatePrivateDataSlot");
    vkDestroyPrivateDataSlot = (PFN_vkDestroyPrivateDataSlot)vkGetDeviceProcAddr(device, "vkDestroyPrivateDataSlot");
    vkGetDeviceBufferMemoryRequirements = (PFN_vkGetDeviceBufferMemoryRequirements)vkGetDeviceProcAddr(device, "vkGetDeviceBufferMemoryRequirements");
    vkGetDeviceImageMemoryRequirements = (PFN_vkGetDeviceImageMemoryRequirements)vkGetDeviceProcAddr(device, "vkGetDeviceImageMemoryRequirements");
    vkGetDeviceImageSparseMemoryRequirements = (PFN_vkGetDeviceImageSparseMemoryRequirements)vkGetDeviceProcAddr(device, "vkGetDeviceImageSparseMemoryRequirements");
    vkGetPrivateData = (PFN_vkGetPrivateData)vkGetDeviceProcAddr(device, "vkGetPrivateData");
    vkQueueSubmit2 = (PFN_vkQueueSubmit2)vkGetDeviceProcAddr(device, "vkQueueSubmit2");
    vkSetPrivateData = (PFN_vkSetPrivateData)vkGetDeviceProcAddr(device, "vkSetPrivateData");
#endif /* defined(VK_VERSION_1_3) */
#if defined(VK_AMDX_shader_enqueue)
    vkCmdDispatchGraphAMDX = (PFN_vkCmdDispatchGraphAMDX)vkGetDeviceProcAddr(device, "vkCmdDispatchGraphAMDX");
    vkCmdDispatchGraphIndirectAMDX = (PFN_vkCmdDispatchGraphIndirectAMDX)vkGetDeviceProcAddr(device, "vkCmdDispatchGraphIndirectAMDX");
    vkCmdDispatchGraphIndirectCountAMDX = (PFN_vkCmdDispatchGraphIndirectCountAMDX)vkGetDeviceProcAddr(device, "vkCmdDispatchGraphIndirectCountAMDX");
    vkCmdInitializeGraphScratchMemoryAMDX = (PFN_vkCmdInitializeGraphScratchMemoryAMDX)vkGetDeviceProcAddr(device, "vkCmdInitializeGraphScratchMemoryAMDX");
    vkCreateExecutionGraphPipelinesAMDX = (PFN_vkCreateExecutionGraphPipelinesAMDX)vkGetDeviceProcAddr(device, "vkCreateExecutionGraphPipelinesAMDX");
    vkGetExecutionGraphPipelineNodeIndexAMDX = (PFN_vkGetExecutionGraphPipelineNodeIndexAMDX)vkGetDeviceProcAddr(device, "vkGetExecutionGraphPipelineNodeIndexAMDX");
    vkGetExecutionGraphPipelineScratchSizeAMDX = (PFN_vkGetExecutionGraphPipelineScratchSizeAMDX)vkGetDeviceProcAddr(device, "vkGetExecutionGraphPipelineScratchSizeAMDX");
#endif /* defined(VK_AMDX_shader_enqueue) */
#if defined(VK_AMD_buffer_marker)
    vkCmdWriteBufferMarkerAMD = (PFN_vkCmdWriteBufferMarkerAMD)vkGetDeviceProcAddr(device, "vkCmdWriteBufferMarkerAMD");
#endif /* defined(VK_AMD_buffer_marker) */
#if defined(VK_AMD_display_native_hdr)
    vkSetLocalDimmingAMD = (PFN_vkSetLocalDimmingAMD)vkGetDeviceProcAddr(device, "vkSetLocalDimmingAMD");
#endif /* defined(VK_AMD_display_native_hdr) */
#if defined(VK_AMD_draw_indirect_count)
    vkCmdDrawIndexedIndirectCountAMD = (PFN_vkCmdDrawIndexedIndirectCountAMD)vkGetDeviceProcAddr(device, "vkCmdDrawIndexedIndirectCountAMD");
    vkCmdDrawIndirectCountAMD = (PFN_vkCmdDrawIndirectCountAMD)vkGetDeviceProcAddr(device, "vkCmdDrawIndirectCountAMD");
#endif /* defined(VK_AMD_draw_indirect_count) */
#if defined(VK_AMD_shader_info)
    vkGetShaderInfoAMD = (PFN_vkGetShaderInfoAMD)vkGetDeviceProcAddr(device, "vkGetShaderInfoAMD");
#endif /* defined(VK_AMD_shader_info) */
#if defined(VK_ANDROID_external_memory_android_hardware_buffer)
    vkGetAndroidHardwareBufferPropertiesANDROID = (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)vkGetDeviceProcAddr(device, "vkGetAndroidHardwareBufferPropertiesANDROID");
    vkGetMemoryAndroidHardwareBufferANDROID = (PFN_vkGetMemoryAndroidHardwareBufferANDROID)vkGetDeviceProcAddr(device, "vkGetMemoryAndroidHardwareBufferANDROID");
#endif /* defined(VK_ANDROID_external_memory_android_hardware_buffer) */
#if defined(VK_EXT_attachment_feedback_loop_dynamic_state)
    vkCmdSetAttachmentFeedbackLoopEnableEXT = (PFN_vkCmdSetAttachmentFeedbackLoopEnableEXT)vkGetDeviceProcAddr(device, "vkCmdSetAttachmentFeedbackLoopEnableEXT");
#endif /* defined(VK_EXT_attachment_feedback_loop_dynamic_state) */
#if defined(VK_EXT_buffer_device_address)
    vkGetBufferDeviceAddressEXT = (PFN_vkGetBufferDeviceAddressEXT)vkGetDeviceProcAddr(device, "vkGetBufferDeviceAddressEXT");
#endif /* defined(VK_EXT_buffer_device_address) */
#if defined(VK_EXT_calibrated_timestamps)
    vkGetCalibratedTimestampsEXT = (PFN_vkGetCalibratedTimestampsEXT)vkGetDeviceProcAddr(device, "vkGetCalibratedTimestampsEXT");
#endif /* defined(VK_EXT_calibrated_timestamps) */
#if defined(VK_EXT_color_write_enable)
    vkCmdSetColorWriteEnableEXT = (PFN_vkCmdSetColorWriteEnableEXT)vkGetDeviceProcAddr(device, "vkCmdSetColorWriteEnableEXT");
#endif /* defined(VK_EXT_color_write_enable) */
#if defined(VK_EXT_conditional_rendering)
    vkCmdBeginConditionalRenderingEXT = (PFN_vkCmdBeginConditionalRenderingEXT)vkGetDeviceProcAddr(device, "vkCmdBeginConditionalRenderingEXT");
    vkCmdEndConditionalRenderingEXT = (PFN_vkCmdEndConditionalRenderingEXT)vkGetDeviceProcAddr(device, "vkCmdEndConditionalRenderingEXT");
#endif /* defined(VK_EXT_conditional_rendering) */
#if defined(VK_EXT_debug_marker)
    vkCmdDebugMarkerBeginEXT = (PFN_vkCmdDebugMarkerBeginEXT)vkGetDeviceProcAddr(device, "vkCmdDebugMarkerBeginEXT");
    vkCmdDebugMarkerEndEXT = (PFN_vkCmdDebugMarkerEndEXT)vkGetDeviceProcAddr(device, "vkCmdDebugMarkerEndEXT");
    vkCmdDebugMarkerInsertEXT = (PFN_vkCmdDebugMarkerInsertEXT)vkGetDeviceProcAddr(device, "vkCmdDebugMarkerInsertEXT");
    vkDebugMarkerSetObjectNameEXT = (PFN_vkDebugMarkerSetObjectNameEXT)vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectNameEXT");
    vkDebugMarkerSetObjectTagEXT = (PFN_vkDebugMarkerSetObjectTagEXT)vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectTagEXT");
#endif /* defined(VK_EXT_debug_marker) */
#if defined(VK_EXT_depth_bias_control)
    vkCmdSetDepthBias2EXT = (PFN_vkCmdSetDepthBias2EXT)vkGetDeviceProcAddr(device, "vkCmdSetDepthBias2EXT");
#endif /* defined(VK_EXT_depth_bias_control) */
#if defined(VK_EXT_descriptor_buffer)
    vkCmdBindDescriptorBufferEmbeddedSamplersEXT = (PFN_vkCmdBindDescriptorBufferEmbeddedSamplersEXT)vkGetDeviceProcAddr(device, "vkCmdBindDescriptorBufferEmbeddedSamplersEXT");
    vkCmdBindDescriptorBuffersEXT = (PFN_vkCmdBindDescriptorBuffersEXT)vkGetDeviceProcAddr(device, "vkCmdBindDescriptorBuffersEXT");
    vkCmdSetDescriptorBufferOffsetsEXT = (PFN_vkCmdSetDescriptorBufferOffsetsEXT)vkGetDeviceProcAddr(device, "vkCmdSetDescriptorBufferOffsetsEXT");
    vkGetBufferOpaqueCaptureDescriptorDataEXT = (PFN_vkGetBufferOpaqueCaptureDescriptorDataEXT)vkGetDeviceProcAddr(device, "vkGetBufferOpaqueCaptureDescriptorDataEXT");
    vkGetDescriptorEXT = (PFN_vkGetDescriptorEXT)vkGetDeviceProcAddr(device, "vkGetDescriptorEXT");
    vkGetDescriptorSetLayoutBindingOffsetEXT = (PFN_vkGetDescriptorSetLayoutBindingOffsetEXT)vkGetDeviceProcAddr(device, "vkGetDescriptorSetLayoutBindingOffsetEXT");
    vkGetDescriptorSetLayoutSizeEXT = (PFN_vkGetDescriptorSetLayoutSizeEXT)vkGetDeviceProcAddr(device, "vkGetDescriptorSetLayoutSizeEXT");
    vkGetImageOpaqueCaptureDescriptorDataEXT = (PFN_vkGetImageOpaqueCaptureDescriptorDataEXT)vkGetDeviceProcAddr(device, "vkGetImageOpaqueCaptureDescriptorDataEXT");
    vkGetImageViewOpaqueCaptureDescriptorDataEXT = (PFN_vkGetImageViewOpaqueCaptureDescriptorDataEXT)vkGetDeviceProcAddr(device, "vkGetImageViewOpaqueCaptureDescriptorDataEXT");
    vkGetSamplerOpaqueCaptureDescriptorDataEXT = (PFN_vkGetSamplerOpaqueCaptureDescriptorDataEXT)vkGetDeviceProcAddr(device, "vkGetSamplerOpaqueCaptureDescriptorDataEXT");
#endif /* defined(VK_EXT_descriptor_buffer) */
#if defined(VK_EXT_descriptor_buffer) && (defined(VK_KHR_acceleration_structure) || defined(VK_NV_ray_tracing))
    vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT = (PFN_vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT)vkGetDeviceProcAddr(device, "vkGetAccelerationStructureOpaqueCaptureDescriptorDataEXT");
#endif /* defined(VK_EXT_descriptor_buffer) && (defined(VK_KHR_acceleration_structure) || defined(VK_NV_ray_tracing)) */
#if defined(VK_EXT_device_fault)
    vkGetDeviceFaultInfoEXT = (PFN_vkGetDeviceFaultInfoEXT)vkGetDeviceProcAddr(device, "vkGetDeviceFaultInfoEXT");
#endif /* defined(VK_EXT_device_fault) */
#if defined(VK_EXT_discard_rectangles)
    vkCmdSetDiscardRectangleEXT = (PFN_vkCmdSetDiscardRectangleEXT)vkGetDeviceProcAddr(device, "vkCmdSetDiscardRectangleEXT");
#endif /* defined(VK_EXT_discard_rectangles) */
#if defined(VK_EXT_discard_rectangles) && VK_EXT_DISCARD_RECTANGLES_SPEC_VERSION >= 2
    vkCmdSetDiscardRectangleEnableEXT = (PFN_vkCmdSetDiscardRectangleEnableEXT)vkGetDeviceProcAddr(device, "vkCmdSetDiscardRectangleEnableEXT");
    vkCmdSetDiscardRectangleModeEXT = (PFN_vkCmdSetDiscardRectangleModeEXT)vkGetDeviceProcAddr(device, "vkCmdSetDiscardRectangleModeEXT");
#endif /* defined(VK_EXT_discard_rectangles) && VK_EXT_DISCARD_RECTANGLES_SPEC_VERSION >= 2 */
#if defined(VK_EXT_display_control)
    vkDisplayPowerControlEXT = (PFN_vkDisplayPowerControlEXT)vkGetDeviceProcAddr(device, "vkDisplayPowerControlEXT");
    vkGetSwapchainCounterEXT = (PFN_vkGetSwapchainCounterEXT)vkGetDeviceProcAddr(device, "vkGetSwapchainCounterEXT");
    vkRegisterDeviceEventEXT = (PFN_vkRegisterDeviceEventEXT)vkGetDeviceProcAddr(device, "vkRegisterDeviceEventEXT");
    vkRegisterDisplayEventEXT = (PFN_vkRegisterDisplayEventEXT)vkGetDeviceProcAddr(device, "vkRegisterDisplayEventEXT");
#endif /* defined(VK_EXT_display_control) */
#if defined(VK_EXT_external_memory_host)
    vkGetMemoryHostPointerPropertiesEXT = (PFN_vkGetMemoryHostPointerPropertiesEXT)vkGetDeviceProcAddr(device, "vkGetMemoryHostPointerPropertiesEXT");
#endif /* defined(VK_EXT_external_memory_host) */
#if defined(VK_EXT_full_screen_exclusive)
    vkAcquireFullScreenExclusiveModeEXT = (PFN_vkAcquireFullScreenExclusiveModeEXT)vkGetDeviceProcAddr(device, "vkAcquireFullScreenExclusiveModeEXT");
    vkReleaseFullScreenExclusiveModeEXT = (PFN_vkReleaseFullScreenExclusiveModeEXT)vkGetDeviceProcAddr(device, "vkReleaseFullScreenExclusiveModeEXT");
#endif /* defined(VK_EXT_full_screen_exclusive) */
#if defined(VK_EXT_hdr_metadata)
    vkSetHdrMetadataEXT = (PFN_vkSetHdrMetadataEXT)vkGetDeviceProcAddr(device, "vkSetHdrMetadataEXT");
#endif /* defined(VK_EXT_hdr_metadata) */
#if defined(VK_EXT_host_image_copy)
    vkCopyImageToImageEXT = (PFN_vkCopyImageToImageEXT)vkGetDeviceProcAddr(device, "vkCopyImageToImageEXT");
    vkCopyImageToMemoryEXT = (PFN_vkCopyImageToMemoryEXT)vkGetDeviceProcAddr(device, "vkCopyImageToMemoryEXT");
    vkCopyMemoryToImageEXT = (PFN_vkCopyMemoryToImageEXT)vkGetDeviceProcAddr(device, "vkCopyMemoryToImageEXT");
    vkTransitionImageLayoutEXT = (PFN_vkTransitionImageLayoutEXT)vkGetDeviceProcAddr(device, "vkTransitionImageLayoutEXT");
#endif /* defined(VK_EXT_host_image_copy) */
#if defined(VK_EXT_host_query_reset)
    vkResetQueryPoolEXT = (PFN_vkResetQueryPoolEXT)vkGetDeviceProcAddr(device, "vkResetQueryPoolEXT");
#endif /* defined(VK_EXT_host_query_reset) */
#if defined(VK_EXT_image_drm_format_modifier)
    vkGetImageDrmFormatModifierPropertiesEXT = (PFN_vkGetImageDrmFormatModifierPropertiesEXT)vkGetDeviceProcAddr(device, "vkGetImageDrmFormatModifierPropertiesEXT");
#endif /* defined(VK_EXT_image_drm_format_modifier) */
#if defined(VK_EXT_line_rasterization)
    vkCmdSetLineStippleEXT = (PFN_vkCmdSetLineStippleEXT)vkGetDeviceProcAddr(device, "vkCmdSetLineStippleEXT");
#endif /* defined(VK_EXT_line_rasterization) */
#if defined(VK_EXT_mesh_shader)
    vkCmdDrawMeshTasksEXT = (PFN_vkCmdDrawMeshTasksEXT)vkGetDeviceProcAddr(device, "vkCmdDrawMeshTasksEXT");
    vkCmdDrawMeshTasksIndirectCountEXT = (PFN_vkCmdDrawMeshTasksIndirectCountEXT)vkGetDeviceProcAddr(device, "vkCmdDrawMeshTasksIndirectCountEXT");
    vkCmdDrawMeshTasksIndirectEXT = (PFN_vkCmdDrawMeshTasksIndirectEXT)vkGetDeviceProcAddr(device, "vkCmdDrawMeshTasksIndirectEXT");
#endif /* defined(VK_EXT_mesh_shader) */
#if defined(VK_EXT_metal_objects)
    vkExportMetalObjectsEXT = (PFN_vkExportMetalObjectsEXT)vkGetDeviceProcAddr(device, "vkExportMetalObjectsEXT");
#endif /* defined(VK_EXT_metal_objects) */
#if defined(VK_EXT_multi_draw)
    vkCmdDrawMultiEXT = (PFN_vkCmdDrawMultiEXT)vkGetDeviceProcAddr(device, "vkCmdDrawMultiEXT");
    vkCmdDrawMultiIndexedEXT = (PFN_vkCmdDrawMultiIndexedEXT)vkGetDeviceProcAddr(device, "vkCmdDrawMultiIndexedEXT");
#endif /* defined(VK_EXT_multi_draw) */
#if defined(VK_EXT_opacity_micromap)
    vkBuildMicromapsEXT = (PFN_vkBuildMicromapsEXT)vkGetDeviceProcAddr(device, "vkBuildMicromapsEXT");
    vkCmdBuildMicromapsEXT = (PFN_vkCmdBuildMicromapsEXT)vkGetDeviceProcAddr(device, "vkCmdBuildMicromapsEXT");
    vkCmdCopyMemoryToMicromapEXT = (PFN_vkCmdCopyMemoryToMicromapEXT)vkGetDeviceProcAddr(device, "vkCmdCopyMemoryToMicromapEXT");
    vkCmdCopyMicromapEXT = (PFN_vkCmdCopyMicromapEXT)vkGetDeviceProcAddr(device, "vkCmdCopyMicromapEXT");
    vkCmdCopyMicromapToMemoryEXT = (PFN_vkCmdCopyMicromapToMemoryEXT)vkGetDeviceProcAddr(device, "vkCmdCopyMicromapToMemoryEXT");
    vkCmdWriteMicromapsPropertiesEXT = (PFN_vkCmdWriteMicromapsPropertiesEXT)vkGetDeviceProcAddr(device, "vkCmdWriteMicromapsPropertiesEXT");
    vkCopyMemoryToMicromapEXT = (PFN_vkCopyMemoryToMicromapEXT)vkGetDeviceProcAddr(device, "vkCopyMemoryToMicromapEXT");
    vkCopyMicromapEXT = (PFN_vkCopyMicromapEXT)vkGetDeviceProcAddr(device, "vkCopyMicromapEXT");
    vkCopyMicromapToMemoryEXT = (PFN_vkCopyMicromapToMemoryEXT)vkGetDeviceProcAddr(device, "vkCopyMicromapToMemoryEXT");
    vkCreateMicromapEXT = (PFN_vkCreateMicromapEXT)vkGetDeviceProcAddr(device, "vkCreateMicromapEXT");
    vkDestroyMicromapEXT = (PFN_vkDestroyMicromapEXT)vkGetDeviceProcAddr(device, "vkDestroyMicromapEXT");
    vkGetDeviceMicromapCompatibilityEXT = (PFN_vkGetDeviceMicromapCompatibilityEXT)vkGetDeviceProcAddr(device, "vkGetDeviceMicromapCompatibilityEXT");
    vkGetMicromapBuildSizesEXT = (PFN_vkGetMicromapBuildSizesEXT)vkGetDeviceProcAddr(device, "vkGetMicromapBuildSizesEXT");
    vkWriteMicromapsPropertiesEXT = (PFN_vkWriteMicromapsPropertiesEXT)vkGetDeviceProcAddr(device, "vkWriteMicromapsPropertiesEXT");
#endif /* defined(VK_EXT_opacity_micromap) */
#if defined(VK_EXT_pageable_device_local_memory)
    vkSetDeviceMemoryPriorityEXT = (PFN_vkSetDeviceMemoryPriorityEXT)vkGetDeviceProcAddr(device, "vkSetDeviceMemoryPriorityEXT");
#endif /* defined(VK_EXT_pageable_device_local_memory) */
#if defined(VK_EXT_pipeline_properties)
    vkGetPipelinePropertiesEXT = (PFN_vkGetPipelinePropertiesEXT)vkGetDeviceProcAddr(device, "vkGetPipelinePropertiesEXT");
#endif /* defined(VK_EXT_pipeline_properties) */
#if defined(VK_EXT_private_data)
    vkCreatePrivateDataSlotEXT = (PFN_vkCreatePrivateDataSlotEXT)vkGetDeviceProcAddr(device, "vkCreatePrivateDataSlotEXT");
    vkDestroyPrivateDataSlotEXT = (PFN_vkDestroyPrivateDataSlotEXT)vkGetDeviceProcAddr(device, "vkDestroyPrivateDataSlotEXT");
    vkGetPrivateDataEXT = (PFN_vkGetPrivateDataEXT)vkGetDeviceProcAddr(device, "vkGetPrivateDataEXT");
    vkSetPrivateDataEXT = (PFN_vkSetPrivateDataEXT)vkGetDeviceProcAddr(device, "vkSetPrivateDataEXT");
#endif /* defined(VK_EXT_private_data) */
#if defined(VK_EXT_sample_locations)
    vkCmdSetSampleLocationsEXT = (PFN_vkCmdSetSampleLocationsEXT)vkGetDeviceProcAddr(device, "vkCmdSetSampleLocationsEXT");
#endif /* defined(VK_EXT_sample_locations) */
#if defined(VK_EXT_shader_module_identifier)
    vkGetShaderModuleCreateInfoIdentifierEXT = (PFN_vkGetShaderModuleCreateInfoIdentifierEXT)vkGetDeviceProcAddr(device, "vkGetShaderModuleCreateInfoIdentifierEXT");
    vkGetShaderModuleIdentifierEXT = (PFN_vkGetShaderModuleIdentifierEXT)vkGetDeviceProcAddr(device, "vkGetShaderModuleIdentifierEXT");
#endif /* defined(VK_EXT_shader_module_identifier) */
#if defined(VK_EXT_shader_object)
    vkCmdBindShadersEXT = (PFN_vkCmdBindShadersEXT)vkGetDeviceProcAddr(device, "vkCmdBindShadersEXT");
    vkCreateShadersEXT = (PFN_vkCreateShadersEXT)vkGetDeviceProcAddr(device, "vkCreateShadersEXT");
    vkDestroyShaderEXT = (PFN_vkDestroyShaderEXT)vkGetDeviceProcAddr(device, "vkDestroyShaderEXT");
    vkGetShaderBinaryDataEXT = (PFN_vkGetShaderBinaryDataEXT)vkGetDeviceProcAddr(device, "vkGetShaderBinaryDataEXT");
#endif /* defined(VK_EXT_shader_object) */
#if defined(VK_EXT_swapchain_maintenance1)
    vkReleaseSwapchainImagesEXT = (PFN_vkReleaseSwapchainImagesEXT)vkGetDeviceProcAddr(device, "vkReleaseSwapchainImagesEXT");
#endif /* defined(VK_EXT_swapchain_maintenance1) */
#if defined(VK_EXT_transform_feedback)
    vkCmdBeginQueryIndexedEXT = (PFN_vkCmdBeginQueryIndexedEXT)vkGetDeviceProcAddr(device, "vkCmdBeginQueryIndexedEXT");
    vkCmdBeginTransformFeedbackEXT = (PFN_vkCmdBeginTransformFeedbackEXT)vkGetDeviceProcAddr(device, "vkCmdBeginTransformFeedbackEXT");
    vkCmdBindTransformFeedbackBuffersEXT = (PFN_vkCmdBindTransformFeedbackBuffersEXT)vkGetDeviceProcAddr(device, "vkCmdBindTransformFeedbackBuffersEXT");
    vkCmdDrawIndirectByteCountEXT = (PFN_vkCmdDrawIndirectByteCountEXT)vkGetDeviceProcAddr(device, "vkCmdDrawIndirectByteCountEXT");
    vkCmdEndQueryIndexedEXT = (PFN_vkCmdEndQueryIndexedEXT)vkGetDeviceProcAddr(device, "vkCmdEndQueryIndexedEXT");
    vkCmdEndTransformFeedbackEXT = (PFN_vkCmdEndTransformFeedbackEXT)vkGetDeviceProcAddr(device, "vkCmdEndTransformFeedbackEXT");
#endif /* defined(VK_EXT_transform_feedback) */
#if defined(VK_EXT_validation_cache)
    vkCreateValidationCacheEXT = (PFN_vkCreateValidationCacheEXT)vkGetDeviceProcAddr(device, "vkCreateValidationCacheEXT");
    vkDestroyValidationCacheEXT = (PFN_vkDestroyValidationCacheEXT)vkGetDeviceProcAddr(device, "vkDestroyValidationCacheEXT");
    vkGetValidationCacheDataEXT = (PFN_vkGetValidationCacheDataEXT)vkGetDeviceProcAddr(device, "vkGetValidationCacheDataEXT");
    vkMergeValidationCachesEXT = (PFN_vkMergeValidationCachesEXT)vkGetDeviceProcAddr(device, "vkMergeValidationCachesEXT");
#endif /* defined(VK_EXT_validation_cache) */
#if defined(VK_FUCHSIA_buffer_collection)
    vkCreateBufferCollectionFUCHSIA = (PFN_vkCreateBufferCollectionFUCHSIA)vkGetDeviceProcAddr(device, "vkCreateBufferCollectionFUCHSIA");
    vkDestroyBufferCollectionFUCHSIA = (PFN_vkDestroyBufferCollectionFUCHSIA)vkGetDeviceProcAddr(device, "vkDestroyBufferCollectionFUCHSIA");
    vkGetBufferCollectionPropertiesFUCHSIA = (PFN_vkGetBufferCollectionPropertiesFUCHSIA)vkGetDeviceProcAddr(device, "vkGetBufferCollectionPropertiesFUCHSIA");
    vkSetBufferCollectionBufferConstraintsFUCHSIA = (PFN_vkSetBufferCollectionBufferConstraintsFUCHSIA)vkGetDeviceProcAddr(device, "vkSetBufferCollectionBufferConstraintsFUCHSIA");
    vkSetBufferCollectionImageConstraintsFUCHSIA = (PFN_vkSetBufferCollectionImageConstraintsFUCHSIA)vkGetDeviceProcAddr(device, "vkSetBufferCollectionImageConstraintsFUCHSIA");
#endif /* defined(VK_FUCHSIA_buffer_collection) */
#if defined(VK_FUCHSIA_external_memory)
    vkGetMemoryZirconHandleFUCHSIA = (PFN_vkGetMemoryZirconHandleFUCHSIA)vkGetDeviceProcAddr(device, "vkGetMemoryZirconHandleFUCHSIA");
    vkGetMemoryZirconHandlePropertiesFUCHSIA = (PFN_vkGetMemoryZirconHandlePropertiesFUCHSIA)vkGetDeviceProcAddr(device, "vkGetMemoryZirconHandlePropertiesFUCHSIA");
#endif /* defined(VK_FUCHSIA_external_memory) */
#if defined(VK_FUCHSIA_external_semaphore)
    vkGetSemaphoreZirconHandleFUCHSIA = (PFN_vkGetSemaphoreZirconHandleFUCHSIA)vkGetDeviceProcAddr(device, "vkGetSemaphoreZirconHandleFUCHSIA");
    vkImportSemaphoreZirconHandleFUCHSIA = (PFN_vkImportSemaphoreZirconHandleFUCHSIA)vkGetDeviceProcAddr(device, "vkImportSemaphoreZirconHandleFUCHSIA");
#endif /* defined(VK_FUCHSIA_external_semaphore) */
#if defined(VK_GOOGLE_display_timing)
    vkGetPastPresentationTimingGOOGLE = (PFN_vkGetPastPresentationTimingGOOGLE)vkGetDeviceProcAddr(device, "vkGetPastPresentationTimingGOOGLE");
    vkGetRefreshCycleDurationGOOGLE = (PFN_vkGetRefreshCycleDurationGOOGLE)vkGetDeviceProcAddr(device, "vkGetRefreshCycleDurationGOOGLE");
#endif /* defined(VK_GOOGLE_display_timing) */
#if defined(VK_HUAWEI_cluster_culling_shader)
    vkCmdDrawClusterHUAWEI = (PFN_vkCmdDrawClusterHUAWEI)vkGetDeviceProcAddr(device, "vkCmdDrawClusterHUAWEI");
    vkCmdDrawClusterIndirectHUAWEI = (PFN_vkCmdDrawClusterIndirectHUAWEI)vkGetDeviceProcAddr(device, "vkCmdDrawClusterIndirectHUAWEI");
#endif /* defined(VK_HUAWEI_cluster_culling_shader) */
#if defined(VK_HUAWEI_invocation_mask)
    vkCmdBindInvocationMaskHUAWEI = (PFN_vkCmdBindInvocationMaskHUAWEI)vkGetDeviceProcAddr(device, "vkCmdBindInvocationMaskHUAWEI");
#endif /* defined(VK_HUAWEI_invocation_mask) */
#if defined(VK_HUAWEI_subpass_shading)
    vkCmdSubpassShadingHUAWEI = (PFN_vkCmdSubpassShadingHUAWEI)vkGetDeviceProcAddr(device, "vkCmdSubpassShadingHUAWEI");
    vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI = (PFN_vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI)vkGetDeviceProcAddr(device, "vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI");
#endif /* defined(VK_HUAWEI_subpass_shading) */
#if defined(VK_INTEL_performance_query)
    vkAcquirePerformanceConfigurationINTEL = (PFN_vkAcquirePerformanceConfigurationINTEL)vkGetDeviceProcAddr(device, "vkAcquirePerformanceConfigurationINTEL");
    vkCmdSetPerformanceMarkerINTEL = (PFN_vkCmdSetPerformanceMarkerINTEL)vkGetDeviceProcAddr(device, "vkCmdSetPerformanceMarkerINTEL");
    vkCmdSetPerformanceOverrideINTEL = (PFN_vkCmdSetPerformanceOverrideINTEL)vkGetDeviceProcAddr(device, "vkCmdSetPerformanceOverrideINTEL");
    vkCmdSetPerformanceStreamMarkerINTEL = (PFN_vkCmdSetPerformanceStreamMarkerINTEL)vkGetDeviceProcAddr(device, "vkCmdSetPerformanceStreamMarkerINTEL");
    vkGetPerformanceParameterINTEL = (PFN_vkGetPerformanceParameterINTEL)vkGetDeviceProcAddr(device, "vkGetPerformanceParameterINTEL");
    vkInitializePerformanceApiINTEL = (PFN_vkInitializePerformanceApiINTEL)vkGetDeviceProcAddr(device, "vkInitializePerformanceApiINTEL");
    vkQueueSetPerformanceConfigurationINTEL = (PFN_vkQueueSetPerformanceConfigurationINTEL)vkGetDeviceProcAddr(device, "vkQueueSetPerformanceConfigurationINTEL");
    vkReleasePerformanceConfigurationINTEL = (PFN_vkReleasePerformanceConfigurationINTEL)vkGetDeviceProcAddr(device, "vkReleasePerformanceConfigurationINTEL");
    vkUninitializePerformanceApiINTEL = (PFN_vkUninitializePerformanceApiINTEL)vkGetDeviceProcAddr(device, "vkUninitializePerformanceApiINTEL");
#endif /* defined(VK_INTEL_performance_query) */
#if defined(VK_KHR_acceleration_structure)
    vkBuildAccelerationStructuresKHR = (PFN_vkBuildAccelerationStructuresKHR)vkGetDeviceProcAddr(device, "vkBuildAccelerationStructuresKHR");
    vkCmdBuildAccelerationStructuresIndirectKHR = (PFN_vkCmdBuildAccelerationStructuresIndirectKHR)vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructuresIndirectKHR");
    vkCmdBuildAccelerationStructuresKHR = (PFN_vkCmdBuildAccelerationStructuresKHR)vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructuresKHR");
    vkCmdCopyAccelerationStructureKHR = (PFN_vkCmdCopyAccelerationStructureKHR)vkGetDeviceProcAddr(device, "vkCmdCopyAccelerationStructureKHR");
    vkCmdCopyAccelerationStructureToMemoryKHR = (PFN_vkCmdCopyAccelerationStructureToMemoryKHR)vkGetDeviceProcAddr(device, "vkCmdCopyAccelerationStructureToMemoryKHR");
    vkCmdCopyMemoryToAccelerationStructureKHR = (PFN_vkCmdCopyMemoryToAccelerationStructureKHR)vkGetDeviceProcAddr(device, "vkCmdCopyMemoryToAccelerationStructureKHR");
    vkCmdWriteAccelerationStructuresPropertiesKHR = (PFN_vkCmdWriteAccelerationStructuresPropertiesKHR)vkGetDeviceProcAddr(device, "vkCmdWriteAccelerationStructuresPropertiesKHR");
    vkCopyAccelerationStructureKHR = (PFN_vkCopyAccelerationStructureKHR)vkGetDeviceProcAddr(device, "vkCopyAccelerationStructureKHR");
    vkCopyAccelerationStructureToMemoryKHR = (PFN_vkCopyAccelerationStructureToMemoryKHR)vkGetDeviceProcAddr(device, "vkCopyAccelerationStructureToMemoryKHR");
    vkCopyMemoryToAccelerationStructureKHR = (PFN_vkCopyMemoryToAccelerationStructureKHR)vkGetDeviceProcAddr(device, "vkCopyMemoryToAccelerationStructureKHR");
    vkCreateAccelerationStructureKHR = (PFN_vkCreateAccelerationStructureKHR)vkGetDeviceProcAddr(device, "vkCreateAccelerationStructureKHR");
    vkDestroyAccelerationStructureKHR = (PFN_vkDestroyAccelerationStructureKHR)vkGetDeviceProcAddr(device, "vkDestroyAccelerationStructureKHR");
    vkGetAccelerationStructureBuildSizesKHR = (PFN_vkGetAccelerationStructureBuildSizesKHR)vkGetDeviceProcAddr(device, "vkGetAccelerationStructureBuildSizesKHR");
    vkGetAccelerationStructureDeviceAddressKHR = (PFN_vkGetAccelerationStructureDeviceAddressKHR)vkGetDeviceProcAddr(device, "vkGetAccelerationStructureDeviceAddressKHR");
    vkGetDeviceAccelerationStructureCompatibilityKHR = (PFN_vkGetDeviceAccelerationStructureCompatibilityKHR)vkGetDeviceProcAddr(device, "vkGetDeviceAccelerationStructureCompatibilityKHR");
    vkWriteAccelerationStructuresPropertiesKHR = (PFN_vkWriteAccelerationStructuresPropertiesKHR)vkGetDeviceProcAddr(device, "vkWriteAccelerationStructuresPropertiesKHR");
#endif /* defined(VK_KHR_acceleration_structure) */
#if defined(VK_KHR_bind_memory2)
    vkBindBufferMemory2KHR = (PFN_vkBindBufferMemory2KHR)vkGetDeviceProcAddr(device, "vkBindBufferMemory2KHR");
    vkBindImageMemory2KHR = (PFN_vkBindImageMemory2KHR)vkGetDeviceProcAddr(device, "vkBindImageMemory2KHR");
#endif /* defined(VK_KHR_bind_memory2) */
#if defined(VK_KHR_buffer_device_address)
    vkGetBufferDeviceAddressKHR = (PFN_vkGetBufferDeviceAddressKHR)vkGetDeviceProcAddr(device, "vkGetBufferDeviceAddressKHR");
    vkGetBufferOpaqueCaptureAddressKHR = (PFN_vkGetBufferOpaqueCaptureAddressKHR)vkGetDeviceProcAddr(device, "vkGetBufferOpaqueCaptureAddressKHR");
    vkGetDeviceMemoryOpaqueCaptureAddressKHR = (PFN_vkGetDeviceMemoryOpaqueCaptureAddressKHR)vkGetDeviceProcAddr(device, "vkGetDeviceMemoryOpaqueCaptureAddressKHR");
#endif /* defined(VK_KHR_buffer_device_address) */
#if defined(VK_KHR_copy_commands2)
    vkCmdBlitImage2KHR = (PFN_vkCmdBlitImage2KHR)vkGetDeviceProcAddr(device, "vkCmdBlitImage2KHR");
    vkCmdCopyBuffer2KHR = (PFN_vkCmdCopyBuffer2KHR)vkGetDeviceProcAddr(device, "vkCmdCopyBuffer2KHR");
    vkCmdCopyBufferToImage2KHR = (PFN_vkCmdCopyBufferToImage2KHR)vkGetDeviceProcAddr(device, "vkCmdCopyBufferToImage2KHR");
    vkCmdCopyImage2KHR = (PFN_vkCmdCopyImage2KHR)vkGetDeviceProcAddr(device, "vkCmdCopyImage2KHR");
    vkCmdCopyImageToBuffer2KHR = (PFN_vkCmdCopyImageToBuffer2KHR)vkGetDeviceProcAddr(device, "vkCmdCopyImageToBuffer2KHR");
    vkCmdResolveImage2KHR = (PFN_vkCmdResolveImage2KHR)vkGetDeviceProcAddr(device, "vkCmdResolveImage2KHR");
#endif /* defined(VK_KHR_copy_commands2) */
#if defined(VK_KHR_create_renderpass2)
    vkCmdBeginRenderPass2KHR = (PFN_vkCmdBeginRenderPass2KHR)vkGetDeviceProcAddr(device, "vkCmdBeginRenderPass2KHR");
    vkCmdEndRenderPass2KHR = (PFN_vkCmdEndRenderPass2KHR)vkGetDeviceProcAddr(device, "vkCmdEndRenderPass2KHR");
    vkCmdNextSubpass2KHR = (PFN_vkCmdNextSubpass2KHR)vkGetDeviceProcAddr(device, "vkCmdNextSubpass2KHR");
    vkCreateRenderPass2KHR = (PFN_vkCreateRenderPass2KHR)vkGetDeviceProcAddr(device, "vkCreateRenderPass2KHR");
#endif /* defined(VK_KHR_create_renderpass2) */
#if defined(VK_KHR_deferred_host_operations)
    vkCreateDeferredOperationKHR = (PFN_vkCreateDeferredOperationKHR)vkGetDeviceProcAddr(device, "vkCreateDeferredOperationKHR");
    vkDeferredOperationJoinKHR = (PFN_vkDeferredOperationJoinKHR)vkGetDeviceProcAddr(device, "vkDeferredOperationJoinKHR");
    vkDestroyDeferredOperationKHR = (PFN_vkDestroyDeferredOperationKHR)vkGetDeviceProcAddr(device, "vkDestroyDeferredOperationKHR");
    vkGetDeferredOperationMaxConcurrencyKHR = (PFN_vkGetDeferredOperationMaxConcurrencyKHR)vkGetDeviceProcAddr(device, "vkGetDeferredOperationMaxConcurrencyKHR");
    vkGetDeferredOperationResultKHR = (PFN_vkGetDeferredOperationResultKHR)vkGetDeviceProcAddr(device, "vkGetDeferredOperationResultKHR");
#endif /* defined(VK_KHR_deferred_host_operations) */
#if defined(VK_KHR_descriptor_update_template)
    vkCreateDescriptorUpdateTemplateKHR = (PFN_vkCreateDescriptorUpdateTemplateKHR)vkGetDeviceProcAddr(device, "vkCreateDescriptorUpdateTemplateKHR");
    vkDestroyDescriptorUpdateTemplateKHR = (PFN_vkDestroyDescriptorUpdateTemplateKHR)vkGetDeviceProcAddr(device, "vkDestroyDescriptorUpdateTemplateKHR");
    vkUpdateDescriptorSetWithTemplateKHR = (PFN_vkUpdateDescriptorSetWithTemplateKHR)vkGetDeviceProcAddr(device, "vkUpdateDescriptorSetWithTemplateKHR");
#endif /* defined(VK_KHR_descriptor_update_template) */
#if defined(VK_KHR_device_group)
    vkCmdDispatchBaseKHR = (PFN_vkCmdDispatchBaseKHR)vkGetDeviceProcAddr(device, "vkCmdDispatchBaseKHR");
    vkCmdSetDeviceMaskKHR = (PFN_vkCmdSetDeviceMaskKHR)vkGetDeviceProcAddr(device, "vkCmdSetDeviceMaskKHR");
    vkGetDeviceGroupPeerMemoryFeaturesKHR = (PFN_vkGetDeviceGroupPeerMemoryFeaturesKHR)vkGetDeviceProcAddr(device, "vkGetDeviceGroupPeerMemoryFeaturesKHR");
#endif /* defined(VK_KHR_device_group) */
#if defined(VK_KHR_display_swapchain)
    vkCreateSharedSwapchainsKHR = (PFN_vkCreateSharedSwapchainsKHR)vkGetDeviceProcAddr(device, "vkCreateSharedSwapchainsKHR");
#endif /* defined(VK_KHR_display_swapchain) */
#if defined(VK_KHR_draw_indirect_count)
    vkCmdDrawIndexedIndirectCountKHR = (PFN_vkCmdDrawIndexedIndirectCountKHR)vkGetDeviceProcAddr(device, "vkCmdDrawIndexedIndirectCountKHR");
    vkCmdDrawIndirectCountKHR = (PFN_vkCmdDrawIndirectCountKHR)vkGetDeviceProcAddr(device, "vkCmdDrawIndirectCountKHR");
#endif /* defined(VK_KHR_draw_indirect_count) */
#if defined(VK_KHR_dynamic_rendering)
    vkCmdBeginRenderingKHR = (PFN_vkCmdBeginRenderingKHR)vkGetDeviceProcAddr(device, "vkCmdBeginRenderingKHR");
    vkCmdEndRenderingKHR = (PFN_vkCmdEndRenderingKHR)vkGetDeviceProcAddr(device, "vkCmdEndRenderingKHR");
#endif /* defined(VK_KHR_dynamic_rendering) */
#if defined(VK_KHR_external_fence_fd)
    vkGetFenceFdKHR = (PFN_vkGetFenceFdKHR)vkGetDeviceProcAddr(device, "vkGetFenceFdKHR");
    vkImportFenceFdKHR = (PFN_vkImportFenceFdKHR)vkGetDeviceProcAddr(device, "vkImportFenceFdKHR");
#endif /* defined(VK_KHR_external_fence_fd) */
#if defined(VK_KHR_external_fence_win32)
    vkGetFenceWin32HandleKHR = (PFN_vkGetFenceWin32HandleKHR)vkGetDeviceProcAddr(device, "vkGetFenceWin32HandleKHR");
    vkImportFenceWin32HandleKHR = (PFN_vkImportFenceWin32HandleKHR)vkGetDeviceProcAddr(device, "vkImportFenceWin32HandleKHR");
#endif /* defined(VK_KHR_external_fence_win32) */
#if defined(VK_KHR_external_memory_fd)
    vkGetMemoryFdKHR = (PFN_vkGetMemoryFdKHR)vkGetDeviceProcAddr(device, "vkGetMemoryFdKHR");
    vkGetMemoryFdPropertiesKHR = (PFN_vkGetMemoryFdPropertiesKHR)vkGetDeviceProcAddr(device, "vkGetMemoryFdPropertiesKHR");
#endif /* defined(VK_KHR_external_memory_fd) */
#if defined(VK_KHR_external_memory_win32)
    vkGetMemoryWin32HandleKHR = (PFN_vkGetMemoryWin32HandleKHR)vkGetDeviceProcAddr(device, "vkGetMemoryWin32HandleKHR");
    vkGetMemoryWin32HandlePropertiesKHR = (PFN_vkGetMemoryWin32HandlePropertiesKHR)vkGetDeviceProcAddr(device, "vkGetMemoryWin32HandlePropertiesKHR");
#endif /* defined(VK_KHR_external_memory_win32) */
#if defined(VK_KHR_external_semaphore_fd)
    vkGetSemaphoreFdKHR = (PFN_vkGetSemaphoreFdKHR)vkGetDeviceProcAddr(device, "vkGetSemaphoreFdKHR");
    vkImportSemaphoreFdKHR = (PFN_vkImportSemaphoreFdKHR)vkGetDeviceProcAddr(device, "vkImportSemaphoreFdKHR");
#endif /* defined(VK_KHR_external_semaphore_fd) */
#if defined(VK_KHR_external_semaphore_win32)
    vkGetSemaphoreWin32HandleKHR = (PFN_vkGetSemaphoreWin32HandleKHR)vkGetDeviceProcAddr(device, "vkGetSemaphoreWin32HandleKHR");
    vkImportSemaphoreWin32HandleKHR = (PFN_vkImportSemaphoreWin32HandleKHR)vkGetDeviceProcAddr(device, "vkImportSemaphoreWin32HandleKHR");
#endif /* defined(VK_KHR_external_semaphore_win32) */
#if defined(VK_KHR_fragment_shading_rate)
    vkCmdSetFragmentShadingRateKHR = (PFN_vkCmdSetFragmentShadingRateKHR)vkGetDeviceProcAddr(device, "vkCmdSetFragmentShadingRateKHR");
#endif /* defined(VK_KHR_fragment_shading_rate) */
#if defined(VK_KHR_get_memory_requirements2)
    vkGetBufferMemoryRequirements2KHR = (PFN_vkGetBufferMemoryRequirements2KHR)vkGetDeviceProcAddr(device, "vkGetBufferMemoryRequirements2KHR");
    vkGetImageMemoryRequirements2KHR = (PFN_vkGetImageMemoryRequirements2KHR)vkGetDeviceProcAddr(device, "vkGetImageMemoryRequirements2KHR");
    vkGetImageSparseMemoryRequirements2KHR = (PFN_vkGetImageSparseMemoryRequirements2KHR)vkGetDeviceProcAddr(device, "vkGetImageSparseMemoryRequirements2KHR");
#endif /* defined(VK_KHR_get_memory_requirements2) */
#if defined(VK_KHR_maintenance1)
    vkTrimCommandPoolKHR = (PFN_vkTrimCommandPoolKHR)vkGetDeviceProcAddr(device, "vkTrimCommandPoolKHR");
#endif /* defined(VK_KHR_maintenance1) */
#if defined(VK_KHR_maintenance3)
    vkGetDescriptorSetLayoutSupportKHR = (PFN_vkGetDescriptorSetLayoutSupportKHR)vkGetDeviceProcAddr(device, "vkGetDescriptorSetLayoutSupportKHR");
#endif /* defined(VK_KHR_maintenance3) */
#if defined(VK_KHR_maintenance4)
    vkGetDeviceBufferMemoryRequirementsKHR = (PFN_vkGetDeviceBufferMemoryRequirementsKHR)vkGetDeviceProcAddr(device, "vkGetDeviceBufferMemoryRequirementsKHR");
    vkGetDeviceImageMemoryRequirementsKHR = (PFN_vkGetDeviceImageMemoryRequirementsKHR)vkGetDeviceProcAddr(device, "vkGetDeviceImageMemoryRequirementsKHR");
    vkGetDeviceImageSparseMemoryRequirementsKHR = (PFN_vkGetDeviceImageSparseMemoryRequirementsKHR)vkGetDeviceProcAddr(device, "vkGetDeviceImageSparseMemoryRequirementsKHR");
#endif /* defined(VK_KHR_maintenance4) */
#if defined(VK_KHR_maintenance5)
    vkCmdBindIndexBuffer2KHR = (PFN_vkCmdBindIndexBuffer2KHR)vkGetDeviceProcAddr(device, "vkCmdBindIndexBuffer2KHR");
    vkGetDeviceImageSubresourceLayoutKHR = (PFN_vkGetDeviceImageSubresourceLayoutKHR)vkGetDeviceProcAddr(device, "vkGetDeviceImageSubresourceLayoutKHR");
    vkGetImageSubresourceLayout2KHR = (PFN_vkGetImageSubresourceLayout2KHR)vkGetDeviceProcAddr(device, "vkGetImageSubresourceLayout2KHR");
    vkGetRenderingAreaGranularityKHR = (PFN_vkGetRenderingAreaGranularityKHR)vkGetDeviceProcAddr(device, "vkGetRenderingAreaGranularityKHR");
#endif /* defined(VK_KHR_maintenance5) */
#if defined(VK_KHR_map_memory2)
    vkMapMemory2KHR = (PFN_vkMapMemory2KHR)vkGetDeviceProcAddr(device, "vkMapMemory2KHR");
    vkUnmapMemory2KHR = (PFN_vkUnmapMemory2KHR)vkGetDeviceProcAddr(device, "vkUnmapMemory2KHR");
#endif /* defined(VK_KHR_map_memory2) */
#if defined(VK_KHR_performance_query)
    vkAcquireProfilingLockKHR = (PFN_vkAcquireProfilingLockKHR)vkGetDeviceProcAddr(device, "vkAcquireProfilingLockKHR");
    vkReleaseProfilingLockKHR = (PFN_vkReleaseProfilingLockKHR)vkGetDeviceProcAddr(device, "vkReleaseProfilingLockKHR");
#endif /* defined(VK_KHR_performance_query) */
#if defined(VK_KHR_pipeline_executable_properties)
    vkGetPipelineExecutableInternalRepresentationsKHR = (PFN_vkGetPipelineExecutableInternalRepresentationsKHR)vkGetDeviceProcAddr(device, "vkGetPipelineExecutableInternalRepresentationsKHR");
    vkGetPipelineExecutablePropertiesKHR = (PFN_vkGetPipelineExecutablePropertiesKHR)vkGetDeviceProcAddr(device, "vkGetPipelineExecutablePropertiesKHR");
    vkGetPipelineExecutableStatisticsKHR = (PFN_vkGetPipelineExecutableStatisticsKHR)vkGetDeviceProcAddr(device, "vkGetPipelineExecutableStatisticsKHR");
#endif /* defined(VK_KHR_pipeline_executable_properties) */
#if defined(VK_KHR_present_wait)
    vkWaitForPresentKHR = (PFN_vkWaitForPresentKHR)vkGetDeviceProcAddr(device, "vkWaitForPresentKHR");
#endif /* defined(VK_KHR_present_wait) */
#if defined(VK_KHR_push_descriptor)
    vkCmdPushDescriptorSetKHR = (PFN_vkCmdPushDescriptorSetKHR)vkGetDeviceProcAddr(device, "vkCmdPushDescriptorSetKHR");
#endif /* defined(VK_KHR_push_descriptor) */
#if defined(VK_KHR_ray_tracing_maintenance1) && defined(VK_KHR_ray_tracing_pipeline)
    vkCmdTraceRaysIndirect2KHR = (PFN_vkCmdTraceRaysIndirect2KHR)vkGetDeviceProcAddr(device, "vkCmdTraceRaysIndirect2KHR");
#endif /* defined(VK_KHR_ray_tracing_maintenance1) && defined(VK_KHR_ray_tracing_pipeline) */
#if defined(VK_KHR_ray_tracing_pipeline)
    vkCmdSetRayTracingPipelineStackSizeKHR = (PFN_vkCmdSetRayTracingPipelineStackSizeKHR)vkGetDeviceProcAddr(device, "vkCmdSetRayTracingPipelineStackSizeKHR");
    vkCmdTraceRaysIndirectKHR = (PFN_vkCmdTraceRaysIndirectKHR)vkGetDeviceProcAddr(device, "vkCmdTraceRaysIndirectKHR");
    vkCmdTraceRaysKHR = (PFN_vkCmdTraceRaysKHR)vkGetDeviceProcAddr(device, "vkCmdTraceRaysKHR");
    vkCreateRayTracingPipelinesKHR = (PFN_vkCreateRayTracingPipelinesKHR)vkGetDeviceProcAddr(device, "vkCreateRayTracingPipelinesKHR");
    vkGetRayTracingCaptureReplayShaderGroupHandlesKHR = (PFN_vkGetRayTracingCaptureReplayShaderGroupHandlesKHR)vkGetDeviceProcAddr(device, "vkGetRayTracingCaptureReplayShaderGroupHandlesKHR");
    vkGetRayTracingShaderGroupHandlesKHR = (PFN_vkGetRayTracingShaderGroupHandlesKHR)vkGetDeviceProcAddr(device, "vkGetRayTracingShaderGroupHandlesKHR");
    vkGetRayTracingShaderGroupStackSizeKHR = (PFN_vkGetRayTracingShaderGroupStackSizeKHR)vkGetDeviceProcAddr(device, "vkGetRayTracingShaderGroupStackSizeKHR");
#endif /* defined(VK_KHR_ray_tracing_pipeline) */
#if defined(VK_KHR_sampler_ycbcr_conversion)
    vkCreateSamplerYcbcrConversionKHR = (PFN_vkCreateSamplerYcbcrConversionKHR)vkGetDeviceProcAddr(device, "vkCreateSamplerYcbcrConversionKHR");
    vkDestroySamplerYcbcrConversionKHR = (PFN_vkDestroySamplerYcbcrConversionKHR)vkGetDeviceProcAddr(device, "vkDestroySamplerYcbcrConversionKHR");
#endif /* defined(VK_KHR_sampler_ycbcr_conversion) */
#if defined(VK_KHR_shared_presentable_image)
    vkGetSwapchainStatusKHR = (PFN_vkGetSwapchainStatusKHR)vkGetDeviceProcAddr(device, "vkGetSwapchainStatusKHR");
#endif /* defined(VK_KHR_shared_presentable_image) */
#if defined(VK_KHR_swapchain)
    vkAcquireNextImageKHR = (PFN_vkAcquireNextImageKHR)vkGetDeviceProcAddr(device, "vkAcquireNextImageKHR");
    vkCreateSwapchainKHR = (PFN_vkCreateSwapchainKHR)vkGetDeviceProcAddr(device, "vkCreateSwapchainKHR");
    vkDestroySwapchainKHR = (PFN_vkDestroySwapchainKHR)vkGetDeviceProcAddr(device, "vkDestroySwapchainKHR");
    vkGetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR)vkGetDeviceProcAddr(device, "vkGetSwapchainImagesKHR");
    vkQueuePresentKHR = (PFN_vkQueuePresentKHR)vkGetDeviceProcAddr(device, "vkQueuePresentKHR");
#endif /* defined(VK_KHR_swapchain) */
#if defined(VK_KHR_synchronization2)
    vkCmdPipelineBarrier2KHR = (PFN_vkCmdPipelineBarrier2KHR)vkGetDeviceProcAddr(device, "vkCmdPipelineBarrier2KHR");
    vkCmdResetEvent2KHR = (PFN_vkCmdResetEvent2KHR)vkGetDeviceProcAddr(device, "vkCmdResetEvent2KHR");
    vkCmdSetEvent2KHR = (PFN_vkCmdSetEvent2KHR)vkGetDeviceProcAddr(device, "vkCmdSetEvent2KHR");
    vkCmdWaitEvents2KHR = (PFN_vkCmdWaitEvents2KHR)vkGetDeviceProcAddr(device, "vkCmdWaitEvents2KHR");
    vkCmdWriteTimestamp2KHR = (PFN_vkCmdWriteTimestamp2KHR)vkGetDeviceProcAddr(device, "vkCmdWriteTimestamp2KHR");
    vkQueueSubmit2KHR = (PFN_vkQueueSubmit2KHR)vkGetDeviceProcAddr(device, "vkQueueSubmit2KHR");
#endif /* defined(VK_KHR_synchronization2) */
#if defined(VK_KHR_synchronization2) && defined(VK_AMD_buffer_marker)
    vkCmdWriteBufferMarker2AMD = (PFN_vkCmdWriteBufferMarker2AMD)vkGetDeviceProcAddr(device, "vkCmdWriteBufferMarker2AMD");
#endif /* defined(VK_KHR_synchronization2) && defined(VK_AMD_buffer_marker) */
#if defined(VK_KHR_synchronization2) && defined(VK_NV_device_diagnostic_checkpoints)
    vkGetQueueCheckpointData2NV = (PFN_vkGetQueueCheckpointData2NV)vkGetDeviceProcAddr(device, "vkGetQueueCheckpointData2NV");
#endif /* defined(VK_KHR_synchronization2) && defined(VK_NV_device_diagnostic_checkpoints) */
#if defined(VK_KHR_timeline_semaphore)
    vkGetSemaphoreCounterValueKHR = (PFN_vkGetSemaphoreCounterValueKHR)vkGetDeviceProcAddr(device, "vkGetSemaphoreCounterValueKHR");
    vkSignalSemaphoreKHR = (PFN_vkSignalSemaphoreKHR)vkGetDeviceProcAddr(device, "vkSignalSemaphoreKHR");
    vkWaitSemaphoresKHR = (PFN_vkWaitSemaphoresKHR)vkGetDeviceProcAddr(device, "vkWaitSemaphoresKHR");
#endif /* defined(VK_KHR_timeline_semaphore) */
#if defined(VK_KHR_video_decode_queue)
    vkCmdDecodeVideoKHR = (PFN_vkCmdDecodeVideoKHR)vkGetDeviceProcAddr(device, "vkCmdDecodeVideoKHR");
#endif /* defined(VK_KHR_video_decode_queue) */
#if defined(VK_KHR_video_encode_queue)
    vkCmdEncodeVideoKHR = (PFN_vkCmdEncodeVideoKHR)vkGetDeviceProcAddr(device, "vkCmdEncodeVideoKHR");
    vkGetEncodedVideoSessionParametersKHR = (PFN_vkGetEncodedVideoSessionParametersKHR)vkGetDeviceProcAddr(device, "vkGetEncodedVideoSessionParametersKHR");
#endif /* defined(VK_KHR_video_encode_queue) */
#if defined(VK_KHR_video_queue)
    vkBindVideoSessionMemoryKHR = (PFN_vkBindVideoSessionMemoryKHR)vkGetDeviceProcAddr(device, "vkBindVideoSessionMemoryKHR");
    vkCmdBeginVideoCodingKHR = (PFN_vkCmdBeginVideoCodingKHR)vkGetDeviceProcAddr(device, "vkCmdBeginVideoCodingKHR");
    vkCmdControlVideoCodingKHR = (PFN_vkCmdControlVideoCodingKHR)vkGetDeviceProcAddr(device, "vkCmdControlVideoCodingKHR");
    vkCmdEndVideoCodingKHR = (PFN_vkCmdEndVideoCodingKHR)vkGetDeviceProcAddr(device, "vkCmdEndVideoCodingKHR");
    vkCreateVideoSessionKHR = (PFN_vkCreateVideoSessionKHR)vkGetDeviceProcAddr(device, "vkCreateVideoSessionKHR");
    vkCreateVideoSessionParametersKHR = (PFN_vkCreateVideoSessionParametersKHR)vkGetDeviceProcAddr(device, "vkCreateVideoSessionParametersKHR");
    vkDestroyVideoSessionKHR = (PFN_vkDestroyVideoSessionKHR)vkGetDeviceProcAddr(device, "vkDestroyVideoSessionKHR");
    vkDestroyVideoSessionParametersKHR = (PFN_vkDestroyVideoSessionParametersKHR)vkGetDeviceProcAddr(device, "vkDestroyVideoSessionParametersKHR");
    vkGetVideoSessionMemoryRequirementsKHR = (PFN_vkGetVideoSessionMemoryRequirementsKHR)vkGetDeviceProcAddr(device, "vkGetVideoSessionMemoryRequirementsKHR");
    vkUpdateVideoSessionParametersKHR = (PFN_vkUpdateVideoSessionParametersKHR)vkGetDeviceProcAddr(device, "vkUpdateVideoSessionParametersKHR");
#endif /* defined(VK_KHR_video_queue) */
#if defined(VK_NVX_binary_import)
    vkCmdCuLaunchKernelNVX = (PFN_vkCmdCuLaunchKernelNVX)vkGetDeviceProcAddr(device, "vkCmdCuLaunchKernelNVX");
    vkCreateCuFunctionNVX = (PFN_vkCreateCuFunctionNVX)vkGetDeviceProcAddr(device, "vkCreateCuFunctionNVX");
    vkCreateCuModuleNVX = (PFN_vkCreateCuModuleNVX)vkGetDeviceProcAddr(device, "vkCreateCuModuleNVX");
    vkDestroyCuFunctionNVX = (PFN_vkDestroyCuFunctionNVX)vkGetDeviceProcAddr(device, "vkDestroyCuFunctionNVX");
    vkDestroyCuModuleNVX = (PFN_vkDestroyCuModuleNVX)vkGetDeviceProcAddr(device, "vkDestroyCuModuleNVX");
#endif /* defined(VK_NVX_binary_import) */
#if defined(VK_NVX_image_view_handle)
    vkGetImageViewAddressNVX = (PFN_vkGetImageViewAddressNVX)vkGetDeviceProcAddr(device, "vkGetImageViewAddressNVX");
    vkGetImageViewHandleNVX = (PFN_vkGetImageViewHandleNVX)vkGetDeviceProcAddr(device, "vkGetImageViewHandleNVX");
#endif /* defined(VK_NVX_image_view_handle) */
#if defined(VK_NV_clip_space_w_scaling)
    vkCmdSetViewportWScalingNV = (PFN_vkCmdSetViewportWScalingNV)vkGetDeviceProcAddr(device, "vkCmdSetViewportWScalingNV");
#endif /* defined(VK_NV_clip_space_w_scaling) */
#if defined(VK_NV_copy_memory_indirect)
    vkCmdCopyMemoryIndirectNV = (PFN_vkCmdCopyMemoryIndirectNV)vkGetDeviceProcAddr(device, "vkCmdCopyMemoryIndirectNV");
    vkCmdCopyMemoryToImageIndirectNV = (PFN_vkCmdCopyMemoryToImageIndirectNV)vkGetDeviceProcAddr(device, "vkCmdCopyMemoryToImageIndirectNV");
#endif /* defined(VK_NV_copy_memory_indirect) */
#if defined(VK_NV_cuda_kernel_launch)
    vkCmdCudaLaunchKernelNV = (PFN_vkCmdCudaLaunchKernelNV)vkGetDeviceProcAddr(device, "vkCmdCudaLaunchKernelNV");
    vkCreateCudaFunctionNV = (PFN_vkCreateCudaFunctionNV)vkGetDeviceProcAddr(device, "vkCreateCudaFunctionNV");
    vkCreateCudaModuleNV = (PFN_vkCreateCudaModuleNV)vkGetDeviceProcAddr(device, "vkCreateCudaModuleNV");
    vkDestroyCudaFunctionNV = (PFN_vkDestroyCudaFunctionNV)vkGetDeviceProcAddr(device, "vkDestroyCudaFunctionNV");
    vkDestroyCudaModuleNV = (PFN_vkDestroyCudaModuleNV)vkGetDeviceProcAddr(device, "vkDestroyCudaModuleNV");
    vkGetCudaModuleCacheNV = (PFN_vkGetCudaModuleCacheNV)vkGetDeviceProcAddr(device, "vkGetCudaModuleCacheNV");
#endif /* defined(VK_NV_cuda_kernel_launch) */
#if defined(VK_NV_device_diagnostic_checkpoints)
    vkCmdSetCheckpointNV = (PFN_vkCmdSetCheckpointNV)vkGetDeviceProcAddr(device, "vkCmdSetCheckpointNV");
    vkGetQueueCheckpointDataNV = (PFN_vkGetQueueCheckpointDataNV)vkGetDeviceProcAddr(device, "vkGetQueueCheckpointDataNV");
#endif /* defined(VK_NV_device_diagnostic_checkpoints) */
#if defined(VK_NV_device_generated_commands)
    vkCmdBindPipelineShaderGroupNV = (PFN_vkCmdBindPipelineShaderGroupNV)vkGetDeviceProcAddr(device, "vkCmdBindPipelineShaderGroupNV");
    vkCmdExecuteGeneratedCommandsNV = (PFN_vkCmdExecuteGeneratedCommandsNV)vkGetDeviceProcAddr(device, "vkCmdExecuteGeneratedCommandsNV");
    vkCmdPreprocessGeneratedCommandsNV = (PFN_vkCmdPreprocessGeneratedCommandsNV)vkGetDeviceProcAddr(device, "vkCmdPreprocessGeneratedCommandsNV");
    vkCreateIndirectCommandsLayoutNV = (PFN_vkCreateIndirectCommandsLayoutNV)vkGetDeviceProcAddr(device, "vkCreateIndirectCommandsLayoutNV");
    vkDestroyIndirectCommandsLayoutNV = (PFN_vkDestroyIndirectCommandsLayoutNV)vkGetDeviceProcAddr(device, "vkDestroyIndirectCommandsLayoutNV");
    vkGetGeneratedCommandsMemoryRequirementsNV = (PFN_vkGetGeneratedCommandsMemoryRequirementsNV)vkGetDeviceProcAddr(device, "vkGetGeneratedCommandsMemoryRequirementsNV");
#endif /* defined(VK_NV_device_generated_commands) */
#if defined(VK_NV_device_generated_commands_compute)
    vkCmdUpdatePipelineIndirectBufferNV = (PFN_vkCmdUpdatePipelineIndirectBufferNV)vkGetDeviceProcAddr(device, "vkCmdUpdatePipelineIndirectBufferNV");
    vkGetPipelineIndirectDeviceAddressNV = (PFN_vkGetPipelineIndirectDeviceAddressNV)vkGetDeviceProcAddr(device, "vkGetPipelineIndirectDeviceAddressNV");
    vkGetPipelineIndirectMemoryRequirementsNV = (PFN_vkGetPipelineIndirectMemoryRequirementsNV)vkGetDeviceProcAddr(device, "vkGetPipelineIndirectMemoryRequirementsNV");
#endif /* defined(VK_NV_device_generated_commands_compute) */
#if defined(VK_NV_external_memory_rdma)
    vkGetMemoryRemoteAddressNV = (PFN_vkGetMemoryRemoteAddressNV)vkGetDeviceProcAddr(device, "vkGetMemoryRemoteAddressNV");
#endif /* defined(VK_NV_external_memory_rdma) */
#if defined(VK_NV_external_memory_win32)
    vkGetMemoryWin32HandleNV = (PFN_vkGetMemoryWin32HandleNV)vkGetDeviceProcAddr(device, "vkGetMemoryWin32HandleNV");
#endif /* defined(VK_NV_external_memory_win32) */
#if defined(VK_NV_fragment_shading_rate_enums)
    vkCmdSetFragmentShadingRateEnumNV = (PFN_vkCmdSetFragmentShadingRateEnumNV)vkGetDeviceProcAddr(device, "vkCmdSetFragmentShadingRateEnumNV");
#endif /* defined(VK_NV_fragment_shading_rate_enums) */
#if defined(VK_NV_low_latency2)
    vkGetLatencyTimingsNV = (PFN_vkGetLatencyTimingsNV)vkGetDeviceProcAddr(device, "vkGetLatencyTimingsNV");
    vkLatencySleepNV = (PFN_vkLatencySleepNV)vkGetDeviceProcAddr(device, "vkLatencySleepNV");
    vkQueueNotifyOutOfBandNV = (PFN_vkQueueNotifyOutOfBandNV)vkGetDeviceProcAddr(device, "vkQueueNotifyOutOfBandNV");
    vkSetLatencyMarkerNV = (PFN_vkSetLatencyMarkerNV)vkGetDeviceProcAddr(device, "vkSetLatencyMarkerNV");
    vkSetLatencySleepModeNV = (PFN_vkSetLatencySleepModeNV)vkGetDeviceProcAddr(device, "vkSetLatencySleepModeNV");
#endif /* defined(VK_NV_low_latency2) */
#if defined(VK_NV_memory_decompression)
    vkCmdDecompressMemoryIndirectCountNV = (PFN_vkCmdDecompressMemoryIndirectCountNV)vkGetDeviceProcAddr(device, "vkCmdDecompressMemoryIndirectCountNV");
    vkCmdDecompressMemoryNV = (PFN_vkCmdDecompressMemoryNV)vkGetDeviceProcAddr(device, "vkCmdDecompressMemoryNV");
#endif /* defined(VK_NV_memory_decompression) */
#if defined(VK_NV_mesh_shader)
    vkCmdDrawMeshTasksIndirectCountNV = (PFN_vkCmdDrawMeshTasksIndirectCountNV)vkGetDeviceProcAddr(device, "vkCmdDrawMeshTasksIndirectCountNV");
    vkCmdDrawMeshTasksIndirectNV = (PFN_vkCmdDrawMeshTasksIndirectNV)vkGetDeviceProcAddr(device, "vkCmdDrawMeshTasksIndirectNV");
    vkCmdDrawMeshTasksNV = (PFN_vkCmdDrawMeshTasksNV)vkGetDeviceProcAddr(device, "vkCmdDrawMeshTasksNV");
#endif /* defined(VK_NV_mesh_shader) */
#if defined(VK_NV_optical_flow)
    vkBindOpticalFlowSessionImageNV = (PFN_vkBindOpticalFlowSessionImageNV)vkGetDeviceProcAddr(device, "vkBindOpticalFlowSessionImageNV");
    vkCmdOpticalFlowExecuteNV = (PFN_vkCmdOpticalFlowExecuteNV)vkGetDeviceProcAddr(device, "vkCmdOpticalFlowExecuteNV");
    vkCreateOpticalFlowSessionNV = (PFN_vkCreateOpticalFlowSessionNV)vkGetDeviceProcAddr(device, "vkCreateOpticalFlowSessionNV");
    vkDestroyOpticalFlowSessionNV = (PFN_vkDestroyOpticalFlowSessionNV)vkGetDeviceProcAddr(device, "vkDestroyOpticalFlowSessionNV");
#endif /* defined(VK_NV_optical_flow) */
#if defined(VK_NV_ray_tracing)
    vkBindAccelerationStructureMemoryNV = (PFN_vkBindAccelerationStructureMemoryNV)vkGetDeviceProcAddr(device, "vkBindAccelerationStructureMemoryNV");
    vkCmdBuildAccelerationStructureNV = (PFN_vkCmdBuildAccelerationStructureNV)vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructureNV");
    vkCmdCopyAccelerationStructureNV = (PFN_vkCmdCopyAccelerationStructureNV)vkGetDeviceProcAddr(device, "vkCmdCopyAccelerationStructureNV");
    vkCmdTraceRaysNV = (PFN_vkCmdTraceRaysNV)vkGetDeviceProcAddr(device, "vkCmdTraceRaysNV");
    vkCmdWriteAccelerationStructuresPropertiesNV = (PFN_vkCmdWriteAccelerationStructuresPropertiesNV)vkGetDeviceProcAddr(device, "vkCmdWriteAccelerationStructuresPropertiesNV");
    vkCompileDeferredNV = (PFN_vkCompileDeferredNV)vkGetDeviceProcAddr(device, "vkCompileDeferredNV");
    vkCreateAccelerationStructureNV = (PFN_vkCreateAccelerationStructureNV)vkGetDeviceProcAddr(device, "vkCreateAccelerationStructureNV");
    vkCreateRayTracingPipelinesNV = (PFN_vkCreateRayTracingPipelinesNV)vkGetDeviceProcAddr(device, "vkCreateRayTracingPipelinesNV");
    vkDestroyAccelerationStructureNV = (PFN_vkDestroyAccelerationStructureNV)vkGetDeviceProcAddr(device, "vkDestroyAccelerationStructureNV");
    vkGetAccelerationStructureHandleNV = (PFN_vkGetAccelerationStructureHandleNV)vkGetDeviceProcAddr(device, "vkGetAccelerationStructureHandleNV");
    vkGetAccelerationStructureMemoryRequirementsNV = (PFN_vkGetAccelerationStructureMemoryRequirementsNV)vkGetDeviceProcAddr(device, "vkGetAccelerationStructureMemoryRequirementsNV");
    vkGetRayTracingShaderGroupHandlesNV = (PFN_vkGetRayTracingShaderGroupHandlesNV)vkGetDeviceProcAddr(device, "vkGetRayTracingShaderGroupHandlesNV");
#endif /* defined(VK_NV_ray_tracing) */
#if defined(VK_NV_scissor_exclusive) && VK_NV_SCISSOR_EXCLUSIVE_SPEC_VERSION >= 2
    vkCmdSetExclusiveScissorEnableNV = (PFN_vkCmdSetExclusiveScissorEnableNV)vkGetDeviceProcAddr(device, "vkCmdSetExclusiveScissorEnableNV");
#endif /* defined(VK_NV_scissor_exclusive) && VK_NV_SCISSOR_EXCLUSIVE_SPEC_VERSION >= 2 */
#if defined(VK_NV_scissor_exclusive)
    vkCmdSetExclusiveScissorNV = (PFN_vkCmdSetExclusiveScissorNV)vkGetDeviceProcAddr(device, "vkCmdSetExclusiveScissorNV");
#endif /* defined(VK_NV_scissor_exclusive) */
#if defined(VK_NV_shading_rate_image)
    vkCmdBindShadingRateImageNV = (PFN_vkCmdBindShadingRateImageNV)vkGetDeviceProcAddr(device, "vkCmdBindShadingRateImageNV");
    vkCmdSetCoarseSampleOrderNV = (PFN_vkCmdSetCoarseSampleOrderNV)vkGetDeviceProcAddr(device, "vkCmdSetCoarseSampleOrderNV");
    vkCmdSetViewportShadingRatePaletteNV = (PFN_vkCmdSetViewportShadingRatePaletteNV)vkGetDeviceProcAddr(device, "vkCmdSetViewportShadingRatePaletteNV");
#endif /* defined(VK_NV_shading_rate_image) */
#if defined(VK_QCOM_tile_properties)
    vkGetDynamicRenderingTilePropertiesQCOM = (PFN_vkGetDynamicRenderingTilePropertiesQCOM)vkGetDeviceProcAddr(device, "vkGetDynamicRenderingTilePropertiesQCOM");
    vkGetFramebufferTilePropertiesQCOM = (PFN_vkGetFramebufferTilePropertiesQCOM)vkGetDeviceProcAddr(device, "vkGetFramebufferTilePropertiesQCOM");
#endif /* defined(VK_QCOM_tile_properties) */
#if defined(VK_QNX_external_memory_screen_buffer)
    vkGetScreenBufferPropertiesQNX = (PFN_vkGetScreenBufferPropertiesQNX)vkGetDeviceProcAddr(device, "vkGetScreenBufferPropertiesQNX");
#endif /* defined(VK_QNX_external_memory_screen_buffer) */
#if defined(VK_VALVE_descriptor_set_host_mapping)
    vkGetDescriptorSetHostMappingVALVE = (PFN_vkGetDescriptorSetHostMappingVALVE)vkGetDeviceProcAddr(device, "vkGetDescriptorSetHostMappingVALVE");
    vkGetDescriptorSetLayoutHostMappingInfoVALVE = (PFN_vkGetDescriptorSetLayoutHostMappingInfoVALVE)vkGetDeviceProcAddr(device, "vkGetDescriptorSetLayoutHostMappingInfoVALVE");
#endif /* defined(VK_VALVE_descriptor_set_host_mapping) */
#if (defined(VK_EXT_extended_dynamic_state)) || (defined(VK_EXT_shader_object))
    vkCmdBindVertexBuffers2EXT = (PFN_vkCmdBindVertexBuffers2EXT)vkGetDeviceProcAddr(device, "vkCmdBindVertexBuffers2EXT");
    vkCmdSetCullModeEXT = (PFN_vkCmdSetCullModeEXT)vkGetDeviceProcAddr(device, "vkCmdSetCullModeEXT");
    vkCmdSetDepthBoundsTestEnableEXT = (PFN_vkCmdSetDepthBoundsTestEnableEXT)vkGetDeviceProcAddr(device, "vkCmdSetDepthBoundsTestEnableEXT");
    vkCmdSetDepthCompareOpEXT = (PFN_vkCmdSetDepthCompareOpEXT)vkGetDeviceProcAddr(device, "vkCmdSetDepthCompareOpEXT");
    vkCmdSetDepthTestEnableEXT = (PFN_vkCmdSetDepthTestEnableEXT)vkGetDeviceProcAddr(device, "vkCmdSetDepthTestEnableEXT");
    vkCmdSetDepthWriteEnableEXT = (PFN_vkCmdSetDepthWriteEnableEXT)vkGetDeviceProcAddr(device, "vkCmdSetDepthWriteEnableEXT");
    vkCmdSetFrontFaceEXT = (PFN_vkCmdSetFrontFaceEXT)vkGetDeviceProcAddr(device, "vkCmdSetFrontFaceEXT");
    vkCmdSetPrimitiveTopologyEXT = (PFN_vkCmdSetPrimitiveTopologyEXT)vkGetDeviceProcAddr(device, "vkCmdSetPrimitiveTopologyEXT");
    vkCmdSetScissorWithCountEXT = (PFN_vkCmdSetScissorWithCountEXT)vkGetDeviceProcAddr(device, "vkCmdSetScissorWithCountEXT");
    vkCmdSetStencilOpEXT = (PFN_vkCmdSetStencilOpEXT)vkGetDeviceProcAddr(device, "vkCmdSetStencilOpEXT");
    vkCmdSetStencilTestEnableEXT = (PFN_vkCmdSetStencilTestEnableEXT)vkGetDeviceProcAddr(device, "vkCmdSetStencilTestEnableEXT");
    vkCmdSetViewportWithCountEXT = (PFN_vkCmdSetViewportWithCountEXT)vkGetDeviceProcAddr(device, "vkCmdSetViewportWithCountEXT");
#endif /* (defined(VK_EXT_extended_dynamic_state)) || (defined(VK_EXT_shader_object)) */
#if (defined(VK_EXT_extended_dynamic_state2)) || (defined(VK_EXT_shader_object))
    vkCmdSetDepthBiasEnableEXT = (PFN_vkCmdSetDepthBiasEnableEXT)vkGetDeviceProcAddr(device, "vkCmdSetDepthBiasEnableEXT");
    vkCmdSetLogicOpEXT = (PFN_vkCmdSetLogicOpEXT)vkGetDeviceProcAddr(device, "vkCmdSetLogicOpEXT");
    vkCmdSetPatchControlPointsEXT = (PFN_vkCmdSetPatchControlPointsEXT)vkGetDeviceProcAddr(device, "vkCmdSetPatchControlPointsEXT");
    vkCmdSetPrimitiveRestartEnableEXT = (PFN_vkCmdSetPrimitiveRestartEnableEXT)vkGetDeviceProcAddr(device, "vkCmdSetPrimitiveRestartEnableEXT");
    vkCmdSetRasterizerDiscardEnableEXT = (PFN_vkCmdSetRasterizerDiscardEnableEXT)vkGetDeviceProcAddr(device, "vkCmdSetRasterizerDiscardEnableEXT");
#endif /* (defined(VK_EXT_extended_dynamic_state2)) || (defined(VK_EXT_shader_object)) */
#if (defined(VK_EXT_extended_dynamic_state3)) || (defined(VK_EXT_shader_object))
    vkCmdSetAlphaToCoverageEnableEXT = (PFN_vkCmdSetAlphaToCoverageEnableEXT)vkGetDeviceProcAddr(device, "vkCmdSetAlphaToCoverageEnableEXT");
    vkCmdSetAlphaToOneEnableEXT = (PFN_vkCmdSetAlphaToOneEnableEXT)vkGetDeviceProcAddr(device, "vkCmdSetAlphaToOneEnableEXT");
    vkCmdSetColorBlendAdvancedEXT = (PFN_vkCmdSetColorBlendAdvancedEXT)vkGetDeviceProcAddr(device, "vkCmdSetColorBlendAdvancedEXT");
    vkCmdSetColorBlendEnableEXT = (PFN_vkCmdSetColorBlendEnableEXT)vkGetDeviceProcAddr(device, "vkCmdSetColorBlendEnableEXT");
    vkCmdSetColorBlendEquationEXT = (PFN_vkCmdSetColorBlendEquationEXT)vkGetDeviceProcAddr(device, "vkCmdSetColorBlendEquationEXT");
    vkCmdSetColorWriteMaskEXT = (PFN_vkCmdSetColorWriteMaskEXT)vkGetDeviceProcAddr(device, "vkCmdSetColorWriteMaskEXT");
    vkCmdSetConservativeRasterizationModeEXT = (PFN_vkCmdSetConservativeRasterizationModeEXT)vkGetDeviceProcAddr(device, "vkCmdSetConservativeRasterizationModeEXT");
    vkCmdSetDepthClampEnableEXT = (PFN_vkCmdSetDepthClampEnableEXT)vkGetDeviceProcAddr(device, "vkCmdSetDepthClampEnableEXT");
    vkCmdSetDepthClipEnableEXT = (PFN_vkCmdSetDepthClipEnableEXT)vkGetDeviceProcAddr(device, "vkCmdSetDepthClipEnableEXT");
    vkCmdSetDepthClipNegativeOneToOneEXT = (PFN_vkCmdSetDepthClipNegativeOneToOneEXT)vkGetDeviceProcAddr(device, "vkCmdSetDepthClipNegativeOneToOneEXT");
    vkCmdSetExtraPrimitiveOverestimationSizeEXT = (PFN_vkCmdSetExtraPrimitiveOverestimationSizeEXT)vkGetDeviceProcAddr(device, "vkCmdSetExtraPrimitiveOverestimationSizeEXT");
    vkCmdSetLineRasterizationModeEXT = (PFN_vkCmdSetLineRasterizationModeEXT)vkGetDeviceProcAddr(device, "vkCmdSetLineRasterizationModeEXT");
    vkCmdSetLineStippleEnableEXT = (PFN_vkCmdSetLineStippleEnableEXT)vkGetDeviceProcAddr(device, "vkCmdSetLineStippleEnableEXT");
    vkCmdSetLogicOpEnableEXT = (PFN_vkCmdSetLogicOpEnableEXT)vkGetDeviceProcAddr(device, "vkCmdSetLogicOpEnableEXT");
    vkCmdSetPolygonModeEXT = (PFN_vkCmdSetPolygonModeEXT)vkGetDeviceProcAddr(device, "vkCmdSetPolygonModeEXT");
    vkCmdSetProvokingVertexModeEXT = (PFN_vkCmdSetProvokingVertexModeEXT)vkGetDeviceProcAddr(device, "vkCmdSetProvokingVertexModeEXT");
    vkCmdSetRasterizationSamplesEXT = (PFN_vkCmdSetRasterizationSamplesEXT)vkGetDeviceProcAddr(device, "vkCmdSetRasterizationSamplesEXT");
    vkCmdSetRasterizationStreamEXT = (PFN_vkCmdSetRasterizationStreamEXT)vkGetDeviceProcAddr(device, "vkCmdSetRasterizationStreamEXT");
    vkCmdSetSampleLocationsEnableEXT = (PFN_vkCmdSetSampleLocationsEnableEXT)vkGetDeviceProcAddr(device, "vkCmdSetSampleLocationsEnableEXT");
    vkCmdSetSampleMaskEXT = (PFN_vkCmdSetSampleMaskEXT)vkGetDeviceProcAddr(device, "vkCmdSetSampleMaskEXT");
    vkCmdSetTessellationDomainOriginEXT = (PFN_vkCmdSetTessellationDomainOriginEXT)vkGetDeviceProcAddr(device, "vkCmdSetTessellationDomainOriginEXT");
#endif /* (defined(VK_EXT_extended_dynamic_state3)) || (defined(VK_EXT_shader_object)) */
#if (defined(VK_EXT_extended_dynamic_state3) && defined(VK_NV_clip_space_w_scaling)) || (defined(VK_EXT_shader_object) && defined(VK_NV_clip_space_w_scaling))
    vkCmdSetViewportWScalingEnableNV = (PFN_vkCmdSetViewportWScalingEnableNV)vkGetDeviceProcAddr(device, "vkCmdSetViewportWScalingEnableNV");
#endif /* (defined(VK_EXT_extended_dynamic_state3) && defined(VK_NV_clip_space_w_scaling)) || (defined(VK_EXT_shader_object) && defined(VK_NV_clip_space_w_scaling)) */
#if (defined(VK_EXT_extended_dynamic_state3) && defined(VK_NV_viewport_swizzle)) || (defined(VK_EXT_shader_object) && defined(VK_NV_viewport_swizzle))
    vkCmdSetViewportSwizzleNV = (PFN_vkCmdSetViewportSwizzleNV)vkGetDeviceProcAddr(device, "vkCmdSetViewportSwizzleNV");
#endif /* (defined(VK_EXT_extended_dynamic_state3) && defined(VK_NV_viewport_swizzle)) || (defined(VK_EXT_shader_object) && defined(VK_NV_viewport_swizzle)) */
#if (defined(VK_EXT_extended_dynamic_state3) && defined(VK_NV_fragment_coverage_to_color)) || (defined(VK_EXT_shader_object) && defined(VK_NV_fragment_coverage_to_color))
    vkCmdSetCoverageToColorEnableNV = (PFN_vkCmdSetCoverageToColorEnableNV)vkGetDeviceProcAddr(device, "vkCmdSetCoverageToColorEnableNV");
    vkCmdSetCoverageToColorLocationNV = (PFN_vkCmdSetCoverageToColorLocationNV)vkGetDeviceProcAddr(device, "vkCmdSetCoverageToColorLocationNV");
#endif /* (defined(VK_EXT_extended_dynamic_state3) && defined(VK_NV_fragment_coverage_to_color)) || (defined(VK_EXT_shader_object) && defined(VK_NV_fragment_coverage_to_color)) */
#if (defined(VK_EXT_extended_dynamic_state3) && defined(VK_NV_framebuffer_mixed_samples)) || (defined(VK_EXT_shader_object) && defined(VK_NV_framebuffer_mixed_samples))
    vkCmdSetCoverageModulationModeNV = (PFN_vkCmdSetCoverageModulationModeNV)vkGetDeviceProcAddr(device, "vkCmdSetCoverageModulationModeNV");
    vkCmdSetCoverageModulationTableEnableNV = (PFN_vkCmdSetCoverageModulationTableEnableNV)vkGetDeviceProcAddr(device, "vkCmdSetCoverageModulationTableEnableNV");
    vkCmdSetCoverageModulationTableNV = (PFN_vkCmdSetCoverageModulationTableNV)vkGetDeviceProcAddr(device, "vkCmdSetCoverageModulationTableNV");
#endif /* (defined(VK_EXT_extended_dynamic_state3) && defined(VK_NV_framebuffer_mixed_samples)) || (defined(VK_EXT_shader_object) && defined(VK_NV_framebuffer_mixed_samples)) */
#if (defined(VK_EXT_extended_dynamic_state3) && defined(VK_NV_shading_rate_image)) || (defined(VK_EXT_shader_object) && defined(VK_NV_shading_rate_image))
    vkCmdSetShadingRateImageEnableNV = (PFN_vkCmdSetShadingRateImageEnableNV)vkGetDeviceProcAddr(device, "vkCmdSetShadingRateImageEnableNV");
#endif /* (defined(VK_EXT_extended_dynamic_state3) && defined(VK_NV_shading_rate_image)) || (defined(VK_EXT_shader_object) && defined(VK_NV_shading_rate_image)) */
#if (defined(VK_EXT_extended_dynamic_state3) && defined(VK_NV_representative_fragment_test)) || (defined(VK_EXT_shader_object) && defined(VK_NV_representative_fragment_test))
    vkCmdSetRepresentativeFragmentTestEnableNV = (PFN_vkCmdSetRepresentativeFragmentTestEnableNV)vkGetDeviceProcAddr(device, "vkCmdSetRepresentativeFragmentTestEnableNV");
#endif /* (defined(VK_EXT_extended_dynamic_state3) && defined(VK_NV_representative_fragment_test)) || (defined(VK_EXT_shader_object) && defined(VK_NV_representative_fragment_test)) */
#if (defined(VK_EXT_extended_dynamic_state3) && defined(VK_NV_coverage_reduction_mode)) || (defined(VK_EXT_shader_object) && defined(VK_NV_coverage_reduction_mode))
    vkCmdSetCoverageReductionModeNV = (PFN_vkCmdSetCoverageReductionModeNV)vkGetDeviceProcAddr(device, "vkCmdSetCoverageReductionModeNV");
#endif /* (defined(VK_EXT_extended_dynamic_state3) && defined(VK_NV_coverage_reduction_mode)) || (defined(VK_EXT_shader_object) && defined(VK_NV_coverage_reduction_mode)) */
#if (defined(VK_EXT_full_screen_exclusive) && defined(VK_KHR_device_group)) || (defined(VK_EXT_full_screen_exclusive) && defined(VK_VERSION_1_1))
    vkGetDeviceGroupSurfacePresentModes2EXT = (PFN_vkGetDeviceGroupSurfacePresentModes2EXT)vkGetDeviceProcAddr(device, "vkGetDeviceGroupSurfacePresentModes2EXT");
#endif /* (defined(VK_EXT_full_screen_exclusive) && defined(VK_KHR_device_group)) || (defined(VK_EXT_full_screen_exclusive) && defined(VK_VERSION_1_1)) */
#if (defined(VK_EXT_host_image_copy)) || (defined(VK_EXT_image_compression_control))
    vkGetImageSubresourceLayout2EXT = (PFN_vkGetImageSubresourceLayout2EXT)vkGetDeviceProcAddr(device, "vkGetImageSubresourceLayout2EXT");
#endif /* (defined(VK_EXT_host_image_copy)) || (defined(VK_EXT_image_compression_control)) */
#if (defined(VK_EXT_shader_object)) || (defined(VK_EXT_vertex_input_dynamic_state))
    vkCmdSetVertexInputEXT = (PFN_vkCmdSetVertexInputEXT)vkGetDeviceProcAddr(device, "vkCmdSetVertexInputEXT");
#endif /* (defined(VK_EXT_shader_object)) || (defined(VK_EXT_vertex_input_dynamic_state)) */
#if (defined(VK_KHR_descriptor_update_template) && defined(VK_KHR_push_descriptor)) || (defined(VK_KHR_push_descriptor) && defined(VK_VERSION_1_1)) || (defined(VK_KHR_push_descriptor) && defined(VK_KHR_descriptor_update_template))
    vkCmdPushDescriptorSetWithTemplateKHR = (PFN_vkCmdPushDescriptorSetWithTemplateKHR)vkGetDeviceProcAddr(device, "vkCmdPushDescriptorSetWithTemplateKHR");
#endif /* (defined(VK_KHR_descriptor_update_template) && defined(VK_KHR_push_descriptor)) || (defined(VK_KHR_push_descriptor) && defined(VK_VERSION_1_1)) || (defined(VK_KHR_push_descriptor) && defined(VK_KHR_descriptor_update_template)) */
#if (defined(VK_KHR_device_group) && defined(VK_KHR_surface)) || (defined(VK_KHR_swapchain) && defined(VK_VERSION_1_1))
    vkGetDeviceGroupPresentCapabilitiesKHR = (PFN_vkGetDeviceGroupPresentCapabilitiesKHR)vkGetDeviceProcAddr(device, "vkGetDeviceGroupPresentCapabilitiesKHR");
    vkGetDeviceGroupSurfacePresentModesKHR = (PFN_vkGetDeviceGroupSurfacePresentModesKHR)vkGetDeviceProcAddr(device, "vkGetDeviceGroupSurfacePresentModesKHR");
#endif /* (defined(VK_KHR_device_group) && defined(VK_KHR_surface)) || (defined(VK_KHR_swapchain) && defined(VK_VERSION_1_1)) */
#if (defined(VK_KHR_device_group) && defined(VK_KHR_swapchain)) || (defined(VK_KHR_swapchain) && defined(VK_VERSION_1_1))
    vkAcquireNextImage2KHR = (PFN_vkAcquireNextImage2KHR)vkGetDeviceProcAddr(device, "vkAcquireNextImage2KHR");
#endif /* (defined(VK_KHR_device_group) && defined(VK_KHR_swapchain)) || (defined(VK_KHR_swapchain) && defined(VK_VERSION_1_1)) */
}