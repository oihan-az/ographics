/**
 * @file runtime.cpp
 * @brief Platform runtime implementation.
 */

#include <oplatform/runtime.hpp>

#include <GLFW/glfw3.h>

#include <stdexcept>

namespace oplatform
{

oplatform::Runtime::Runtime() 
{
    if (!glfwInit())
    {
        throw std::runtime_error("Failed to initialize GLFW.");
    }
}

Runtime::~Runtime()
{
    glfwTerminate();
}

} // namespace oplatform