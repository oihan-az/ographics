#include "vk_pipeline_stage.hpp"

namespace ogfx
{

VkPipelineStageFlags to_vk_pipeline_stage(PipelineStage stage)
{
    VkPipelineStageFlags flags = 0;

    if (has_flag(stage, PipelineStage::TopOfPipe))
        flags |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

    if (has_flag(stage, PipelineStage::DrawIndirect))
        flags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;

    if (has_flag(stage, PipelineStage::VertexInput))
        flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;

    if (has_flag(stage, PipelineStage::VertexShader))
        flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;

    if (has_flag(stage, PipelineStage::FragmentShader))
        flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    if (has_flag(stage, PipelineStage::ColorAttachmentOutput))
        flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    if (has_flag(stage, PipelineStage::ComputeShader))
        flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

    if (has_flag(stage, PipelineStage::Transfer))
        flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;

    if (has_flag(stage, PipelineStage::BottomOfPipe))
        flags |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

    return flags;
}

PipelineStage from_vk_pipeline_stage(VkPipelineStageFlags flags)
{
    PipelineStage stage = PipelineStage::None;

    if (flags & VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT)
        stage = stage | PipelineStage::TopOfPipe;

    if (flags & VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT)
        stage = stage | PipelineStage::DrawIndirect;

    if (flags & VK_PIPELINE_STAGE_VERTEX_INPUT_BIT)
        stage = stage | PipelineStage::VertexInput;

    if (flags & VK_PIPELINE_STAGE_VERTEX_SHADER_BIT)
        stage = stage | PipelineStage::VertexShader;

    if (flags & VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
        stage = stage | PipelineStage::FragmentShader;

    if (flags & VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
        stage = stage | PipelineStage::ColorAttachmentOutput;

    if (flags & VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT)
        stage = stage | PipelineStage::ComputeShader;

    if (flags & VK_PIPELINE_STAGE_TRANSFER_BIT)
        stage = stage | PipelineStage::Transfer;

    if (flags & VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT)
        stage = stage | PipelineStage::BottomOfPipe;

    return stage;
}

} // namespace ogfx