#ifndef OMEGA_ENTITY_SYSTEM_HPP
#define OMEGA_ENTITY_SYSTEM_HPP

#include <omega/core/entity/entity.hpp>
#include <omega/core/entity/entity_slot.hpp>
#include <omega/core/entity/entity_iterator.hpp>

#include <vector>

namespace omega
{

class EventSystem;

class EntitySystem
{
  public:
    EntitySystem(EventSystem& events);

    void update();

    Entity create_entity();
    bool destroy_entity(Entity entity);

    bool is_alive(Entity entity) const;
    bool is_destructible(Entity entity) const;

    uint32_t entity_count() const;

    EntityIterator begin() const;
    EntityIterator end() const;

  private:
    friend class EntityIterator;

    // Entities
    std::vector<EntitySlot> m_entity_slots;
    std::vector<uint32_t> m_free_indices;

    uint32_t m_entity_count = 0;

    // Events
    EventSystem& m_events;
};

} // namespace omega

#endif // OMEGA_ENTITY_SYSTEM_HPP