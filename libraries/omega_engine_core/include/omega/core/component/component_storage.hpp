#ifndef OMEGA_COMPONENT_STORAGE_HPP
#define OMEGA_COMPONENT_STORAGE_HPP

#include <omega/core/entity/entity.hpp>
#include <omega/core/container/sparse_set.hpp>

#include <cstdint>

namespace omega
{

class EventSystem;

class IComponentStorage
{
  public:
    virtual ~IComponentStorage() = default;

    virtual void clear() = 0;
    virtual bool remove(Entity entity) = 0;
    virtual bool remove(Entity entity, SparseSetSwapInfo<Entity>& swap_info) = 0;
    virtual bool contains(Entity entity) const = 0;

    virtual void publish_added(Entity entity, EventSystem& events) = 0;
    virtual void publish_removed(Entity entity, const SparseSetSwapInfo<Entity>& swap_info, EventSystem& events) = 0;
    virtual void publish_changed(Entity entity, EventSystem& events) = 0;
};

template <typename T> 
class ComponentStorage : public IComponentStorage
{
  public:
    bool add(Entity entity, T component);
    bool remove(Entity entity) override;
    bool remove(Entity entity, SparseSetSwapInfo<Entity>& swap_info) override;
    bool contains(Entity entity) const override;

    void clear() override;

    uint32_t get_index(Entity entity);

    T* get(Entity entity);
    const T* get(Entity entity) const;

    uint32_t size() const;

    Entity entity_at(uint32_t index) const;

    void publish_added(Entity entity, EventSystem& events) override;
    void publish_removed(Entity entity, const SparseSetSwapInfo<Entity>& swap_info, EventSystem& events) override;
    void publish_changed(Entity entity, EventSystem& events) override;

  private:
    SparseSetContainer<T, Entity> m_set;
};

} // namespace omega

#include "component_storage.tpp"

#endif // OMEGA_COMPONENT_STORAGE_HPP