#include <oplatform/backends/glfw/glfw_access.hpp>

#include "glfw_window.hpp"

namespace oplatform::glfw
{

GLFWwindow* GLFWAccess::window_handle(const Window& window)
{
    return window.m_impl->m_handle;
}

} // namespace oplatform::glfw