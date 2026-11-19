#include <omega_editor/editor.hpp>

namespace omega_editor
{

Editor::Editor(omega::Engine& engine) : m_engine(engine) {}

void Editor::render()
{
    m_hierarchy_panel.render(m_engine);
}

} // namespace omega_editor