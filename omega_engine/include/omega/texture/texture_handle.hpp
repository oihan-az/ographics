#ifndef OMEGA_TEXTURE_HANDLE_HPP
#define OMEGA_TEXTURE_HANDLE_HPP

#include <cstdint>
#include <limits>

namespace omega
{

struct TextureHandle
{
    static constexpr uint32_t invalid_index = std::numeric_limits<uint32_t>::max();

    uint32_t index = invalid_index;
    uint32_t generation = 0;

    constexpr bool is_valid() const { return index != invalid_index; }

    constexpr bool operator==(const TextureHandle&) const = default;
};

struct TextureHandleHash
{
    size_t operator()(const TextureHandle& handle) const noexcept
    {
        return handle.index ^ (handle.generation << 1);
    }
};

} // namespace omega

#endif // OMEGA_TEXTURE_HANDLE_HPP