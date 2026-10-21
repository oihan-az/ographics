#ifndef OGFX_VK_PIPELINE_LAYOUT_HPP
#define OGFX_VK_PIPELINE_LAYOUT_HPP

#include <ogfx/pipeline_layout.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

struct PipelineLayout::Impl
{
    VkDevice m_device = VK_NULL_HANDLE;
    VkPipelineLayout m_pipeline_layout = VK_NULL_HANDLE;
};

} // namespace ogfx

#endif // OGFX_VK_PIPELINE_LAYOUT_HPP
