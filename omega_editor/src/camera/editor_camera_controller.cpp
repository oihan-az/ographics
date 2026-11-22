#include <omega_editor/camera/editor_camera_controller.hpp>
#include <omega_editor/layout/camera_viewport.hpp>

#include <omega/core/math/angle.hpp>
#include <omega/core/math/quaternion.hpp>

#include <omega/camera/camera.hpp>
#include <omega/engine/engine.hpp>
#include <omega/transform/transform.hpp>

#include <ImGuizmo.h>
#include <imgui.h>

#include <algorithm>
#include <cmath>

namespace
{

constexpr omega::Vector3 world_up{0.0f, 1.0f, 0.0f};

omega::Vector3 camera_forward(const omega::Transform& transform)
{
    return transform.rotation * omega::Vector3{0.0f, 0.0f, -1.0f};
}

omega::Vector3 camera_right(const omega::Transform& transform)
{
    return transform.rotation * omega::Vector3{1.0f, 0.0f, 0.0f};
}

omega::Vector3 camera_up(const omega::Transform& transform)
{
    return transform.rotation * omega::Vector3{0.0f, 1.0f, 0.0f};
}

bool contains(const omega_editor::ViewportRect& rect, const ImVec2& point)
{
    return point.x >= rect.position.x && point.y >= rect.position.y && point.x < rect.position.x + rect.size.x &&
           point.y < rect.position.y + rect.size.y;
}

} // namespace

namespace omega_editor
{

void EditorCameraController::update(float delta_time, const CameraViewport& viewport,
                                    const EditorCameraControls& controls, omega::Engine& engine,
                                    omega::Entity editor_camera, omega::Entity selected_entity)
{
    // Only control the internal editor camera
    if (viewport.camera().entity != editor_camera)
    {
        m_mode = CameraNavigationMode::None;
        return;
    }

    auto& components = engine.components();
    auto* transform = components.get<omega::Transform>(editor_camera);
    if (!transform)
    {
        m_mode = CameraNavigationMode::None;
        return;
    }

    update_navigation_mode(viewport);

    const ImGuiIO& io = ImGui::GetIO();
    bool transform_changed = false;

    const bool can_focus = m_mode == CameraNavigationMode::None && contains(viewport.rect(), io.MousePos) &&
                                     !io.WantTextInput && !ImGui::IsAnyItemActive();

    if (can_focus && ImGui::IsKeyPressed(controls.focus, false))
    {
        transform_changed |= focus_entity(*transform, engine, selected_entity);
    }

    switch (m_mode)
    {
    case CameraNavigationMode::Fly:
    {
        transform_changed |= update_rotation(*transform, controls);
        transform_changed |= update_movement(*transform, controls, delta_time);

        // Adjusts the fly speed
        if (io.MouseWheel != 0.0f)
        {
            m_speed_multiplier = std::clamp(m_speed_multiplier * std::exp(io.MouseWheel * 0.15f), 0.1f, 100.0f);
        }

        break;
    }

    case CameraNavigationMode::Orbit:
        transform_changed |= update_orbit(*transform, controls);
        break;

    case CameraNavigationMode::Pan:
        transform_changed |= update_pan(*transform, viewport, controls, engine);
        break;

    case CameraNavigationMode::Dolly:
        transform_changed |= update_dolly(*transform, viewport, controls, engine, io.MouseDelta.y * 0.01f);
        break;

    case CameraNavigationMode::None:
        break;
    }

    // Wheel zoom, independently of mouse drag
    if (m_mode != CameraNavigationMode::Fly && io.MouseWheel != 0.0f && contains(viewport.rect(), io.MousePos) &&
        !ImGui::IsAnyItemHovered() && !ImGui::IsAnyItemActive())
    {
        transform_changed |= update_dolly(*transform, viewport, controls, engine, -io.MouseWheel * 0.15f);
    }

    if (transform_changed)
    {
        components.mark_dirty<omega::Transform>(editor_camera);
    }
}

bool EditorCameraController::is_navigating() const
{
    return m_mode != CameraNavigationMode::None;
}

void EditorCameraController::update_navigation_mode(const CameraViewport& viewport)
{
    const ImGuiIO& io = ImGui::GetIO();

    // Once a drag starts, keep its mode until the
    // corresponding mouse button is released
    if (m_mode != CameraNavigationMode::None)
    {
        const ImGuiMouseButton button = m_mode == CameraNavigationMode::Orbit ? ImGuiMouseButton_Left
                                        : m_mode == CameraNavigationMode::Pan ? ImGuiMouseButton_Middle
                                                                              : ImGuiMouseButton_Right;

        if (!ImGui::IsMouseDown(button))
            m_mode = CameraNavigationMode::None;

        return;
    }

    if (!contains(viewport.rect(), io.MousePos))
        return;

    if (io.WantTextInput || ImGui::IsAnyItemHovered() || ImGui::IsAnyItemActive() || ImGuizmo::IsUsing())
        return;

    // Alt navigation takes priority over regular navigation
    if (io.KeyAlt)
    {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            m_mode = CameraNavigationMode::Orbit;
        else if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle))
            m_mode = CameraNavigationMode::Pan;
        else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
            m_mode = CameraNavigationMode::Dolly;

