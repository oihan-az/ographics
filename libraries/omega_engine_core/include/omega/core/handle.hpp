#ifndef OMEGA_CORE_HANDLE_HPP
#define OMEGA_CORE_HANDLE_HPP

#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>

namespace omega
{

template <typename Tag> 
struct Handle
{
    static constexpr uint32_t invalid_index = std::numeric_limits<uint32_t>::max();

    uint32_t index = invalid_index;
    uint32_t generation = 0;

    constexpr bool is_valid() const { return index != invalid_index; }
    static constexpr Handle invalid() { return {}; }

    constexpr bool operator==(const Handle&) const = default;
};

template <typename Tag> 
struct HandleHash
{
    uint64_t operator()(const Handle<Tag>& handle) const noexcept
    {
        return (static_cast<uint64_t>(handle.generation) << 32) | handle.index;
    }
};

} // namespace omega

#endif // OMEGA_CORE_HANDLE_HPP