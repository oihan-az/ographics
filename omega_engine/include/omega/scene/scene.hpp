#ifndef OMEGA_SCENE_HPP
#define OMEGA_SCENE_HPP

#include "entity_slot.hpp"
#include "entity_iterator.hpp"
#include "entity_commad.hpp"
#include <omega/core/entity.hpp>

#include <cstdint>
#include <vector>

namespace omega
{

class Scene
{
  public:
    Entity create_entity();
    bool destroy_entity(Entity entity);
    void flush_commands();

    bool is_alive(Entity entity) const;
    bool is_destructible(Entity entity) const;

    uint32_t entity_count() const;

    EntityIterator begin() const;
    EntityIterator end() const;

  private:
    std::vector<EntitySlot> m_entity_slots;
    std::vector<uint32_t> m_free_indices;

    uint32_t m_entity_count = 0;

    std::vector<EntityCommand> m_pending_commands;

    friend class EntityIterator;
};

} // namespace omega

#endif // OMEGA_SCENE_HPP