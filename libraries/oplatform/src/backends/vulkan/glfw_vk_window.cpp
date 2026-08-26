#include <oplatform/window.hpp>

#include <debug.hpp>

#include <glfw_window.hpp>

#include <ogfx/instance.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <stdexcept>

namespace oplatform
{

    [[nodiscard]] ::ogfx::Surface Window::create_surface(const ::ogfx::Instance& instance) const 
    {
        VkSurfaceKHR vk_surface = VK_NULL_HANDLE;
        VkInstance vk_instance = reinterpret_cast<VkInstance>(instance.native_handle());

        OPLATFORM_LOG("Creating Vulkan surface");

        if (glfwCreateWindowSurface(vk_instance, m_impl->m_handle, nullptr, &vk_surface) != VK_SUCCESS)
        {
            OPLATFORM_LOG_ERROR("Failed to create Vulkan surface");
            throw std::runtime_error("Failed to create Vulkan surface.");
        }

        OPLATFORM_LOG("Vulkan surface created");

        return ogfx::Surface(instance, vk_surface);
    }

}
