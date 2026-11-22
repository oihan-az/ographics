#include <omega_editor/icons/icons.hpp>

#include <algorithm>

namespace omega_editor::icons
{

void draw_fullscreen(ImDrawList* draw, ImVec2 center, bool fullscreen, ImU32 color)
{
    const float x = center.x;
    const float y = center.y;

    constexpr float s = 6.0f;
    constexpr float t = 1.5f;

    if (!fullscreen)
    {
        // Maximize: outward corners
        draw->AddLine({x - s, y - 2}, {x - s, y - s}, color, t);
        draw->AddLine({x - s, y - s}, {x - 2, y - s}, color, t);

        draw->AddLine({x + 2, y - s}, {x + s, y - s}, color, t);
        draw->AddLine({x + s, y - s}, {x + s, y - 2}, color, t);

        draw->AddLine({x - s, y + 2}, {x - s, y + s}, color, t);
        draw->AddLine({x - s, y + s}, {x - 2, y + s}, color, t);

        draw->AddLine({x + 2, y + s}, {x + s, y + s}, color, t);
        draw->AddLine({x + s, y + s}, {x + s, y + 2}, color, t);
    }
    else
    {
        // Restore: inward corners
        draw->AddLine({x - s, y - 2}, {x - 2, y - 2}, color, t);
        draw->AddLine({x - 2, y - s}, {x - 2, y - 2}, color, t);

        draw->AddLine({x + 2, y - 2}, {x + s, y - 2}, color, t);
        draw->AddLine({x + 2, y - s}, {x + 2, y - 2}, color, t);

        draw->AddLine({x - s, y + 2}, {x - 2, y + 2}, color, t);
        draw->AddLine({x - 2, y + 2}, {x - 2, y + s}, color, t);

        draw->AddLine({x + 2, y + 2}, {x + s, y + 2}, color, t);
        draw->AddLine({x + 2, y + 2}, {x + 2, y + s}, color, t);
    }
}

void draw_chevron(ImDrawList* draw, ImVec2 center, bool down, ImU32 color)
{
    const float direction = down ? 1.0f : -1.0f;

    draw->AddLine({center.x - 4.0f, center.y - 2.0f * direction}, {center.x, center.y + 2.0f * direction}, color, 1.5f);

    draw->AddLine({center.x, center.y + 2.0f * direction}, {center.x + 4.0f, center.y - 2.0f * direction}, color, 1.5f);
}

void draw_camera(ImDrawList* draw_list, ImVec2 position, float size, ImU32 color)
{
    const auto point = [&](float x, float y) -> ImVec2 { return {position.x + x * size, position.y + y * size}; };

    const float thickness = std::max(1.0f, size * 0.085f);

    // Camera body
    draw_list->AddRect(point(0.12f, 0.28f), point(0.88f, 0.82f), color, size * 0.08f, 0, thickness);

    // Top housing
    draw_list->AddLine(point(0.29f, 0.28f), point(0.39f, 0.17f), color, thickness);

    draw_list->AddLine(point(0.39f, 0.17f), point(0.65f, 0.17f), color, thickness);

    draw_list->AddLine(point(0.65f, 0.17f), point(0.75f, 0.28f), color, thickness);

    // Lens
    draw_list->AddCircle(point(0.50f, 0.55f), size * 0.17f, color, 24, thickness);
}

} // namespace omega_editor::icons