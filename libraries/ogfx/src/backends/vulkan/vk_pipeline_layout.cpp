#include <debug.hpp>
#include <ogfx/enum_string.hpp>

#include "vk_pipeline_layout.hpp"
#include "vk_device.hpp"
#include "vk_resource_set_layout.hpp"
#include "vk_shader_stage.hpp"

#include <string>
#include <vector>

namespace ogfx
{

PipelineLayout::PipelineLayout(const Device& device, const PipelineLayoutDesc& desc) : m_impl(std::make_unique<Impl>())
{
    m_impl->m_device = device.m_impl->m_device;

    OGFX_LOG("Creating pipeline layout:");
    OGFX_LOG("  Set layout count: " + std::to_string(desc.set_layouts.size()));
    OGFX_LOG("  Inline constant range count: " + std::to_string(desc.inline_constant_ranges.size()));

    // Descriptor sets
    std::vector<VkDescriptorSetLayout> set_layouts;
    set_layouts.reserve(desc.set_layouts.size());

    for (size_t i = 0; i < desc.set_layouts.size(); ++i)
    {
        const ResourceSetLayout* layout = desc.set_layouts[i];

        set_layouts.push_back(layout->m_impl->m_descriptor_set_layout);

        OGFX_LOG("  Set layout " + std::to_string(i) + ":");
        OGFX_LOG("    Binding count: " + std::to_string(layout->m_desc.bindings.size()));

        for (const ResourceBindingDesc& binding : layout->m_desc.bindings)
        {
            OGFX_LOG("    Binding " + std::to_string(binding.binding) + ":");
            OGFX_LOG("      Type: " + to_string_enum(binding.type));
            OGFX_LOG("      Array count: " + std::to_string(binding.array_count));
            OGFX_LOG("      Stages: " + to_string_bitmask(binding.stages));
        }
    }

    // Push constants
    std::vector<VkPushConstantRange> inline_constant_ranges;
    inline_constant_ranges.reserve(desc.inline_constant_ranges.size());

    for (size_t i = 0; i < desc.inline_constant_ranges.size(); ++i)
    {
        const InlineConstantRange& range = desc.inline_constant_ranges[i];

        VkPushConstantRange vk_range{};
        vk_range.stageFlags = to_vk_shader_stage_flags(range.stages);
        vk_range.offset = range.offset;
        vk_range.size = range.size;

        inline_constant_ranges.push_back(vk_range);

        OGFX_LOG("  Inline constant range " + std::to_string(i) + ":");
        OGFX_LOG("    Stages: " + to_string_bitmask(range.stages));
        OGFX_LOG("    Offset: " + std::to_string(range.offset));
        OGFX_LOG("    Size: " + std::to_string(range.size));
    }

    // Pipeline layout
    VkPipelineLayoutCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    create_info.setLayoutCount = static_cast<uint32_t>(set_layouts.size());
    create_info.pSetLayouts = set_layouts.empty() ? nullptr : set_layouts.data();
    create_info.pushConstantRangeCount = static_cast<uint32_t>(inline_constant_ranges.size());
    create_info.pPushConstantRanges = inline_constant_ranges.empty() ? nullptr : inline_constant_ranges.data();

    const VkResult result = vkCreatePipelineLayout(m_impl->m_device, &create_info, nullptr, &m_impl->m_pipeline_layout);

    if (result != VK_SUCCESS)
    {
        OGFX_LOG_ERROR("Failed to create pipeline layout");
        throw std::runtime_error("Failed to create pipeline layout");
    }

    OGFX_LOG("Pipeline layout created");
}

PipelineLayout::~PipelineLayout()
{
    if (m_impl && m_impl->m_pipeline_layout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(m_impl->m_device, m_impl->m_pipeline_layout, nullptr);

        m_impl->m_pipeline_layout = VK_NULL_HANDLE;

        OGFX_LOG("Pipeline layout destroyed");
    }
}

PipelineLayout::PipelineLayout(PipelineLayout&&) noexcept = default;

PipelineLayout& PipelineLayout::operator=(PipelineLayout&&) noexcept = default;

} // namespace ogfx