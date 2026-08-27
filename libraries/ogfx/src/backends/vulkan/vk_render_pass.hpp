#ifndef OGFX_VK_RENDER_PASS_HPP
#define OGFX_VK_RENDER_PASS_HPP

#include <ogfx/render_pass.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

struct RenderPass::Impl
{
    VkRenderPass m_render_pass = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
};

} // namespace ogfx

#endif // OGFX_VK_RENDER_PASS_HPP