#include <debug.hpp>
#include <ogfx/enum_string.hpp>

#include "vk_device.hpp"
#include "vk_resource_set_layout.hpp"
#include "vk_resource_type.hpp"
#include "vk_shader_stage.hpp"

namespace ogfx
{

ResourceSetLayout::ResourceSetLayout(const Device& device, const ResourceSetLayoutDesc& desc) 
    : m_desc(desc), m_impl(std::make_unique<Impl>())
{
    m_impl->m_device = device.m_impl->m_device;

    OGFX_LOG("Creating resource set layout:");

    std::vector<VkDescriptorSetLayoutBinding> bindings;
    bindings.reserve(desc.bindings.size());

    for (const ResourceBindingDesc& binding_desc : desc.bindings)
    {
        OGFX_LOG("  Binding: " + std::to_string(binding_desc.binding));
        OGFX_LOG("    Type: " + to_string_enum(binding_desc.type));
        OGFX_LOG("    Array count: " + std::to_string(binding_desc.array_count));
        OGFX_LOG("    Stages: " + to_string_bitmask(binding_desc.stages));

        VkDescriptorSetLayoutBinding binding{};
        binding.binding = binding_desc.binding;
        binding.descriptorType = to_vk_descriptor_type(binding_desc.type);
        binding.descriptorCount = binding_desc.array_count;
        binding.stageFlags = to_vk_shader_stage_flags(binding_desc.stages);
        binding.pImmutableSamplers = nullptr;

        bindings.push_back(binding);
    }

    VkDescriptorSetLayoutCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    create_info.bindingCount = static_cast<uint32_t>(bindings.size());
    create_info.pBindings = bindings.empty() ? nullptr : bindings.data();

    if (vkCreateDescriptorSetLayout(m_impl->m_device, &create_info, nullptr, &m_impl->m_descriptor_set_layout) != VK_SUCCESS)
    {
        OGFX_LOG_ERROR("Failed to create resource set layout");
        throw std::runtime_error("Failed to create resource set layout");
    }

    OGFX_LOG("Resource set layout created");
}

ResourceSetLayout::~ResourceSetLayout()
{
    if (m_impl && m_impl->m_descriptor_set_layout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(m_impl->m_device, m_impl->m_descriptor_set_layout, nullptr);

        m_impl->m_descriptor_set_layout = VK_NULL_HANDLE;

        OGFX_LOG("Resource set layout destroyed");
    }
}

ResourceSetLayout::ResourceSetLayout(ResourceSetLayout&&) noexcept = default;

ResourceSetLayout& ResourceSetLayout::operator=(ResourceSetLayout&&) noexcept = default;

} // namespace ogfx
