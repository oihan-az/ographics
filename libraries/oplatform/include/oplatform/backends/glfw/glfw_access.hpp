#ifndef OPLATFORM_GLFW_ACCESS_HPP
#define OPLATFORM_GLFW_ACCESS_HPP

#include <oplatform/window.hpp>

#include <GLFW/glfw3.h>

namespace oplatform::glfw
{

class GLFWAccess
{
  public:
    static GLFWwindow* window_handle(const Window& window);
};

} // namespace oplatform::glfw

#endif // OPLATFORM_GLFW_ACCESS_HPP