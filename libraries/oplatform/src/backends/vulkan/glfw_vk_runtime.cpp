#include <oplatform/runtime.hpp>

#include <debug.hpp>

#include <GLFW/glfw3.h>

#include <stdexcept>

namespace oplatform
{

ogfx::InstanceExtension Runtime::required_instance_extensions() const
{
    uint32_t count = 0;
    const char** extensions = glfwGetRequiredInstanceExtensions(&count);

    if (!extensions)
    {
        OPLATFORM_LOG_ERROR("GLFW could not provide required Vulkan instance extensions");
        throw std::runtime_error("GLFW could not provide required Vulkan instance extensions.");
    }

    ogfx::InstanceExtension result = ogfx::InstanceExtension::None;

    OPLATFORM_LOG("Required Vulkan instance extensions:");

    for (uint32_t i = 0; i < count; ++i)
    {
        OPLATFORM_LOG("  " + std::string(extensions[i]));

        if (std::strcmp(extensions[i], "VK_KHR_surface") == 0)
            result |= ogfx::InstanceExtension::Surface;
        else if (std::strcmp(extensions[i], "VK_KHR_win32_surface") == 0)
            result |= ogfx::InstanceExtension::Win32Surface;
        else if (std::strcmp(extensions[i], "VK_KHR_xlib_surface") == 0)
            result |= ogfx::InstanceExtension::XlibSurface;
        else if (std::strcmp(extensions[i], "VK_KHR_xcb_surface") == 0)
            result |= ogfx::InstanceExtension::XcbSurface;
        else if (std::strcmp(extensions[i], "VK_KHR_wayland_surface") == 0)
            result |= ogfx::InstanceExtension::WaylandSurface;
        else if (std::strcmp(extensions[i], "VK_EXT_metal_surface") == 0)
            result |= ogfx::InstanceExtension::MetalSurface;
        else
        {
            OPLATFORM_LOG_ERROR("Unrecognized Vulkan instance extension required by GLFW: " +
                                std::string(extensions[i]));
            throw std::runtime_error("Unrecognized GLFW Vulkan instance extension.");
        }
    }

    return result;
}

} // namespace oplatform