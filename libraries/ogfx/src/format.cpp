#include <ogfx/format.hpp>

namespace ogfx
{

const char* to_string(Format format)
{
    switch (format)
    {
    case Format::Undefined:
        return "Undefined";

    case Format::R8_UNORM:
        return "R8_UNORM";

    case Format::RG8_UNORM:
        return "RG8_UNORM";

    case Format::RGBA8_UNORM:
        return "RGBA8_UNORM";

    case Format::BGRA8_UNORM:
        return "BGRA8_UNORM";

    case Format::RGBA8_SRGB:
        return "RGBA8_SRGB";

    case Format::BGRA8_SRGB:
        return "BGRA8_SRGB";

    case Format::RG16_FLOAT:
        return "RG16_FLOAT";

    case Format::RGBA16_FLOAT:
        return "RGBA16_FLOAT";

    case Format::RG32_FLOAT:
        return "RG32_FLOAT";

    case Format::RGB32_FLOAT:
        return "RGB32_FLOAT";

    case Format::RGBA32_FLOAT:
        return "RGBA32_FLOAT";

    case Format::D16_UNORM:
        return "D16_UNORM";

    case Format::D24_UNORM_S8_UINT:
        return "D24_UNORM_S8_UINT";

    case Format::D32_FLOAT:
        return "D32_FLOAT";

    case Format::D32_FLOAT_S8_UINT:
        return "D32_FLOAT_S8_UINT";
    }

    return "Unknown";
}

} // namespace ogfx