#ifndef OMEGA_EDITOR_COMPONENT_EDITOR_REGISTRY_TPP
#define OMEGA_EDITOR_COMPONENT_EDITOR_REGISTRY_TPP

#include <omega/core/component/component_system.hpp>

#include <stdexcept>

namespace omega_editor
{

template <typename T> 
void ComponentEditorRegistry::register_component(std::string name, std::function<bool(T&)> draw)
{
    const std::type_index type = typeid(T);

    const auto it = std::find_if(m_editors.begin(), m_editors.end(), [type](const ComponentEditor& editor) { return editor.type == type; });
    if (it != m_editors.end())
        throw std::logic_error("Component editor already registered");

    ComponentEditor editor;
    editor.type = type;
    editor.name = std::move(name);

    editor.has = [](omega::Entity entity, omega::ComponentSystem& components) { return components.has<T>(entity); };
    editor.draw = [draw = std::move(draw)](omega::Entity entity, omega::ComponentSystem& components)
    {
        T* component = components.get<T>(entity);
        if (component && draw(*component))
            components.mark_dirty<T>(entity);
    };
    editor.create = [](omega::Entity entity, omega::ComponentSystem& components) { components.add<T>(entity, T{}); };
    editor.remove = [](omega::Entity entity, omega::ComponentSystem& components) { components.remove<T>(entity); };
    editor.prepare = [](omega::ComponentSystem& components) { components.get_storage<T>(); };

    m_editors.push_back(std::move(editor));
}

} // namespace omega_editor

#endif // OMEGA_EDITOR_COMPONENT_EDITOR_REGISTRY_TPP