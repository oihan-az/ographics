#include "vk_format.hpp"

namespace ogfx
{

VkFormat to_vk_format(Format format)
{
    switch (format)
    {
    case Format::Undefined:
        return VK_FORMAT_UNDEFINED;

    case Format::R8_UNORM:
        return VK_FORMAT_R8_UNORM;

    case Format::RG8_UNORM:
        return VK_FORMAT_R8G8_UNORM;

    case Format::RGBA8_UNORM:
        return VK_FORMAT_R8G8B8A8_UNORM;

    case Format::BGRA8_UNORM:
        return VK_FORMAT_B8G8R8A8_UNORM;

    case Format::RGBA8_SRGB:
        return VK_FORMAT_R8G8B8A8_SRGB;

    case Format::BGRA8_SRGB:
        return VK_FORMAT_B8G8R8A8_SRGB;

    case Format::RG16_FLOAT:
        return VK_FORMAT_R16G16_SFLOAT;

    case Format::RGBA16_FLOAT:
        return VK_FORMAT_R16G16B16A16_SFLOAT;

    case Format::RG32_FLOAT:
        return VK_FORMAT_R32G32_SFLOAT;

    case Format::RGB32_FLOAT:
        return VK_FORMAT_R32G32B32_SFLOAT;

    case Format::RGBA32_FLOAT:
        return VK_FORMAT_R32G32B32A32_SFLOAT;

    case Format::D16_UNORM:
        return VK_FORMAT_D16_UNORM;

    case Format::D24_UNORM_S8_UINT:
        return VK_FORMAT_D24_UNORM_S8_UINT;

    case Format::D32_FLOAT:
        return VK_FORMAT_D32_SFLOAT;

    case Format::D32_FLOAT_S8_UINT:
        return VK_FORMAT_D32_SFLOAT_S8_UINT;
    }

    return VK_FORMAT_UNDEFINED;
}

Format from_vk_format(VkFormat format)
{
    switch (format)
    {
    case VK_FORMAT_R8_UNORM:
        return Format::R8_UNORM;

    case VK_FORMAT_R8G8_UNORM:
        return Format::RG8_UNORM;

    case VK_FORMAT_R8G8B8A8_UNORM:
        return Format::RGBA8_UNORM;

    case VK_FORMAT_B8G8R8A8_UNORM:
        return Format::BGRA8_UNORM;

    case VK_FORMAT_R8G8B8A8_SRGB:
        return Format::RGBA8_SRGB;

    case VK_FORMAT_B8G8R8A8_SRGB:
        return Format::BGRA8_SRGB;

    case VK_FORMAT_R16G16_SFLOAT:
        return Format::RG16_FLOAT;

    case VK_FORMAT_R16G16B16A16_SFLOAT:
        return Format::RGBA16_FLOAT;

    case VK_FORMAT_R32G32_SFLOAT:
        return Format::RG32_FLOAT;

    case VK_FORMAT_R32G32B32_SFLOAT:
        return Format::RGB32_FLOAT;

    case VK_FORMAT_R32G32B32A32_SFLOAT:
        return Format::RGBA32_FLOAT;

    case VK_FORMAT_D16_UNORM:
        return Format::D16_UNORM;

    case VK_FORMAT_D24_UNORM_S8_UINT:
        return Format::D24_UNORM_S8_UINT;

    case VK_FORMAT_D32_SFLOAT:
        return Format::D32_FLOAT;

    case VK_FORMAT_D32_SFLOAT_S8_UINT:
        return Format::D32_FLOAT_S8_UINT;

    default:
        return Format::Undefined;
    }
}

} // namespace ogfx