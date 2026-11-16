#ifndef OMEGA_COMPONENT_EVENT_HPP
#define OMEGA_COMPONENT_EVENT_HPP

#include <omega/core/entity/entity.hpp>
#include <omega/core/container/sparse_set.hpp>

namespace omega
{

enum class ComponentEventType
{
    Added,
    Modified,
    Removed
};

template <typename T> 
struct ComponentEvent
{
    ComponentEventType type;
    Entity entity;
    SparseSetSwapInfo<Entity> swap_info{};
};

} // namespace omega

#endif // OMEGA_COMPONENT_EVENT_HPP