#ifndef OMEGA_HIERARCHY_EVENT_HPP
#define OMEGA_HIERARCHY_EVENT_HPP

#include <omega/core/entity/entity.hpp>

namespace omega
{

struct HierarchyParentChangedEvent
{
    Entity child;
    Entity parent;
};

struct HierarchyModifiedEvent
{
    Entity entity;
};

} // namespace omega

#endif // OMEGA_HIERARCHY_EVENT_HPP