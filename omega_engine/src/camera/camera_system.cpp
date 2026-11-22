#include <omega/camera/camera_system.hpp>
#include <omega/camera/matrix_camera.hpp>

#include <omega/core/event/event_system.hpp>
#include <omega/core/component/component_event.hpp>
#include <omega/core/component/component_system.hpp>
#include <omega/core/hierarchy/hierarchy_event.hpp>
#include <omega/core/hierarchy/hierarchy_system.hpp>

namespace omega
{

CameraSystem::CameraSystem(EventSystem& events, ComponentSystem& components, 
                           HierarchySystem& hierarchy, EntitySystem& entities,
                           TransformSystem& transforms)
    : m_events(events), m_components(components), m_hierarchy(hierarchy), m_entities(entities), m_transforms(transforms)
{
}

void CameraSystem::update() 
{
    // Process perspective camera events
    const auto& perspective_events = m_events.get_queue<ComponentEvent<PerspectiveCamera>>().get_events();
    for (const auto& event : perspective_events)
    {
        invalidate(event.entity);

        switch (event.type)
        {
        case ComponentEventType::Added:
        case ComponentEventType::Modified:
            break;

        case ComponentEventType::Removed:
            m_perspective_cache.handle_swap(event.swap_info.exchanged, event.swap_info.old_index, event.swap_info.new_index);
            break;
        }
    }

    // Process orthogonal camera events
    const auto& orthogonal_events = m_events.get_queue<ComponentEvent<OrthogonalCamera>>().get_events();
    for (const auto& event : orthogonal_events)
    {
        invalidate(event.entity);

        switch (event.type)
        {
        case ComponentEventType::Added:
        case ComponentEventType::Modified:
            break;

        case ComponentEventType::Removed:
            m_orthogonal_cache.handle_swap(event.swap_info.exchanged, event.swap_info.old_index, event.swap_info.new_index);
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

CameraData CameraSystem::get_camera_data(Entity entity, CameraType type)
{
    if (!m_entities.is_alive(entity))
        return CameraData{};

    uint32_t dense_idx = 0;
    Matrix4 projection_matrix{1.0f};

    DenseIndexedCache<CameraData>* cache = nullptr;

    switch (type)
    {
    case CameraType::Perspective:
    {
        const auto* camera = m_components.get<PerspectiveCamera>(entity);

        if (!camera)
            return CameraData{};

        auto& storage = m_components.get_storage<PerspectiveCamera>();

        dense_idx = storage.get_index(entity);
        cache = &m_perspective_cache;

        if (!cache->is_dirty(dense_idx))
            return *cache->get(dense_idx);

        projection_matrix = mtx_perspective_projection(*camera);
        break;
    }

    case CameraType::Orthographic:
    {
        const auto* camera = m_components.get<OrthogonalCamera>(entity);

        if (!camera)
            return CameraData{};

        auto& storage = m_components.get_storage<OrthogonalCamera>();

        dense_idx = storage.get_index(entity);
        cache = &m_orthogonal_cache;

        if (!cache->is_dirty(dense_idx))
            return *cache->get(dense_idx);

        projection_matrix = mtx_orthogonal_projection(*camera);
        break;
    }

    default:
        return CameraData{};
    }

    // Calculate the view matrix from the entity's world transform
    const Matrix4 world_matrix = m_transforms.get_world_matrix(entity);
    const Matrix4 view_matrix = inverse(world_matrix);

    CameraData data;
    data.view = view_matrix;
    data.projection = projection_matrix;
    data.view_projection = projection_matrix * view_matrix;

    cache->set(dense_idx, data);

    return data;
}

void CameraSystem::invalidate(Entity entity, CameraType type)
{
    switch (type)
    {
    case CameraType::Perspective:
    {
        if (!m_components.has<PerspectiveCamera>(entity))
            break;

        auto& storage = m_components.get_storage<PerspectiveCamera>();
        m_perspective_cache.invalidate(storage.get_index(entity));
        break;
    }

    case CameraType::Orthographic:
    {
        if (!m_components.has<OrthogonalCamera>(entity))
            break;

        auto& storage = m_components.get_storage<OrthogonalCamera>();
        m_orthogonal_cache.invalidate(storage.get_index(entity));
        break;
    }
    }
}

void CameraSystem::invalidate(Entity entity)
{
    invalidate(entity, CameraType::Perspective);
    invalidate(entity, CameraType::Orthographic);
}

} // namespace omega
