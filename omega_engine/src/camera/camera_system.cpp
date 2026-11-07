#include <omega/camera/camera_system.hpp>
#include <omega/camera/matrix_camera.hpp>

namespace omega
{

CameraSystem::CameraSystem(Scene& scene, EventSystem& events, TransformSystem& transforms)
    : m_scene(scene), m_events(events), m_transforms(transforms)
{}

void CameraSystem::update() 
{
    // Process perspective camera events
    const auto& perspective_events = m_events.get_queue<ComponentEvent<PerspectiveCamera>>().get_events();
    for (const auto& event : perspective_events)
    {
        switch (event.type)
        {
        case ComponentEventType::Added:
        case ComponentEventType::Changed:
            invalidate(event.entity);
            break;
        case ComponentEventType::Removed:
            m_cache.erase(event.entity);
            break;
        }
    }

    // Process orthogonal camera events
    const auto& orthogonal_events = m_events.get_queue<ComponentEvent<OrthogonalCamera>>().get_events();
    for (const auto& event : orthogonal_events)
    {
        switch (event.type)
        {
        case ComponentEventType::Added:
        case ComponentEventType::Changed:
            invalidate(event.entity);
            break;
        case ComponentEventType::Removed:
            m_cache.erase(event.entity);
            break;
        }
    }

    // Process world transform events
    const auto& transform_events = m_events.get_queue<WorldTransformChangedEvent>().get_events();
    for (const auto& event : transform_events)
    {
        invalidate(event.entity);
    }
}

CameraData CameraSystem::get_camera_data(Entity entity)
{
    // Return default data for entities that are no longer alive
    if (!m_scene.is_alive(entity))
        return CameraData{};

    const PerspectiveCamera* perspective_camera = m_scene.get<PerspectiveCamera>(entity);
    const OrthogonalCamera* orthogonal_camera = m_scene.get<OrthogonalCamera>(entity);

    // A camera entity must have exactly one camera component
    if ((perspective_camera == nullptr) == (orthogonal_camera == nullptr))
        return CameraData{};

    // Reuse cached data when it is still valid
    if (!m_cache.is_dirty(entity))
        return *m_cache.get(entity);

    // Calculate the view matrix from the entity's world transform
    Matrix4 world_matrix = m_transforms.get_world_matrix(entity);
    Matrix4 view_matrix = inverse(world_matrix);

    // Calculate the appropriate projection matrix
    Matrix4 projection_matrix;
    if (perspective_camera)
        projection_matrix = mtx_perspective_projection(*perspective_camera);
    else
        projection_matrix = mtx_orthogonal_projection(*orthogonal_camera);

    // Calculate the view-projection matrix
    CameraData data;
    data.view = view_matrix;
    data.projection = projection_matrix;
    data.view_projection = projection_matrix * view_matrix;

    // Update the cache
    m_cache.set(entity, data);

    return data;
}

void CameraSystem::invalidate(Entity entity)
{
    m_cache.invalidate(entity);
}

} // namespace omega
