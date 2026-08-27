#ifndef OGFX_VK_PIPELINE_STAGE_HPP
#define OGFX_VK_PIPELINE_STAGE_HPP

#include <ogfx/pipeline_stage.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

VkPipelineStageFlags to_vk_pipeline_stage(PipelineStage stage);
PipelineStage from_vk_pipeline_stage(VkPipelineStageFlags flags);

} // namespace ogfx

#endif // OGFX_VK_PIPELINE_STAGE_HPP