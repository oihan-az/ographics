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

struct EntityHash
{
    size_t operator()(const Entity& entity) const
    {
        size_t seed = std::hash<uint32_t>{}(entity.index);
        seed ^= std::hash<uint32_t>{}(entity.generation) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

} // namespace omega

#endif // OMEGA_ENTITY_HPP