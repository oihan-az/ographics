#include "vk_format.hpp"

namespace ogfx
{

VkFormat to_vk_format(Format format)
{
    switch (format)
    {
    case Format::Undefined:
        return VK_FORMAT_UNDEFINED;

    case Format::BGRA8_UNORM:
        return VK_FORMAT_B8G8R8A8_UNORM;

    case Format::RGBA8_UNORM:
        return VK_FORMAT_R8G8B8A8_UNORM;

    case Format::BGRA8_SRGB:
        return VK_FORMAT_B8G8R8A8_SRGB;

    case Format::RGBA8_SRGB:
        return VK_FORMAT_R8G8B8A8_SRGB;

    case Format::D24_UNORM_S8_UINT:
        return VK_FORMAT_D24_UNORM_S8_UINT;

    case Format::D32_FLOAT:
        return VK_FORMAT_D32_SFLOAT;
    }

    return VK_FORMAT_UNDEFINED;
}

Format from_vk_format(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_B8G8R8A8_UNORM:
        return Format::BGRA8_UNORM;

    case VK_FORMAT_R8G8B8A8_UNORM:
        return Format::RGBA8_UNORM;

    case VK_FORMAT_B8G8R8A8_SRGB:
        return Format::BGRA8_SRGB;

    case VK_FORMAT_R8G8B8A8_SRGB:
        return Format::RGBA8_SRGB;

    case VK_FORMAT_D24_UNORM_S8_UINT:
        return Format::D24_UNORM_S8_UINT;

    case VK_FORMAT_D32_SFLOAT:
        return Format::D32_FLOAT;

    default:
        return Format::Undefined;
    }
}

} // namespace ogfx