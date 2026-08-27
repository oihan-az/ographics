#ifndef OGFX_VK_FRAMEBUFFER_HPP
#define OGFX_VK_FRAMEBUFFER_HPP

#include <ogfx/framebuffer.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

struct Framebuffer::Impl
{
    VkFramebuffer m_framebuffer = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
};

} // namespace ogfx

#endif // OGFX_VK_FRAMEBUFFER_HPP