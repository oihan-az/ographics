#ifndef OGFX_VK_IMAGE_HPP
#define OGFX_VK_IMAGE_HPP

#include <ogfx/image.hpp>
#include <ogfx/format.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

VkImageType to_vk_image_type(ImageType type);
ImageType from_vk_image_type(VkImageType type);
VkImageCreateFlags to_vk_image_create_flags(ImageType type);

struct Image::Impl
{
    VkDevice m_device = VK_NULL_HANDLE;
    VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
    VkImage m_image = VK_NULL_HANDLE;
    VkDeviceMemory m_memory = VK_NULL_HANDLE;
};

} // namespace ogfx

#endif // OGFX_VK_IMAGE_HPP