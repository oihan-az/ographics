/**
 * @file window.cpp
 * @brief GLFW-based window implementation.
 */

#include <oplatform/window.hpp>

#include <debug.hpp>

#include "glfw_window.hpp"

#include <GLFW/glfw3.h>

#include <stdexcept>

namespace oplatform
{

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

    OPLATFORM_LOG("Creating GLFW window");

    // Create the GLFW window
    m_impl->m_handle = glfwCreateWindow(static_cast<int>(desc.width), static_cast<int>(desc.height), desc.title.c_str(),
                                      monitor, nullptr);

    // Check window creation
    if (!m_impl->m_handle)
    {
        OPLATFORM_LOG_ERROR("Failed to create GLFW window");
        throw std::runtime_error("Failed to create window.");
    }

    OPLATFORM_LOG("GLFW window created");

    // Save window data
    m_impl->m_width = desc.width;
    m_impl->m_height = desc.height;
}

Window::~Window()
{
    if (m_impl && m_impl->m_handle)
    {
        // Destroy the window
        glfwDestroyWindow(m_impl->m_handle);

        OPLATFORM_LOG("GLFW window destroyed");
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
    glfwGetWindowSize(m_impl->m_handle, &width, &height);
    m_impl->m_width = static_cast<uint32_t>(width);
    m_impl->m_height = static_cast<uint32_t>(height);
}

bool Window::should_close() const
{
    return glfwWindowShouldClose(m_impl->m_handle);
}

uint32_t Window::width() const
{
    return m_impl->m_width;
}

uint32_t Window::height() const
{
    return m_impl->m_height;
}

} // namespace oplatform