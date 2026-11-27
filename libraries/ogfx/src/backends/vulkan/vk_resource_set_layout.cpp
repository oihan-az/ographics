#include <debug.hpp>
#include <ogfx/enum_string.hpp>

#include "vk_device.hpp"
#include "vk_resource_set_layout.hpp"
#include "vk_resource_type.hpp"
#include "vk_shader_stage.hpp"

namespace ogfx
{

VkDescriptorBindingFlags to_vk_resource_binding_flags(ResourceBindingFlags flags)
{
    VkDescriptorBindingFlags result = 0;

    if (has_flag(flags, ResourceBindingFlags::PartiallyBound))
    {
        result |= VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
    }

    if (has_flag(flags, ResourceBindingFlags::VariableDescriptorCount))
    {
        result |= VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;
    }

    return result;
}

ResourceBindingFlags from_vk_resource_binding_flags(VkDescriptorBindingFlags flags)
{
    ResourceBindingFlags result = ResourceBindingFlags::None;

    if (flags & VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT)
    {
        result = result | ResourceBindingFlags::PartiallyBound;
    }

    if (flags & VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT)
    {
        result = result | ResourceBindingFlags::VariableDescriptorCount;
    }

    return result;
}

ResourceSetLayout::ResourceSetLayout(const Device& device, const ResourceSetLayoutDesc& desc) 
    : m_desc(desc), m_impl(std::make_unique<Impl>())
{
    m_impl->m_device = device.m_impl->m_device;

    OGFX_LOG("Creating resource set layout:");

    std::vector<VkDescriptorSetLayoutBinding> bindings;
    std::vector<VkDescriptorBindingFlags> binding_flags;

    bindings.reserve(desc.bindings.size());
    binding_flags.reserve(desc.bindings.size());

    for (const ResourceBindingDesc& binding_desc : desc.bindings)
    {
        OGFX_LOG("  Binding: " + std::to_string(binding_desc.binding));
        OGFX_LOG("    Type: " + to_string_enum(binding_desc.type));
        OGFX_LOG("    Array count: " + std::to_string(binding_desc.array_count));
        OGFX_LOG("    Stages: " + to_string_bitmask(binding_desc.stages));
        OGFX_LOG("    Flags: " + to_string_bitmask(binding_desc.flags));

        VkDescriptorSetLayoutBinding binding{};
        binding.binding = binding_desc.binding;
        binding.descriptorType = to_vk_descriptor_type(binding_desc.type);
        binding.descriptorCount = binding_desc.array_count;
        binding.stageFlags = to_vk_shader_stage_flags(binding_desc.stages);
        binding.pImmutableSamplers = nullptr;

        bindings.push_back(binding);

        binding_flags.push_back(to_vk_resource_binding_flags(binding_desc.flags));
    }

    VkDescriptorSetLayoutBindingFlagsCreateInfo binding_flags_info{};
    binding_flags_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    binding_flags_info.bindingCount = static_cast<uint32_t>(binding_flags.size());
    binding_flags_info.pBindingFlags = binding_flags.empty() ? nullptr : binding_flags.data();

    VkDescriptorSetLayoutCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    create_info.bindingCount = static_cast<uint32_t>(bindings.size());
    create_info.pBindings = bindings.empty() ? nullptr : bindings.data();
    create_info.pNext = &binding_flags_info;

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
