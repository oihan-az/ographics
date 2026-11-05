#ifndef OMEGA_COMPONENT_EVENT_HPP
#define OMEGA_COMPONENT_EVENT_HPP

#include <omega/core/entity.hpp>

#include <typeindex>

namespace omega
{

enum class ComponentEventType
{
    Added,
    Removed,
    Changed
};

struct ComponentEvent
{
    Entity entity;
    std::type_index component_type;
    ComponentEventType type;
};

} // namespace omega

#endif // OMEGA_COMPONENT_EVENT_HPP