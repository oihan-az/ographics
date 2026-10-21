#include <debug.hpp>
#include <ogfx/enum_string.hpp>

#include "vk_image_view.hpp"
#include "vk_image.hpp"
#include "vk_format.hpp"

namespace ogfx
{

VkImageViewType to_vk_image_view_type(ImageViewType type)
{
    switch (type)
    {
    case ImageViewType::View1D:
        return VK_IMAGE_VIEW_TYPE_1D;

    case ImageViewType::View2D:
        return VK_IMAGE_VIEW_TYPE_2D;

    case ImageViewType::View3D:
        return VK_IMAGE_VIEW_TYPE_3D;

    case ImageViewType::View1DArray:
        return VK_IMAGE_VIEW_TYPE_1D_ARRAY;

    case ImageViewType::View2DArray:
        return VK_IMAGE_VIEW_TYPE_2D_ARRAY;

    case ImageViewType::Cube:
        return VK_IMAGE_VIEW_TYPE_CUBE;

    case ImageViewType::CubeArray:
        return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;

    case ImageViewType::Undefined:
    default:
        return VK_IMAGE_VIEW_TYPE_2D;
    }
}

ImageViewType from_vk_image_view_type(VkImageViewType type)
{
    switch (type)
    {
    case VK_IMAGE_VIEW_TYPE_1D:
        return ImageViewType::View1D;

    case VK_IMAGE_VIEW_TYPE_2D:
        return ImageViewType::View2D;

    case VK_IMAGE_VIEW_TYPE_3D:
        return ImageViewType::View3D;

    case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
        return ImageViewType::View1DArray;

    case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
        return ImageViewType::View2DArray;

    case VK_IMAGE_VIEW_TYPE_CUBE:
        return ImageViewType::Cube;

    case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
        return ImageViewType::CubeArray;

    default:
        return ImageViewType::Undefined;
    }
}

ImageView::ImageView(const ImageViewDesc& desc) : m_impl(std::make_unique<Impl>())
{
    const VkDevice device = desc.image->m_impl->m_device;
    const VkImage vk_image = desc.image->m_impl->m_image;
    const VkFormat format = to_vk_format(desc.image->format());

    OGFX_LOG("Creating Vulkan image view:");
    OGFX_LOG("  Type: " + to_string_enum(desc.type));
    OGFX_LOG("  Format: " + to_string_enum(desc.image->format()));
    OGFX_LOG("  Base mip level: " + std::to_string(desc.base_mip_level));
    OGFX_LOG("  Mip level count: " + std::to_string(desc.mip_level_count));
    OGFX_LOG("  Base array layer: " + std::to_string(desc.base_array_layer));
    OGFX_LOG("  Array layer count: " + std::to_string(desc.array_layer_count));

    VkImageViewCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = vk_image;
    create_info.viewType = to_vk_image_view_type(desc.type);
    create_info.format = format;

    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    create_info.subresourceRange.baseMipLevel = desc.base_mip_level;
    create_info.subresourceRange.levelCount = desc.mip_level_count;

    create_info.subresourceRange.baseArrayLayer = desc.base_array_layer;
    create_info.subresourceRange.layerCount = desc.array_layer_count;

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