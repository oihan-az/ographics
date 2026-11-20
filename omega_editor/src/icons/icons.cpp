#include <omega_editor/icons/icons.hpp>

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

} // namespace omega_editor::icons