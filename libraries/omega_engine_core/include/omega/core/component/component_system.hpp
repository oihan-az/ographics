#ifndef OMEGA_COMPONENT_SYSTEM_HPP
#define OMEGA_COMPONENT_SYSTEM_HPP

#include <omega/core/component/component_storage.hpp>
#include <omega/core/event/event_system.hpp>
#include <omega/core/entity/entity_system.hpp>

#include <typeindex>
#include <memory>
#include <unordered_map>
#include <vector>

namespace omega
{

class ComponentSystem
{
  public:

    ComponentSystem(EventSystem& events, EntitySystem& entities);

    void update();

    template <typename T> ComponentStorage<T>& get_storage();
    template <typename T> bool remove_storage();
    template <typename T> bool has_storage() const;

    template <typename T> bool add(Entity entity, T component);
    template <typename T> bool remove(Entity entity);
    template <typename T> bool mark_dirty(Entity entity);
    template <typename T> bool has(Entity entity) const;

    template <typename T> T* get(Entity entity);
    template <typename T> const T* get(Entity entity) const;

    template <typename T> std::vector<Entity> entities_with() const;

  private:
    std::unordered_map<std::type_index, std::unique_ptr<IComponentStorage>> m_storages;

    // Events
    EventSystem& m_events;

    // Entities
    EntitySystem& m_entities;
};
} // namespace omega

#include "component_system.tpp"

#endif // OMEGA_COMPONENT_SYSTEM_HPP