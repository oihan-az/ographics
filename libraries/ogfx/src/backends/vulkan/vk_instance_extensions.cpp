
#include "vk_instance_extensions.hpp"

#include <vulkan/vulkan.h>

#if defined(VK_USE_PLATFORM_WIN32_KHR)
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#endif

#if defined(VK_USE_PLATFORM_XLIB_KHR)
#include <X11/Xlib.h>
#include <vulkan/vulkan_xlib.h>
#endif

#if defined(VK_USE_PLATFORM_XCB_KHR)
#include <vulkan/vulkan_xcb.h>
#include <xcb/xcb.h>
#endif

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
#include <vulkan/vulkan_wayland.h>
#include <wayland-client.h>
#endif

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
#include <vulkan/vulkan_android.h>
#endif

#if defined(VK_USE_PLATFORM_METAL_EXT)
#include <vulkan/vulkan_metal.h>
#endif

namespace ogfx
{

std::vector<const char*> to_vk_instance_extensions(InstanceExtension extensions)
{
    std::vector<const char*> result;

    // Surface
    if (has_flag(extensions, InstanceExtension::Surface))
        result.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

if (has_flag(extensions, InstanceExtension::Surface))
        result.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (has_flag(extensions, InstanceExtension::Win32Surface))
        result.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif

#if defined(VK_USE_PLATFORM_XLIB_KHR)
    if (has_flag(extensions, InstanceExtension::XlibSurface))
        result.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif

#if defined(VK_USE_PLATFORM_XCB_KHR)
    if (has_flag(extensions, InstanceExtension::XcbSurface))
        result.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    if (has_flag(extensions, InstanceExtension::WaylandSurface))
        result.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#endif

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    if (has_flag(extensions, InstanceExtension::AndroidSurface))
        result.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#endif

#if defined(VK_USE_PLATFORM_METAL_EXT)
    if (has_flag(extensions, InstanceExtension::MetalSurface))
        result.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
#endif

    if (has_flag(extensions, InstanceExtension::HeadlessSurface))
        result.push_back(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);

    // Debugging and validation
    if (has_flag(extensions, InstanceExtension::DebugUtils))
        result.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    if (has_flag(extensions, InstanceExtension::DebugReport))
        result.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

    if (has_flag(extensions, InstanceExtension::ValidationFeatures))
        result.push_back(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);

    if (has_flag(extensions, InstanceExtension::ValidationFlags))
        result.push_back(VK_EXT_VALIDATION_FLAGS_EXTENSION_NAME);

    if (has_flag(extensions, InstanceExtension::LayerSettings))
        result.push_back(VK_EXT_LAYER_SETTINGS_EXTENSION_NAME);

    // Physical device and device discovery
    if (has_flag(extensions, InstanceExtension::GetPhysicalDeviceProperties2))
        result.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

    if (has_flag(extensions, InstanceExtension::DeviceGroupCreation))
        result.push_back(VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME);

    if (has_flag(extensions, InstanceExtension::ToolingInfo))
        result.push_back(VK_EXT_TOOLING_INFO_EXTENSION_NAME);

    // External resource capabilities
    if (has_flag(extensions, InstanceExtension::ExternalMemoryCapabilities))
        result.push_back(VK_KHR_EXTERNAL_MEMORY_CAPABILITIES_EXTENSION_NAME);

    if (has_flag(extensions, InstanceExtension::ExternalSemaphoreCapabilities))
        result.push_back(VK_KHR_EXTERNAL_SEMAPHORE_CAPABILITIES_EXTENSION_NAME);

    if (has_flag(extensions, InstanceExtension::ExternalFenceCapabilities))
        result.push_back(VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME);

    // Surface capabilities
    if (has_flag(extensions, InstanceExtension::GetSurfaceCapabilities2))
        result.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);

    if (has_flag(extensions, InstanceExtension::SurfaceProtectedCapabilities))
        result.push_back(VK_KHR_SURFACE_PROTECTED_CAPABILITIES_EXTENSION_NAME);

    if (has_flag(extensions, InstanceExtension::SurfaceMaintenance1))
        result.push_back(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME);

    if (has_flag(extensions, InstanceExtension::SwapchainColorspace))
        result.push_back(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME);

    // Display
    if (has_flag(extensions, InstanceExtension::Display))
        result.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);

    if (has_flag(extensions, InstanceExtension::GetDisplayProperties2))
        result.push_back(VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME);

    if (has_flag(extensions, InstanceExtension::DirectModeDisplay))
        result.push_back(VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME);

    if (has_flag(extensions, InstanceExtension::AcquireDrmDisplay))
        result.push_back(VK_EXT_ACQUIRE_DRM_DISPLAY_EXTENSION_NAME);

    // Portability
    if (has_flag(extensions, InstanceExtension::PortabilityEnumeration))
        result.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

    return result;
}

} // namespace ogfx