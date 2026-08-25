/**
 * @file window.cpp
 * @brief GLFW-based window implementation.
 */

#include <oplatform/window.hpp>

#include <GLFW/glfw3.h>

#include <stdexcept>

namespace oplatform
{

struct Window::Impl
{
    GLFWwindow* handle = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
};

Window::Window(const WindowDesc& desc) : m_impl(std::make_unique<Impl>())
{
    // Don't create OpenGL context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, desc.resizable ? GLFW_TRUE : GLFW_FALSE);

    GLFWmonitor* monitor = nullptr;
    if (desc.fullscreen)
    {
        monitor = glfwGetPrimaryMonitor();
    }

    // Create the GLFW window
    m_impl->handle = glfwCreateWindow(static_cast<int>(desc.width), static_cast<int>(desc.height), desc.title.c_str(),
                                      monitor, nullptr);

    // Check window creation
    if (!m_impl->handle)
    {
        throw std::runtime_error("Failed to create window.");
    }

    // Save window data
    m_impl->width = desc.width;
    m_impl->height = desc.height;
}

Window::~Window()
{
    if (m_impl && m_impl->handle)
    {
        // Destroy the window
        glfwDestroyWindow(m_impl->handle);
    }
}

Window::Window(Window&& other) noexcept = default;

Window& Window::operator=(Window&& other) noexcept = default;

void Window::poll_events()
{
    glfwPollEvents();

    // Update the window size
    int width = 0;
    int height = 0;
    glfwGetWindowSize(m_impl->handle, &width, &height);
    m_impl->width = static_cast<uint32_t>(width);
    m_impl->height = static_cast<uint32_t>(height);
}

bool Window::should_close() const
{
    return glfwWindowShouldClose(m_impl->handle);
}

uint32_t Window::width() const
{
    return m_impl->width;
}

uint32_t Window::height() const
{
    return m_impl->height;
}

} // namespace oplatform