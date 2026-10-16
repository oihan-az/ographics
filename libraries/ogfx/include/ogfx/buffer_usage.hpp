#ifndef OGFX_BUFFER_USAGE_HPP
#define OGFX_BUFFER_USAGE_HPP

#include <cstdint>
#include <string>

namespace ogfx
{

enum class BufferUsage : uint32_t
{
    None = 0,
    Vertex = 1 << 0,
    Index = 1 << 1,
    Uniform = 1 << 2,
    Storage = 1 << 3,
    TransferSrc = 1 << 4,
    TransferDst = 1 << 5
};

constexpr BufferUsage operator|(BufferUsage lhs, BufferUsage rhs)
{
    return static_cast<BufferUsage>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr BufferUsage operator&(BufferUsage lhs, BufferUsage rhs)
{
    return static_cast<BufferUsage>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr bool has_flag(BufferUsage value, BufferUsage flag)
{
    return (value & flag) == flag;
}

std::string to_string(BufferUsage usage);

} // namespace ogfx

#endif // OGFX_BUFFER_USAGE_HPP