#ifndef OGFX_FORMAT_HPP
#define OGFX_FORMAT_HPP

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
    RGBA32_FLOAT,

    D16_UNORM,
    D24_UNORM_S8_UINT,
    D32_FLOAT,
    D32_FLOAT_S8_UINT,
};

const char* to_string(Format format);

} // namespace ogfx

#endif // OGFX_FORMAT_HPP