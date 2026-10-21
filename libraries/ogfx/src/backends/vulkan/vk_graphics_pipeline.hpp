#ifndef OGFX_VK_GRAPHICS_PIEPLINE_HPP
#define OGFX_VK_GRAPHICS_PIEPLINE_HPP

#include <ogfx/graphics_pipeline.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

struct GraphicsPipeline::Impl
{
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
};

} // namespace ogfx

#endif // OGFX_VK_GRAPHICS_PIEPLINE_HPP