#ifndef OGFX_GLFW_WINDOW_HPP
#define OGFX_GLFW_WINDOW_HPP

struct GLFWwindow;

namespace oplatform
{

struct Window::Impl
{
    GLFWwindow* m_handle = nullptr;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
};

} // namespace oplatform

#endif // OGFX_GLFW_WINDOW_HPP