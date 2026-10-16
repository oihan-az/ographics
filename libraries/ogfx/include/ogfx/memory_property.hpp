#ifndef OGFX_MEMORY_PROPERTY_HPP
#define OGFX_MEMORY_PROPERTY_HPP

#include <ogfx/enum_flags.hpp>

#include <cstdint>
#include <string>

namespace ogfx
{

OGFX_BITMASK_ENUM(MemoryProperty, uint32_t,
    None = 0,
    DeviceLocal = 1u << 0,
    HostVisible = 1u << 1,
    HostCoherent = 1u << 2,
    HostCached = 1u << 3
);

} // namespace ogfx

#endif // OGFX_MEMORY_PROPERTY_HPP