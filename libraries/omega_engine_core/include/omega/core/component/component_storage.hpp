#ifndef OMEGA_COMPONENT_STORAGE_HPP
#define OMEGA_COMPONENT_STORAGE_HPP

#include <omega/core/entity/entity.hpp>
#include <omega/core/entity/entity_indexer.hpp>
#include <omega/core/container/sparse_set.hpp>

#include <cstdint>
#include <vector>

namespace omega
{

class EventSystem;

class IComponentStorage
{
  public:
    virtual ~IComponentStorage() = default;

    virtual bool remove(Entity entity) = 0;
};

template <typename T> 
class ComponentStorage : public IComponentStorage
{
  public:
    ComponentStorage(EventSystem& events);

    bool add(Entity entity, T component);
    bool remove(Entity entity) override;
    bool mark_dirty(Entity entity);
    bool contains(Entity entity) const;

    uint32_t get_index(Entity entity);

    T* get(Entity entity);
    const T* get(Entity entity) const;

    uint32_t size() const;

    Entity entity_at(uint32_t index) const;
    std::vector<Entity> entities() const;

  private:
    SparseSetContainer<T, Entity> m_set;

    // Events
    EventSystem& m_events;
};

} // namespace omega

#include "component_storage.tpp"

#endif // OMEGA_COMPONENT_STORAGE_HPP