        return;
    }

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        m_mode = CameraNavigationMode::Fly;
    else if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle))
        m_mode = CameraNavigationMode::Pan;
}

void EditorCameraController::initialize_pivot(const omega::Transform& transform)
{
    if (m_pivot_initialized)
        return;

    m_pivot = transform.position + camera_forward(transform) * m_orbit_distance;

    m_pivot_initialized = true;
}

bool EditorCameraController::focus_entity(omega::Transform& camera_transform, omega::Engine& engine,
                                          omega::Entity selected_entity)
{
    if (selected_entity == omega::Entity{})
        return false;

    if (!engine.components().has<omega::Transform>(selected_entity))
        return false;

    const glm::mat4 world_matrix = engine.transforms().get_world_matrix(selected_entity);

    const omega::Vector3 target = glm::vec3(world_matrix[3]);
    constexpr float focus_distance = 5.0f;

    m_pivot = target;
    m_orbit_distance = focus_distance;
    m_pivot_initialized = true;

    camera_transform.position = m_pivot - camera_forward(camera_transform) * m_orbit_distance;

    return true;
}

bool EditorCameraController::update_rotation(omega::Transform& transform, const EditorCameraControls& controls)
{
    const ImVec2 delta = ImGui::GetIO().MouseDelta;

    if (delta.x == 0.0f && delta.y == 0.0f)
        return false;

    const float yaw = omega::radians(-delta.x * controls.look_sensitivity);
    const float pitch = omega::radians(-delta.y * controls.look_sensitivity);

    const omega::Quaternion yaw_rotation = omega::angle_axis(yaw, world_up);
    omega::Quaternion rotation = omega::normalize(yaw_rotation * transform.rotation);
    const omega::Vector3 right = rotation * omega::Vector3{1.0f, 0.0f, 0.0f};
    const omega::Quaternion pitch_rotation = omega::angle_axis(pitch, right);

    // Prevent the camera from flipping at the poles
    const omega::Quaternion candidate = omega::normalize(pitch_rotation * rotation);
    const omega::Vector3 forward = candidate * omega::Vector3{0.0f, 0.0f, -1.0f};
    if (std::abs(omega::dot(forward, world_up)) < 0.995f)
        rotation = candidate;

    transform.rotation = rotation;

    // Maintain the pivot in front of the camera during free look
    m_pivot = transform.position + camera_forward(transform) * m_orbit_distance;

    return true;
}

bool EditorCameraController::update_movement(omega::Transform& transform, const EditorCameraControls& controls,
                                             float delta_time)
{
    omega::Vector3 direction{0.0f};

    if (ImGui::IsKeyDown(controls.forward))
        direction += camera_forward(transform);

    if (ImGui::IsKeyDown(controls.backward))
        direction -= camera_forward(transform);

    if (ImGui::IsKeyDown(controls.right))
        direction += camera_right(transform);

    if (ImGui::IsKeyDown(controls.left))
        direction -= camera_right(transform);

    if (ImGui::IsKeyDown(controls.up))
        direction += world_up;

    if (ImGui::IsKeyDown(controls.down))
        direction -= world_up;

    const float length = glm::length(direction);
    if (length < 0.0001f)
        return false;

    direction /= length;

    float speed = controls.movement_speed * m_speed_multiplier;
    if (ImGui::IsKeyDown(controls.boost) || ImGui::GetIO().KeyShift)
        speed *= controls.boost_multiplier;

    const omega::Vector3 movement = direction * speed * delta_time;
    transform.position += movement;

    if (m_pivot_initialized)
        m_pivot += movement;

    return true;
}

