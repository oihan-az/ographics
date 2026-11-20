#ifndef OMEGA_EDITOR_CAMERA_VIEWPORT_HPP
#define OMEGA_EDITOR_CAMERA_VIEWPORT_HPP

#include <omega/core/entity/entity.hpp>

#include <imgui.h>

#include <array>

namespace omega
{

class ComponentSystem;

} // namespace omega

namespace omega_editor
{
  
enum class AspectRatioPreset
{
    Ratio16_9,
    Ratio16_10,
    Ratio4_3,
    Ratio21_9,
    Ratio32_9,
    Ratio1_1,
    Ratio9_16
};

struct AspectRatioPresetInfo
{
    AspectRatioPreset preset;
    const char* label;
    float ratio;
};

inline constexpr std::array<omega_editor::AspectRatioPresetInfo, 7> aspect_ratio_presets
{
    {
        {omega_editor::AspectRatioPreset::Ratio16_9, "16:9", 16.0f / 9.0f},
        {omega_editor::AspectRatioPreset::Ratio16_10, "16:10", 16.0f / 10.0f},
        {omega_editor::AspectRatioPreset::Ratio4_3, "4:3", 4.0f / 3.0f},
        {omega_editor::AspectRatioPreset::Ratio21_9, "21:9", 21.0f / 9.0f},
        {omega_editor::AspectRatioPreset::Ratio32_9, "32:9", 32.0f / 9.0f},
        {omega_editor::AspectRatioPreset::Ratio1_1, "1:1", 1.0f},
        {omega_editor::AspectRatioPreset::Ratio9_16, "9:16", 9.0f / 16.0f}
    }
};

struct ViewportRect
{
    ImVec2 position{0.0f, 0.0f};
    ImVec2 size{0.0f, 0.0f};
};

class CameraViewport
{
  public:
    CameraViewport(omega::ComponentSystem& components, omega::Entity camera_entity);

    void update(const ViewportRect& available);

    void set_aspect_ratio(AspectRatioPreset preset);
    AspectRatioPreset aspect_ratio_preset() const;
    float aspect_ratio() const;
    const char* aspect_ratio_label() const;

    void set_fullscreen(bool fullscreen);
    bool fullscreen() const;
    void toggle_fullscreen();

    const ViewportRect& rect() const;

  private:
    omega::ComponentSystem& m_components;
    omega::Entity m_camera_entity;

    ViewportRect m_rect;
    AspectRatioPreset m_aspect_ratio = AspectRatioPreset::Ratio16_9;
    bool m_fullscreen = false;
};

} // namespace omega_editor

#endif