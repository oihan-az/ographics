#ifndef OMEGA_EDITOR_COMPONENT_EDITOR_REGISTRY_HPP
#define OMEGA_EDITOR_COMPONENT_EDITOR_REGISTRY_HPP

#include <omega_editor/panels/component_editor.hpp>

#include <functional>
#include <vector>

namespace omega_editor
{

class ComponentEditorRegistry
{
  public:
    template <typename T> void register_component(std::string name, std::function<bool(T&)> draw);

    std::vector<ComponentEditor>& editors();
    const std::vector<ComponentEditor>& editors() const;

  private:
    std::vector<ComponentEditor> m_editors;
};

} // namespace omega_editor

#include "component_editor_registry.tpp"

#endif // OMEGA_EDITOR_PANEL_INSPECTION_HPP