#ifndef OGFX_IMAGE_ASPECT_HPP
#define OGFX_IMAGE_ASPECT_HPP

#include <ogfx/enum_flags.hpp>

#include <cstdint>
#include <string>

namespace ogfx
{

OGFX_BITMASK_ENUM(ImageAspect, uint32_t,
    None = 0,
    Color = 1u << 0,
    Depth = 1u << 1,
    Stencil = 1u << 2
);

} // namespace ogfx

#endif // OGFX_IMAGE_ASPECT_HPP