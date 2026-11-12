#ifndef OMEGA_MODEL_HANDLE_HPP
#define OMEGA_MODEL_HANDLE_HPP

#include <cstdint>
#include <limits>

namespace omega
{

struct ModelHandle
{
    static constexpr uint32_t invalid_index = std::numeric_limits<uint32_t>::max();

    uint32_t index = invalid_index;
    uint32_t generation = 0;

    constexpr bool is_valid() const { return index != invalid_index; }
    constexpr bool operator==(const ModelHandle&) const = default;
};

} // namespace omega

#endif