#ifndef OMEGA_SCENE_HPP
#define OMEGA_SCENE_HPP

#include "entity_slot.hpp"
#include "entity_iterator.hpp"
#include "entity_commad.hpp"

#include <omega/core/entity.hpp>
#include <omega/core/component_manager.hpp>
#include <omega/core/event.hpp>
#include <omega/core/component_event.hpp>

#include <cstdint>
#include <vector>

namespace omega
{

class Scene
{
  public:
    Scene() = default;
    explicit Scene(EventSystem& events);

    // Entities
    Entity create_entity();
    bool destroy_entity(Entity entity);
    void flush_commands();

    bool is_alive(Entity entity) const;
    bool is_destructible(Entity entity) const;

    uint32_t entity_count() const;

    EntityIterator begin() const;
    EntityIterator end() const;

    // Components
    template <typename T> bool add(Entity entity, T component);
    template <typename T> bool remove(Entity entity);

    template <typename T> T* get(Entity entity);
    template <typename T> const T* get(Entity entity) const;

    template <typename T> bool has(Entity entity) const;

  private:

    // Entities
    std::vector<EntitySlot> m_entity_slots;
    std::vector<uint32_t> m_free_indices;

    uint32_t m_entity_count = 0;

    std::vector<EntityCommand> m_pending_commands;

    friend class EntityIterator;

    // Components
    ComponentManager m_components;

    // Events
    EventSystem* m_events = nullptr;

    void publish_component_event(Entity entity, std::type_index component_type, ComponentEventType type);
};

} // namespace omega

#include "scene.tpp"

#endif // OMEGA_SCENE_HPP