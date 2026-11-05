#ifndef OMEGA_COMPONENT_MANAGER_HPP
#define OMEGA_COMPONENT_MANAGER_HPP

#include <omega/core/component_storage.hpp>

#include <typeindex>
#include <memory>
#include <unordered_map>

namespace omega
{

class ComponentManager
{
  public:

    template <typename T> ComponentStorage<T>& get_storage();
    template <typename T> bool remove_storage();

    template <typename T> bool has_storage() const;

    void clear();

    template <typename T> bool add(Entity entity, T component);
    template <typename T> bool remove(Entity entity);

    template <typename T> T* get(Entity entity);
    template <typename T> const T* get(Entity entity) const;

    template <typename T> bool has(Entity entity) const;

  private:
    std::unordered_map<std::type_index, std::unique_ptr<IComponentStorage>> m_storages;
};
} // namespace omega

#include "component_manager.tpp"

#endif // OMEGA_COMPONENT_MANAGER_HPP