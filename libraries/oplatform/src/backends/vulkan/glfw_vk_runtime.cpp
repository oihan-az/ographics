#include <oplatform/runtime.hpp>

#include <debug.hpp>

#include <GLFW/glfw3.h>

#include <stdexcept>

namespace oplatform
{

std::vector<const char*> Runtime::required_instance_extensions() const 
{
    uint32_t count = 0;
    const char** extensions = glfwGetRequiredInstanceExtensions(&count);

    if (!extensions)
    {
        OPLATFORM_LOG_ERROR("GLFW could not provide required Vulkan instance extensions");
        throw std::runtime_error("GLFW could not provide required Vulkan instance extensions.");
    }

    OPLATFORM_LOG("Required Vulkan instance extensions:");

    for (uint32_t i = 0; i < count; ++i)
    {
        OPLATFORM_LOG("  " + std::string(extensions[i]));
    }

    return {extensions, extensions + count};
}

} // namespace oplatform