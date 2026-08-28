#ifndef OGFX_VK_SHADER_HPP
#define OGFX_VK_SHADER_HPP

#include <ogfx/shader.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

struct Shader::Impl
{
    VkShaderModule m_shader_module = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
};

} // namespace ogfx

#endif // OGFX_VK_SHADER_HPP