#ifndef OGFX_FORMAT_HPP
#define OGFX_FORMAT_HPP

#include <cstdint>

namespace ogfx
{

enum class Format
{
    Undefined,

    R8_UNORM,
    RG8_UNORM,
    RGBA8_UNORM,
    BGRA8_UNORM,

    RGBA8_SRGB,
    BGRA8_SRGB,

    RG16_FLOAT,
    RGBA16_FLOAT,

    RG32_FLOAT,
    RGB32_FLOAT,
    RGBA32_FLOAT,

    D16_UNORM,
    D24_UNORM_S8_UINT,
    D32_FLOAT,
    D32_FLOAT_S8_UINT,
};

struct FormatInfo
{
    uint32_t bytes_per_pixel = 0;
};

bool inline get_format_info(Format format, FormatInfo& out_info)
{
    switch (format)
    {
    case Format::R8_UNORM:
        out_info = {1};
        return true;

    case Format::RG8_UNORM:
        out_info = {2};
        return true;

    case Format::RG16_FLOAT:
        out_info = {4};
        return true;

    case Format::RGBA8_UNORM:
    case Format::BGRA8_UNORM:
    case Format::RGBA8_SRGB:
    case Format::BGRA8_SRGB:
    case Format::D24_UNORM_S8_UINT:
    case Format::D32_FLOAT:
        out_info = {4};
        return true;

    case Format::RGBA16_FLOAT:
    case Format::RG32_FLOAT:
    case Format::D32_FLOAT_S8_UINT:
        out_info = {8};
        return true;

    case Format::RGB32_FLOAT:
        out_info = {12};
        return true;

    case Format::RGBA32_FLOAT:
        out_info = {16};
        return true;

    case Format::D16_UNORM:
        out_info = {2};
        return true;

    case Format::Undefined:
    default:
        out_info = {};
        return false;
    }
}

} // namespace ogfx

#endif // OGFX_FORMAT_HPP