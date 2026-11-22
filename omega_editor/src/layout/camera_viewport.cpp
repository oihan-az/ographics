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

void CameraViewport::set_camera(CameraSelection camera)
{
    m_camera = camera;
}

CameraSelection CameraViewport::camera() const
{
    return m_camera;
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
    if (m_camera.type != omega::CameraType::Perspective)
        return;

    if (!m_components.has<omega::PerspectiveCamera>(m_camera.entity))
        return;

    const auto it = std::find_if(aspect_ratio_presets.begin(), aspect_ratio_presets.end(),
                                 [preset](const auto& info) { return info.preset == preset; });

    if (it == aspect_ratio_presets.end())
        return;

    auto camera = m_components.get<omega::PerspectiveCamera>(m_camera.entity);

    if (camera->proj.aspect_ratio == it->ratio)
        return;

    camera->proj.aspect_ratio = it->ratio;

    m_components.mark_dirty<omega::PerspectiveCamera>(m_camera.entity);
}

std::optional<AspectRatioPreset> CameraViewport::aspect_ratio_preset() const
{
    constexpr float epsilon = 0.0001f;

    const float ratio = aspect_ratio();
    for (const auto& info : aspect_ratio_presets)
    {
        if (std::abs(ratio - info.ratio) < epsilon)
            return info.preset;
    }

    return std::nullopt;
}

float CameraViewport::aspect_ratio() const
{
    switch (m_camera.type)
    {
    case omega::CameraType::Perspective:
    {
        if (!m_components.has<omega::PerspectiveCamera>(m_camera.entity))
            break;

        const auto camera = m_components.get<omega::PerspectiveCamera>(m_camera.entity);

        return camera->proj.aspect_ratio;
    }

    case omega::CameraType::Orthographic:
    {
        if (!m_components.has<omega::OrthogonalCamera>(m_camera.entity))
            break;

        const auto camera = m_components.get<omega::OrthogonalCamera>(m_camera.entity);
        const auto& proj = camera->proj;

        const float width = proj.right - proj.left;
        const float height = proj.top - proj.bottom;

        if (std::abs(height) < 0.0001f)
            break;

        return std::abs(width / height);
    }
    }

    return 16.0f / 9.0f;
}

const char* CameraViewport::aspect_ratio_label() const
{
    const auto preset = aspect_ratio_preset();
    if (!preset)
        return "Custom";

    for (const auto& info : aspect_ratio_presets)
    {
        if (info.preset == *preset)
            return info.label;
    }

    return "Custom";
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