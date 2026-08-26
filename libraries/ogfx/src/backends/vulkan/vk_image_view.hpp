#ifndef OGFX_VK_IMAGE_VIEW_HPP
#define OGFX_VK_IMAGE_VIEW_HPP

#include <ogfx/image_view.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

struct ImageView::Impl
{
    VkImageView m_image_view = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
};

} // namespace ogfx

#endif // OGFX_VK_IMAGE_VIEW_HPP