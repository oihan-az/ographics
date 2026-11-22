#ifndef OMEGA_EDITOR_UI_ICONS_HPP
#define OMEGA_EDITOR_UI_ICONS_HPP

#include <imgui.h>

namespace omega_editor::icons
{

void draw_fullscreen(ImDrawList* draw, ImVec2 center, bool fullscreen, ImU32 color);
void draw_chevron(ImDrawList* draw, ImVec2 center, bool down, ImU32 color);
void draw_camera(ImDrawList* draw_list, ImVec2 position, float size, ImU32 color);
void draw_translate(ImDrawList* draw_list, ImVec2 position, float size, ImU32 color);
void draw_rotate(ImDrawList* draw_list, ImVec2 position, float size, ImU32 color);
void draw_scale(ImDrawList* draw_list, ImVec2 position, float size, ImU32 color);


} // namespace omega_editor::icons

#endif // OMEGA_EDITOR_UI_ICONS_HPP