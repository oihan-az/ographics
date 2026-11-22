#ifndef OMEGA_EDITOR_CAMERA_CONTROLS_HPP
#define OMEGA_EDITOR_CAMERA_CONTROLS_HPP

#include <imgui.h>

namespace omega_editor
{

struct EditorCameraControls
{
    // Movement
    float movement_speed = 5.0f;
    float boost_multiplier = 3.0f;

    // Rotation
    float look_sensitivity = 0.15f;
    float orbit_sensitivity = 0.2f;

    // Translation and zoom
    float pan_sensitivity = 1.0f;
    float dolly_sensitivity = 1.0f;

    // Mouse and keyboard bindings
    static constexpr ImGuiMouseButton look_button = ImGuiMouseButton_Right;
    static constexpr ImGuiMouseButton orbit_button = ImGuiMouseButton_Left;
    static constexpr ImGuiMouseButton pan_button = ImGuiMouseButton_Middle;
    static constexpr ImGuiMouseButton dolly_button = ImGuiMouseButton_Right;

    static constexpr ImGuiKey forward = ImGuiKey_W;
    static constexpr ImGuiKey backward = ImGuiKey_S;
    static constexpr ImGuiKey left = ImGuiKey_A;
    static constexpr ImGuiKey right = ImGuiKey_D;
    static constexpr ImGuiKey down = ImGuiKey_Q;
    static constexpr ImGuiKey up = ImGuiKey_E;

    static constexpr ImGuiKey boost = ImGuiKey_LeftShift;
    static constexpr ImGuiKey focus = ImGuiKey_F;
};

} // namespace omega_editor

#endif // OMEGA_EDITOR_CAMERA_CONTROLS_HPP