#ifndef OGFX_VK_PIPELINE_TYPE_HPP
#define OGFX_VK_PIPELINE_TYPE_HPP

#include <ogfx/pipeline_type.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

VkPipelineBindPoint to_vk_pipeline_type(PipelineType pipeline_type);
PipelineType from_vk_pipeline_type(VkPipelineBindPoint pipeline_type);

} // namespace ogfx

#endif // OGFX_VK_PIPELINE_TYPE_HPP