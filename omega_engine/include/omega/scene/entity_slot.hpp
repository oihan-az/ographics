#ifndef OMEGA_ENTITY_SLOT_HPP
#define OMEGA_ENTITY_SLOT_HPP

#include <cstdint>

namespace omega
{

enum class EntityState
{
    Free,
    PendingCreation,
    Alive,
    Retired
};

struct EntitySlot
{
    uint32_t generation = 0;
    EntityState state = EntityState::Free;

    bool is_alive() const { return state == EntityState::Alive;};
};

} // namespace omega

#endif // OMEGA_ENTITY_SLOT_HPP