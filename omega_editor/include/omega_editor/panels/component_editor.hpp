#ifndef OMEGA_EDITOR_COMPONENT_EDITOR_HPP
#define OMEGA_EDITOR_COMPONENT_EDITOR_HPP

#include <omega/core/entity/entity.hpp>

#include <typeindex>
#include <string>
#include <functional>

namespace omega
{

class ComponentSystem;

} // namespace omega

namespace omega_editor
{

struct ComponentEditor
{
    std::type_index type{typeid(void)};
    std::string name;

    std::function<bool(omega::Entity, omega::ComponentSystem&)> has;
    std::function<void(omega::Entity, omega::ComponentSystem&)> draw;
    std::function<void(omega::Entity, omega::ComponentSystem&)> create;
    std::function<void(omega::Entity, omega::ComponentSystem&)> remove;
};

} // namespace omega_editor

#endif // OMEGA_EDITOR_COMPONENT_EDITOR_HPP