bool EditorCameraController::update_orbit(omega::Transform& transform, const EditorCameraControls& controls)
{
    const ImVec2 delta = ImGui::GetIO().MouseDelta;

    if (delta.x == 0.0f && delta.y == 0.0f)
        return false;

    initialize_pivot(transform);

    const float yaw = glm::radians(-delta.x * controls.orbit_sensitivity);
    const float pitch = glm::radians(-delta.y * controls.orbit_sensitivity);

    const omega::Quaternion yaw_rotation = glm::angleAxis(yaw, world_up);
    omega::Quaternion rotation = glm::normalize(yaw_rotation * transform.rotation);
    const omega::Vector3 right = rotation * omega::Vector3{1.0f, 0.0f, 0.0f};
    const omega::Quaternion pitch_rotation = glm::angleAxis(pitch, right);

    const omega::Quaternion candidate = glm::normalize(pitch_rotation * rotation);
    const omega::Vector3 forward = candidate * omega::Vector3{0.0f, 0.0f, -1.0f};
    if (std::abs(glm::dot(forward, world_up)) < 0.995f)
        rotation = candidate;

    transform.rotation = rotation;

    // Keep the camera at a constant distance from the pivot
    transform.position = m_pivot - camera_forward(transform) * m_orbit_distance;

    return true;
}

bool EditorCameraController::update_pan(omega::Transform& transform, const CameraViewport& viewport,
                                        const EditorCameraControls& controls, omega::Engine& engine)
{
    const ImVec2 delta = ImGui::GetIO().MouseDelta;

    if (delta.x == 0.0f && delta.y == 0.0f)
        return false;

    const float viewport_height = viewport.rect().size.y;

    if (viewport_height <= 0.0f)
        return false;

    initialize_pivot(transform);

    float visible_height = 0.0f;

    const auto selection = viewport.camera();
    auto& components = engine.components();
    if (selection.type == omega::CameraType::Perspective)
    {
        const auto* camera = components.get<omega::PerspectiveCamera>(selection.entity);

        if (!camera)
            return false;

        visible_height = 2.0f * m_orbit_distance * std::tan(camera->proj.field_of_view * 0.5f);
    }
    else
    {
        const auto* camera = components.get<omega::OrthogonalCamera>(selection.entity);

        if (!camera)
            return false;

        visible_height = std::abs(camera->proj.top - camera->proj.bottom);
    }

    const float units_per_pixel = visible_height / viewport_height * controls.pan_sensitivity;
    const omega::Vector3 movement = -camera_right(transform) * delta.x * units_per_pixel + camera_up(transform) * delta.y * units_per_pixel;

    transform.position += movement;
    m_pivot += movement;

    return true;
}

bool EditorCameraController::update_dolly(omega::Transform& transform, const CameraViewport& viewport,
                                          const EditorCameraControls& controls, omega::Engine& engine, float amount)
{
    if (amount == 0.0f)
        return false;

    initialize_pivot(transform);

    const float factor = std::exp(amount * controls.dolly_sensitivity);
    const auto selection = viewport.camera();
    if (selection.type == omega::CameraType::Perspective)
    {
        const float previous_distance = m_orbit_distance;

        m_orbit_distance = std::clamp(m_orbit_distance * factor, 0.1f, 10000.0f);
        if (m_orbit_distance == previous_distance)
            return false;

        transform.position = m_pivot - camera_forward(transform) * m_orbit_distance;
        return true;
    }

    auto& components = engine.components();

    auto* camera = components.get<omega::OrthogonalCamera>(selection.entity);
    if (!camera)
        return false;

    auto& proj = camera->proj;

    const float center_x = (proj.left + proj.right) * 0.5f;
    const float center_y = (proj.bottom + proj.top) * 0.5f;

    const float half_width = std::clamp(std::abs(proj.right - proj.left) * 0.5f * factor, 0.01f, 100000.0f);
    const float half_height = std::clamp(std::abs(proj.top - proj.bottom) * 0.5f * factor, 0.01f, 100000.0f);

    proj.left = center_x - half_width;
    proj.right = center_x + half_width;
    proj.bottom = center_y - half_height;
    proj.top = center_y + half_height;

    components.mark_dirty<omega::OrthogonalCamera>(selection.entity);

    return false;
}

} // namespace omega_editor