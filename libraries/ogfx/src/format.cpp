#include <ogfx/format.hpp>

namespace ogfx
{

const char* to_string(Format format)
{
    switch (format)
    {
    case Format::Undefined:
        return "Undefined";

    case Format::BGRA8_UNORM:
        return "BGRA8_UNORM";

    case Format::RGBA8_UNORM:
        return "RGBA8_UNORM";

    case Format::BGRA8_SRGB:
        return "BGRA8_SRGB";

    case Format::RGBA8_SRGB:
        return "RGBA8_SRGB";

    case Format::D24_UNORM_S8_UINT:
        return "D24_UNORM_S8_UINT";

    case Format::D32_FLOAT:
        return "D32_FLOAT";
    }

    return "Unknown";
}

} // namespace ogfx