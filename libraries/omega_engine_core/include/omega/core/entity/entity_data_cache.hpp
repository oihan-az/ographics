#ifndef OMEGA_ENTITY_DATA_CACHE_HPP
#define OMEGA_ENTITY_DATA_CACHE_HPP

#include <omega/core/entity/entity.hpp>
#include <omega/core/entity/entity_indexer.hpp>
#include <omega/core/container/sparse_set.hpp>

#include <unordered_map>

namespace omega
{

template <typename T> 
class EntityDataCache
{
  public:
    bool contains(Entity entity) const;
    bool is_dirty(Entity entity) const;

    T* get(Entity entity);
    const T* get(Entity entity) const;

    void set(Entity entity, T data);
    void invalidate(Entity entity);

    void erase(Entity entity);
    void clear();

  private:
    struct CacheEntry
    {
        T data{};
        bool dirty = true;
    };

    SparseSetContainer<CacheEntry, Entity> m_set;
};

} // namespace omega

#include "entity_data_cache.tpp"

#endif // OMEGA_ENTITY_DATA_CACHE_HPP