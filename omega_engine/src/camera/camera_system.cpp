#include <omega/camera/camera_system.hpp>
#include <omega/camera/matrix_camera.hpp>

namespace omega
{

CameraSystem::CameraSystem(Scene& scene, EventSystem& events, TransformSystem& transforms)
    : m_scene(scene), m_events(events), m_transforms(transforms)
{
    m_perspective_camera_subscription = m_events.subscribe<ComponentEvent<PerspectiveCamera>>
    (
        [this](const ComponentEvent<PerspectiveCamera>& event) 
        { 
                on_perspective_camera_event(event); 
        }
    );

    m_orthogonal_camera_subscription = m_events.subscribe<ComponentEvent<OrthogonalCamera>>
    (
        [this](const ComponentEvent<OrthogonalCamera>& event) 
        { 
            on_orthogonal_camera_event(event); 
        }
    );

    m_world_transform_subscription = m_events.subscribe<WorldTransformChangedEvent>
    (
        [this](const WorldTransformChangedEvent& event) 
        { 
            on_world_transform_changed(event); 
        }
    );
}

CameraSystem::~CameraSystem()
{
    m_events.unsubscribe(m_perspective_camera_subscription);
    m_events.unsubscribe(m_orthogonal_camera_subscription);
    m_events.unsubscribe(m_world_transform_subscription);
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

void CameraSystem::on_perspective_camera_event(const ComponentEvent<PerspectiveCamera>& event)
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

void CameraSystem::on_orthogonal_camera_event(const ComponentEvent<OrthogonalCamera>& event)
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

void CameraSystem::on_world_transform_changed(const WorldTransformChangedEvent& event)
{
    invalidate(event.entity);
}

} // namespace omega
