#include <omega_editor/layout/camera_viewport.hpp>

#include <omega/core/component/component_system.hpp>
#include <omega/camera/camera.hpp>

#include <algorithm>
#include <cmath>

namespace omega_editor
{

CameraViewport::CameraViewport(omega::ComponentSystem& components)
    : m_components(components)
{
}

void CameraViewport::set_camera(omega::Entity camera_entity)
{
    m_camera_entity = camera_entity;
}

void CameraViewport::update(const ViewportRect& available)
{
    const float width = available.size.x;
    const float height = available.size.y;

    if (width <= 0.0f || height <= 0.0f)
    {
        m_rect = {available.position, {0.0f, 0.0f}};
        return;
    }

    const float ratio = aspect_ratio();

    const float fitted_width = std::min(width, height * ratio);
    const float fitted_height = fitted_width / ratio;

    m_rect.size = {fitted_width, fitted_height};
    m_rect.position = {available.position.x + (width - fitted_width) * 0.5f,
                       available.position.y + (height - fitted_height) * 0.5f};
}

void CameraViewport::set_aspect_ratio(AspectRatioPreset preset)
{
    if (m_aspect_ratio == preset)
        return;

    m_aspect_ratio = preset;

    if (!m_components.has<omega::PerspectiveCamera>(m_camera_entity))
        return;

    auto camera = m_components.get<omega::PerspectiveCamera>(m_camera_entity);
    camera->proj.aspect_ratio = aspect_ratio();

    m_components.mark_dirty<omega::PerspectiveCamera>(m_camera_entity);
}

AspectRatioPreset CameraViewport::aspect_ratio_preset() const
{
    return m_aspect_ratio;
}

float CameraViewport::aspect_ratio() const
{
    for (const auto& info : aspect_ratio_presets)
    {
        if (info.preset == m_aspect_ratio)
            return info.ratio;
    }

    return 16.0f / 9.0f;
}

const char* CameraViewport::aspect_ratio_label() const
{
    for (const auto& info : aspect_ratio_presets)
    {
        if (info.preset == m_aspect_ratio)
            return info.label;
    }

    return "16:9";
}

void CameraViewport::set_fullscreen(bool fullscreen)
{
    m_fullscreen = fullscreen;
}

bool CameraViewport::fullscreen() const
{
    return m_fullscreen;
}

void CameraViewport::toggle_fullscreen()
{
    m_fullscreen = !m_fullscreen;
}

const ViewportRect& CameraViewport::rect() const
{
    return m_rect;
}

} // namespace omega_editor