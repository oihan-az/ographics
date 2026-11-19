#ifndef OMEGA_EDITOR_HPP
#define OMEGA_EDITOR_HPP

#include <omega_editor/panels/hierarchy_panel.hpp>

namespace omega
{

class Engine;

} // namespace omega

namespace omega_editor
{

class Editor
{
  public:
    Editor(omega::Engine& engine);
    void render();

  private:
    omega::Engine& m_engine;

    HierarchyPanel m_hierarchy_panel;
};

} // namespace omega_editor

#endif // OMEGA_EDITOR_HPP