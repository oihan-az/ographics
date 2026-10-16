#ifndef OGFX_IMAGE_USAGE_HPP
#define OGFX_IMAGE_USAGE_HPP

#include <ogfx/enum_flags.hpp>

#include <cstdint>
#include <string>

namespace ogfx
{

OGFX_BITMASK_ENUM(ImageUsage, uint32_t,
    None = 0,
    TransferSrc = 1u << 0,
    TransferDst = 1u << 1,
    Sampled = 1u << 2,
    Storage = 1u << 3,
    ColorAttachment = 1u << 4,
    DepthStencil = 1u << 5
);

} // namespace ogfx

#endif // OGFX_IMAGE_USAGE_HPP