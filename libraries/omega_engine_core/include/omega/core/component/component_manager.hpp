#ifndef OMEGA_COMPONENT_MANAGER_HPP
#define OMEGA_COMPONENT_MANAGER_HPP

#include <omega/core/component/component_storage.hpp>

#include <typeindex>
#include <memory>
#include <unordered_map>
#include <vector>

namespace omega
{

class EventSystem;

class ComponentManager
{
  public:

    template <typename T> ComponentStorage<T>& get_storage();
    template <typename T> bool remove_storage();

    template <typename T> bool has_storage() const;

    template <typename T> bool add(Entity entity, T component);
    template <typename T> bool remove(Entity entity);

    template <typename T> T* get(Entity entity);
    template <typename T> const T* get(Entity entity) const;

    template <typename T> bool has(Entity entity) const;

    template <typename T, typename Callback> void for_each(Callback&& callback);
    template <typename T, typename Callback> void for_each(Callback&& callback) const;

    template <typename T> void publish_added(Entity entity, EventSystem& events);
    template <typename T> void publish_removed(Entity entity, EventSystem& events);
    template <typename T> void publish_changed(Entity entity, EventSystem& events);

    void clear_all();
    void remove_all(Entity entity);
    void remove_all(Entity entity, EventSystem& events);

  private:
    std::unordered_map<std::type_index, std::unique_ptr<IComponentStorage>> m_storages;
};
} // namespace omega

#include "component_manager.tpp"

#endif // OMEGA_COMPONENT_MANAGER_HPP