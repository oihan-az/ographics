#ifndef OGFX_VK_IMAGE_HPP
#define OGFX_VK_IMAGE_HPP

#include <ogfx/image.hpp>
#include <ogfx/format.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

struct Image::Impl
{
    VkImage m_image = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
};

} // namespace ogfx

#endif // OGFX_VK_IMAGE_HPP