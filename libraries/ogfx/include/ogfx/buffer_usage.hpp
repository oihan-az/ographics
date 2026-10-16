#ifndef OGFX_BUFFER_USAGE_HPP
#define OGFX_BUFFER_USAGE_HPP

#include <ogfx/enum_flags.hpp>

#include <cstdint>
#include <string>

namespace ogfx
{

OGFX_BITMASK_ENUM(BufferUsage, uint32_t,
    None = 0,
    Vertex = 1u << 0,
    Index = 1u << 1,
    Uniform = 1u << 2,
    Storage = 1u << 3,
    TransferSrc = 1u << 4,
    TransferDst = 1u << 5
);

} // namespace ogfx

#endif // OGFX_BUFFER_USAGE_HPP