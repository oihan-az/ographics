#ifndef OMEGA_EDITOR_PANEL_SLOT_HPP
#define OMEGA_EDITOR_PANEL_SLOT_HPP

namespace omega_editor
{

enum class PanelAnchor
{
    Left,
    Right,
    Top,
    Bottom,
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight
};

struct PanelDimension
{
    float size = 0.20f;
    float min = 0.15f;
    float max = 0.35f;
};

struct PanelSize
{
    PanelDimension width;
    PanelDimension height;
};

struct PanelBounds
{
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

class PanelSlot
{
  public:
    PanelSlot(PanelAnchor anchor, PanelSize size);

    bool begin(const char* title);
    void end();

    PanelAnchor anchor() const;

    float width_ratio() const;
    float height_ratio() const;

    const PanelBounds& bounds() const;

  private:
    PanelAnchor m_anchor;
    PanelSize m_size;
    PanelBounds m_bounds;
};

} // namespace omega_editor

#endif // OMEGA_EDITOR_PANEL_SLOT_HPP