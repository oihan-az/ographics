#include <debug.hpp>
#include <ogfx/enum_string.hpp>

#include "vk_compute_pipeline.hpp"
#include "vk_pipeline.hpp"
#include "vk_device.hpp"
#include "vk_pipeline_layout.hpp"
#include "vk_shader.hpp"
#include "vk_shader_stage.hpp"

#include <stdexcept>

namespace ogfx
{

ComputePipeline::ComputePipeline(const Device& device, const ComputePipelineDesc& desc) : Pipeline(device)
{
    if (desc.layout == nullptr || !desc.layout->m_impl)
    {
        OGFX_LOG_ERROR("Cannot create compute pipeline: invalid pipeline layout");
        throw std::invalid_argument("Compute pipeline requires a valid pipeline layout.");
    }

    if (desc.shader == nullptr || !desc.shader->m_impl)
    {
        OGFX_LOG_ERROR("Cannot create compute pipeline: invalid shader");
        throw std::invalid_argument("Compute pipeline requires a valid shader.");
    }

    if (desc.shader->m_stage != ShaderStage::Compute)
    {
        OGFX_LOG_ERROR("Cannot create compute pipeline: shader stage is " + to_string_enum(desc.shader->m_stage));
        throw std::invalid_argument("Compute pipeline requires a compute shader.");
    }

    OGFX_LOG("Creating compute pipeline:");
    OGFX_LOG("  Shader stage: " + to_string_enum(desc.shader->m_stage));
    OGFX_LOG("  Entry point: '" + desc.shader->m_entry_point + "'");

    VkPipelineShaderStageCreateInfo shader_stage{};
    shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shader_stage.module = desc.shader->m_impl->m_shader_module;
    shader_stage.pName = desc.shader->m_entry_point.c_str();

    VkComputePipelineCreateInfo pipeline_info{};
    pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipeline_info.stage = shader_stage;
    pipeline_info.layout = desc.layout->m_impl->m_pipeline_layout;

    if (vkCreateComputePipelines(m_impl->m_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &m_impl->m_pipeline) !=
        VK_SUCCESS)
    {
        OGFX_LOG_ERROR("Failed to create compute pipeline");
        throw std::runtime_error("Failed to create compute pipeline.");
    }

    OGFX_LOG("Compute pipeline created");
}

ComputePipeline::~ComputePipeline()
{
    OGFX_LOG("Compute pipeline destroyed");
}

ComputePipeline::ComputePipeline(ComputePipeline&&) noexcept = default;
ComputePipeline& ComputePipeline::operator=(ComputePipeline&&) noexcept = default;

} // namespace ogfx