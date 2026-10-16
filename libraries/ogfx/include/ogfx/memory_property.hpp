#ifndef OGFX_MEMORY_PROPERTY_HPP
#define OGFX_MEMORY_PROPERTY_HPP

#include <cstdint>
#include <string>

namespace ogfx
{

enum class MemoryProperty : uint32_t
{
    None = 0,
    DeviceLocal = 1 << 0,
    HostVisible = 1 << 1,
    HostCoherent = 1 << 2,
    HostCached = 1 << 3
};

constexpr MemoryProperty operator|(MemoryProperty lhs, MemoryProperty rhs)
{
    return static_cast<MemoryProperty>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr MemoryProperty operator&(MemoryProperty lhs, MemoryProperty rhs)
{
    return static_cast<MemoryProperty>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr bool has_flag(MemoryProperty value, MemoryProperty flag)
{
    return (value & flag) == flag;
}

std::string to_string(MemoryProperty property);

} // namespace ogfx

#endif // OGFX_MEMORY_PROPERTY_HPP