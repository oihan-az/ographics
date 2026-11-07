#ifndef OMEGA_ENTITY_INDEXER_HPP
#define OMEGA_ENTITY_INDEXER_HPP

#include <omega/core/container/sparse_set.hpp>
#include <omega/core/entity/entity.hpp>

namespace omega
{

template <>
struct KeyIndexer<Entity>
{
    uint32_t operator()(const Entity& entity) const
    {
        return entity.index;
    }
};

} // namespace omega

#endif // OMEGA_ENTITY_HPP