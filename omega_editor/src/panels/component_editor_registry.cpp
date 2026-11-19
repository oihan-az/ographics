#include <omega_editor/panels/component_editor_registry.hpp>

namespace omega_editor
{

std::vector<ComponentEditor>& ComponentEditorRegistry::editors()
{
    return m_editors;
}

const std::vector<ComponentEditor>& ComponentEditorRegistry::editors() const
{
    return m_editors;
}

} // namespace omega_editor