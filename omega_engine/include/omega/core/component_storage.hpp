#ifndef OMEGA_COMPONENT_STORAGE_HPP
#define OMEGA_COMPONENT_STORAGE_HPP

#include <omega/core/entity.hpp>

#include <cstdint>
#include <vector>

namespace omega
{

template <typename T> 
class ComponentStorage
{
  public:
    bool add(Entity entity, T component);
    bool remove(Entity entity);

    bool contains(Entity entity) const;

    void clear();

    T* get(Entity entity);
    const T* get(Entity entity) const;

    uint32_t size() const;

    auto begin();
    auto end();
    auto begin() const;
    auto end() const;

    Entity entity_at(uint32_t index) const;

  private:
    std::vector<uint32_t> m_sparse;
    std::vector<Entity> m_entities;
    std::vector<T> m_components;
};

} // namespace omega

#include "component_storage.tpp"

#endif // OMEGA_COMPONENT_STORAGE_HPP