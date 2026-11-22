#include <omega_editor/icons/icons.hpp>

#include <algorithm>
#include <cmath>

namespace
{

struct IconPath
{
    ImDrawList* draw_list;
    ImVec2 origin;
    float scale;
    float thickness;
    ImU32 color;

    ImVec2 first{};
    ImVec2 last{};

    IconPath(ImDrawList* list, ImVec2 position, float size, ImU32 col)
        : draw_list(list), origin(position), scale(size / 24.0f), thickness(2.0f * scale), color(col)
    {
    }

    ImVec2 point(float x, float y) const
    {
        return {origin.x + x * scale, origin.y + y * scale};
    }

    void move(float x, float y)
    {
        draw_list->PathClear();
        first = last = point(x, y);
        draw_list->PathLineTo(first);
    }

    void line(float x, float y)
    {
        last = point(x, y);
        draw_list->PathLineTo(last);
    }

    void bezier(float x1, float y1, float x2, float y2, float x3, float y3)
    {
        last = point(x3, y3);

        draw_list->PathBezierCubicCurveTo(point(x1, y1), point(x2, y2), last);
    }

    void stroke()
    {
        draw_list->PathStroke(color, 0, thickness);

        // Rounded line caps
        const float radius = thickness * 0.5f;

        draw_list->AddCircleFilled(first, radius, color, 8);

        draw_list->AddCircleFilled(last, radius, color, 8);
    }
};

} // namespace

namespace omega_editor::icons
{

void draw_fullscreen(ImDrawList* draw, ImVec2 center, bool fullscreen, ImU32 color)
{
    constexpr float size = 18.0f;

    const ImVec2 origin{center.x - size * 0.5f, center.y - size * 0.5f};

    IconPath path(draw, origin, size, color);

    if (!fullscreen)
    {
        // Maximize: expand toward the four corners

        // Top-left
        path.move(8, 3);
        path.line(3, 3);
        path.line(3, 8);
        path.stroke();

        // Top-right
        path.move(16, 3);
        path.line(21, 3);
        path.line(21, 8);
        path.stroke();

        // Bottom-left
        path.move(3, 16);
        path.line(3, 21);
        path.line(8, 21);
        path.stroke();

        // Bottom-right
        path.move(16, 21);
        path.line(21, 21);
        path.line(21, 16);
        path.stroke();
    }
    else
    {
        // Minimize: contract toward the center

        // Top-left
        path.move(3, 8);
        path.line(8, 8);
        path.line(8, 3);
        path.stroke();

        // Top-right
        path.move(16, 3);
        path.line(16, 8);
        path.line(21, 8);
        path.stroke();

        // Bottom-left
        path.move(3, 16);
        path.line(8, 16);
        path.line(8, 21);
        path.stroke();

        // Bottom-right
        path.move(16, 21);
        path.line(16, 16);
        path.line(21, 16);
        path.stroke();
    }
}

void draw_chevron(ImDrawList* draw, ImVec2 center, bool down, ImU32 color)
{
    constexpr float size = 16.0f;

    const ImVec2 origin{center.x - size * 0.5f, center.y - size * 0.5f};

    IconPath path(draw, origin, size, color);

    if (down)
    {
        // Chevron Down
        path.move(6, 9);
        path.line(12, 15);
        path.line(18, 9);
    }
    else
    {
        // Chevron Up
        path.move(18, 15);
        path.line(12, 9);
        path.line(6, 15);
    }

    path.stroke();
}

void draw_camera(ImDrawList* draw_list, ImVec2 position, float size, ImU32 color)
{
    IconPath path(draw_list, position, size, color);

    // Camera body
    path.move(3.0f, 7.0f);
    path.line(21.0f, 7.0f);
    path.line(21.0f, 20.0f);
    path.line(3.0f, 20.0f);
    path.line(3.0f, 7.0f);
    path.stroke();

    // Top housing
    path.move(7.0f, 7.0f);
    path.line(9.4f, 4.4f);
    path.line(15.6f, 4.4f);
    path.line(18.0f, 7.0f);
    path.stroke();

    // Lens
    draw_list->AddCircle(path.point(12.0f, 13.2f), 4.0f * path.scale, color, 32, path.thickness);
}

void draw_translate(ImDrawList* draw_list, ImVec2 position, float size, ImU32 color)
{
    IconPath path(draw_list, position, size, color);

    // Main vertical and horizontal axes
    path.move(5, 3);
    path.line(5, 19);
    path.line(21, 19);
    path.stroke();

    // Depth axis
    path.move(5, 19);
    path.line(11, 13);
    path.stroke();

    // Vertical arrowhead
    path.move(2, 6);
    path.line(5, 3);
    path.line(8, 6);
    path.stroke();

    // Horizontal arrowhead
    path.move(18, 16);
    path.line(21, 19);
    path.line(18, 22);
    path.stroke();
}

void draw_rotate(ImDrawList* draw_list, ImVec2 position, float size, ImU32 color)
{
    IconPath path(draw_list, position, size, color);

    // Vertical elliptical orbit
    path.move(16.472f, 7.528f);

    path.bezier(15.313f, 2.891f, 12.574f, 0.795f, 10.163f, 2.699f);

    path.bezier(7.752f, 4.604f, 6.469f, 9.878f, 7.207f, 14.848f);

    path.bezier(7.945f, 19.818f, 10.460f, 22.835f, 13.000f, 21.798f);

    path.stroke();

    // Horizontal elliptical orbit
    path.move(21.798f, 11.000f);

    path.bezier(20.883f, 8.760f, 17.046f, 7.115f, 12.478f, 7.006f);

    path.bezier(7.910f, 6.896f, 3.776f, 8.350f, 2.438f, 10.537f);

    path.bezier(1.099f, 12.723f, 2.898f, 15.085f, 6.805f, 16.272f);

    path.bezier(10.713f, 17.460f, 15.734f, 17.171f, 19.000f, 15.571f);

    path.stroke();

    // Rotation arrowhead
    path.move(15.194f, 13.707f);
    path.line(19.008f, 15.567f);
    path.line(17.148f, 19.381f);
    path.stroke();
}

void draw_scale(ImDrawList* draw_list, ImVec2 position, float size, ImU32 color)
{
    IconPath path(draw_list, position, size, color);

    // Outer resize boundary with rounded corners
    path.move(12, 3);
    path.line(5, 3);

    path.bezier(3.895f, 3.000f, 3.000f, 3.895f, 3.000f, 5.000f);

    path.line(3, 19);

    path.bezier(3.000f, 20.105f, 3.895f, 21.000f, 5.000f, 21.000f);

    path.line(19, 21);

    path.bezier(20.105f, 21.000f, 21.000f, 20.105f, 21.000f, 19.000f);

    path.line(21, 12);
    path.stroke();

    // Inner resize corner
    path.move(14, 15);
    path.line(9, 15);
    path.line(9, 10);
    path.stroke();

    // Upper-right expansion corner
    path.move(16, 3);
    path.line(21, 3);
    path.line(21, 8);
    path.stroke();

    // Diagonal resize direction
    path.move(21, 3);
    path.line(9, 15);
    path.stroke();
}

} // namespace omega_editor::icons