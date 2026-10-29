#ifndef OGFX_ACCESS_FLAGS_HPP
#define OGFX_ACCESS_FLAGS_HPP

#include <ogfx/enum_flags.hpp>

namespace ogfx
{

OGFX_BITMASK_ENUM(AccessFlags, uint32_t,
    None = 0,
    IndirectCommandRead = 1u << 0,
    IndexRead = 1u << 1,
    VertexAttributeRead = 1u << 2,
    UniformRead = 1u << 3,
    ShaderRead = 1u << 4,
    ShaderWrite = 1u << 5,
    ColorAttachmentRead = 1u << 6,
    ColorAttachmentWrite = 1u << 7,
    DepthStencilAttachmentRead = 1u << 8,
    DepthStencilAttachmentWrite = 1u << 9,
    TransferRead = 1u << 10,
    TransferWrite = 1u << 11,
    HostRead = 1u << 12,
    HostWrite = 1u << 13,
    MemoryRead = 1u << 14,
    MemoryWrite = 1u << 15,
    AccelerationStructureRead  = 1u << 16,
    AccelerationStructureWrite = 1u << 17
);

} // namespace ogfx

#endif // OGFX_ACCESS_FLAGS_HPP