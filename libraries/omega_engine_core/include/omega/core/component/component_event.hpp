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

enum class ComponentModifiedOrigin
{
    Direct,
    Propagated
};

template <typename T> 
struct ComponentEvent
{
    ComponentEventType type;
    Entity entity;
    SparseSetSwapInfo<Entity> swap_info{};
    ComponentModifiedOrigin origin = ComponentModifiedOrigin::Direct;
};

} // namespace omega

#endif // OMEGA_COMPONENT_EVENT_HPP