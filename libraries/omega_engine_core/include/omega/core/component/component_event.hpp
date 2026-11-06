#ifndef OMEGA_COMPONENT_EVENT_HPP
#define OMEGA_COMPONENT_EVENT_HPP

#include <omega/core/entity/entity.hpp>

#include <typeindex>

namespace omega
{

enum class ComponentEventType
{
    Added,
    Removed,
    Changed
};

template<typename T>
struct ComponentEvent
{
    Entity entity;
    ComponentEventType type;
};

} // namespace omega

#endif // OMEGA_COMPONENT_EVENT_HPP