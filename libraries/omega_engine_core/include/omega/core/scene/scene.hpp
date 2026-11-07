#ifndef OMEGA_SCENE_HPP
#define OMEGA_SCENE_HPP

#include <omega/core/entity/entity.hpp>
#include <omega/core/entity/entity_slot.hpp>
#include <omega/core/entity/entity_iterator.hpp>
#include <omega/core/entity/entity_commad.hpp>
#include <omega/core/component/component_manager.hpp>
#include <omega/core/event/event_system.hpp>

#include <cstdint>
#include <vector>
#include <unordered_map>

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

    bool set_parent(Entity child, Entity parent);
    bool clear_parent(Entity child);

    bool has_parent(Entity entity) const;

    Entity get_parent(Entity entity) const;
    const std::vector<Entity>& get_children(Entity entity) const;

    // Components
    template <typename T> bool add(Entity entity, T component);
    template <typename T> bool remove(Entity entity);
    template <typename T> bool mark_dirty(Entity entity);

    template <typename T> T* get(Entity entity);
    template <typename T> const T* get(Entity entity) const;

    template <typename T> bool has(Entity entity) const;

    template <typename T, typename Callback> void for_each(Callback&& callback);
    template <typename T, typename Callback> void for_each(Callback&& callback) const;

  private:

    // Entities
    std::vector<EntitySlot> m_entity_slots;
    std::vector<uint32_t> m_free_indices;

    uint32_t m_entity_count = 0;

    std::vector<EntityCommand> m_pending_commands;

    std::unordered_map<Entity, Entity, EntityHash> m_parents;
    std::unordered_map<Entity, std::vector<Entity>, EntityHash> m_children;

    friend class EntityIterator;

    // Components
    ComponentManager m_components;

    // Events
    EventSystem* m_events = nullptr;
};

} // namespace omega

#include "scene.tpp"

#endif // OMEGA_SCENE_HPP