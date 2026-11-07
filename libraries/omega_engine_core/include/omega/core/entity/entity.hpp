#ifndef OMEGA_ENTITY_HPP
#define OMEGA_ENTITY_HPP

#include <cstdint>
#include <limits>
#include <functional>

namespace omega
{

struct Entity
{
    static constexpr uint32_t invalid_index = std::numeric_limits<uint32_t>::max();

    uint32_t index = invalid_index;
    uint32_t generation = 0;

    bool is_valid() const { return index != invalid_index; }
    static Entity invalid() { return {}; }

    bool operator==(const Entity&) const = default;
};

} // namespace omega

#endif // OMEGA_ENTITY_HPP