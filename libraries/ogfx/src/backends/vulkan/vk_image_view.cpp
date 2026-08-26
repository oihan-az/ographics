#include <ogfx/image_view.hpp>
#include <ogfx/image.hpp>

#include <debug.hpp>

#include <vulkan/vulkan.h>

#include "vk_image_view.hpp"
#include "vk_image.hpp"
#include "vk_format.hpp"

namespace ogfx
{

ImageView::ImageView(Image& image) : m_impl(std::make_unique<Impl>())
{
    const VkDevice device = image.m_impl->m_device;
    const VkImage vk_image = image.m_impl->m_image;
    const VkFormat format = to_vk_format(image.format());

    VkImageViewCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = vk_image;
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create_info.format = format;

    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    create_info.subresourceRange.baseMipLevel = 0;
    create_info.subresourceRange.levelCount = 1;

    create_info.subresourceRange.baseArrayLayer = 0;
    create_info.subresourceRange.layerCount = 1;

    OGFX_LOG("Creating Vulkan image view");

    if (vkCreateImageView(device, &create_info, nullptr, &m_impl->m_image_view) != VK_SUCCESS)
    {
        OGFX_LOG_ERROR("Failed to create Vulkan image view");
        throw std::runtime_error("Failed to create Vulkan image view.");
    }

    OGFX_LOG("Vulkan image view created");

    m_impl->m_device = device;
}

ImageView::~ImageView()
{
    if (m_impl && m_impl->m_image_view != VK_NULL_HANDLE)
    {
        vkDestroyImageView(m_impl->m_device, m_impl->m_image_view, nullptr);

        OGFX_LOG("Vulkan image view destroyed");
    }
}

ImageView::ImageView(ImageView&&) noexcept = default;

ImageView& ImageView::operator=(ImageView&&) noexcept = default;

} // namespace ogfx