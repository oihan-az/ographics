#include <ogfx/image.hpp>
#include <ogfx/device.hpp>

#include <debug.hpp>

#include <vulkan/vulkan.h>

#include "vk_image.hpp"
#include "vk_device.hpp"
#include "vk_format.hpp"
#include "vk_image_usage.hpp"

namespace ogfx
{

Image::Image(const Device& device, const ImageDesc& desc) : m_desc(desc), m_impl(std::make_unique<Impl>())
{
    m_impl->m_device = device.m_impl->m_device;

    if (!m_desc.owned)
    {
        return;
    }

    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.format = to_vk_format(m_desc.format);
    image_info.extent = {m_desc.width, m_desc.height, 1};
    image_info.mipLevels = m_desc.mip_levels;
    image_info.arrayLayers = m_desc.array_layers;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = to_vk_image_usage(m_desc.usage);
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if (vkCreateImage(m_impl->m_device, &image_info, nullptr, &m_impl->m_image) != VK_SUCCESS)
    {
        m_impl->m_image = VK_NULL_HANDLE;
        throw std::runtime_error("Failed to create Vulkan image");
    }

    OGFX_LOG("Vulkan image created");
}

Image::~Image() 
{
    if (m_desc.owned && m_impl && m_impl->m_image != VK_NULL_HANDLE)
    {
        vkDestroyImage(m_impl->m_device, m_impl->m_image, nullptr);

        OGFX_LOG("Vulkan image destroyed");
    }
}

Image::Image(Image&&) noexcept = default;
Image& Image::operator=(Image&&) noexcept = default;

uint32_t Image::width() const
{
    return m_desc.width;
}

uint32_t Image::height() const
{
    return m_desc.height;
}

Format Image::format() const
{
    return m_desc.format;
}

} // namespace ogfx