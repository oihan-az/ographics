#ifndef OMEGA_COMPONENT_STORAGE_HPP
#define OMEGA_COMPONENT_STORAGE_HPP

#include <omega/core/entity/entity.hpp>

#include <cstdint>
#include <vector>

namespace omega
{

class IComponentStorage
{
  public:
    virtual ~IComponentStorage() = default;

    virtual void clear() = 0;
    virtual bool remove(Entity entity) = 0;
    virtual bool contains(Entity entity) const = 0;
};

template <typename T> 
class ComponentStorage : public IComponentStorage
{
  public:
    bool add(Entity entity, T component);
    bool remove(Entity entity) override;

    bool contains(Entity entity) const override;

    void clear() override;

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