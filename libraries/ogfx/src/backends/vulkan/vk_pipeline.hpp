#ifndef OGFX_VK_PIPELINE_HPP
#define OGFX_VK_PIPELINE_HPP

#include <ogfx/pipeline.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

struct Pipeline::Impl
{
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
};

} // namespace ogfx

#endif // OGFX_VK_PIPELINE_HPP