#ifndef OMEGA_COMPONENT_EVENT_HPP
#define OMEGA_COMPONENT_EVENT_HPP

#include <omega/core/entity/entity.hpp>
#include <omega/core/container/sparse_set.hpp>

namespace omega
{

template <typename T> struct ComponentAddedEvent
{
    Entity entity;
};

template <typename T> struct ComponentModifiedEvent
{
    Entity entity;
};

template <typename T> struct ComponentRemovedEvent
{
    Entity entity;
    SparseSetSwapInfo<Entity> swap_info;
};

} // namespace omega

#endif // OMEGA_COMPONENT_EVENT_HPP