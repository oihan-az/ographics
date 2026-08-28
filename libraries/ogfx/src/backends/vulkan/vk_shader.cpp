#include <debug.hpp>

#include "vk_shader.hpp"
#include "vk_device.hpp"

namespace ogfx
{

Shader::Shader(const Device& device, const ShaderDesc& desc) : m_stage(desc.stage), m_impl(std::make_unique<Impl>())
{
    m_impl->m_device = device.m_impl->m_device;

    OGFX_LOG(std::string("Creating Vulkan ") + to_string(desc.stage) + " shader");

    if (desc.bytecode.empty())
    {
        OGFX_LOG_ERROR("Cannot create Vulkan shader from empty bytecode");
        throw std::runtime_error("Cannot create Vulkan shader from empty bytecode.");
    }

    if (desc.bytecode.size() % sizeof(uint32_t) != 0)
    {
        OGFX_LOG_ERROR("Invalid Vulkan shader bytecode size");
        throw std::runtime_error("Invalid Vulkan shader bytecode size.");
    }

    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = desc.bytecode.size();
    create_info.pCode = reinterpret_cast<const uint32_t*>(desc.bytecode.data());

    if (vkCreateShaderModule(m_impl->m_device, &create_info, nullptr, &m_impl->m_shader_module) != VK_SUCCESS)
    {
        OGFX_LOG_ERROR(std::string("Failed to create Vulkan ") + to_string(desc.stage) + " shader");

        throw std::runtime_error("Failed to create Vulkan shader.");
    }

    OGFX_LOG(std::string("Vulkan ") + to_string(desc.stage) + " shader created");
}

Shader::~Shader()
{
    if (m_impl && m_impl->m_shader_module != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(m_impl->m_device, m_impl->m_shader_module, nullptr);

        OGFX_LOG(std::string("Vulkan ") + to_string(m_stage) + " shader destroyed");
    }
}

Shader::Shader(Shader&&) noexcept = default;

Shader& Shader::operator=(Shader&&) noexcept = default;

} // namespace ogfx