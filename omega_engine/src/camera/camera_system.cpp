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
    : m_events(events), m_components(components), m_hierarchy(hierarchy), m_entitites(entities), m_transforms(transforms)
{
}

void CameraSystem::update() 
{
    auto& perspective_storage = m_components.get_storage<PerspectiveCamera>();
    auto& orthogonal_storage = m_components.get_storage<OrthogonalCamera>();

    // Process added
    const auto& perspective_added = m_events.get_queue<ComponentAddedEvent<PerspectiveCamera>>().get_events();
    if (!perspective_added.empty())
    {
        for (auto it = perspective_added.rbegin(); it != perspective_added.rend(); ++it)
        {
            invalidate(it->entity);
        }
    }

    const auto& orthogonal_added = m_events.get_queue<ComponentAddedEvent<OrthogonalCamera>>().get_events();
    if (!orthogonal_added.empty())
    {
        for (auto it = orthogonal_added.rbegin(); it != orthogonal_added.rend(); ++it)
        {
            invalidate(it->entity);
        }
    }

    // Process modified
    const auto& perspective_modified = m_events.get_queue<ComponentModifiedEvent<PerspectiveCamera>>().get_events();
    if (!perspective_modified.empty())
    {
        for (auto it = perspective_modified.rbegin(); it != perspective_modified.rend(); ++it)
        {
            invalidate(it->entity);
        }
    }

    const auto& orthogonal_modified = m_events.get_queue<ComponentModifiedEvent<OrthogonalCamera>>().get_events();
    if (!orthogonal_modified.empty())
    {
        for (auto it = orthogonal_modified.rbegin(); it != orthogonal_modified.rend(); ++it)
        {
            invalidate(it->entity);
        }
    }

    // Process removed
    const auto& perspective_removed = m_events.get_queue<ComponentRemovedEvent<PerspectiveCamera>>().get_events();
    if (!perspective_removed.empty())
    {
        for (auto it = perspective_removed.rbegin(); it != perspective_removed.rend(); ++it)
        {
            invalidate(it->entity);
            m_perspective_cache.handle_swap(it->swap_info.exchanged, it->swap_info.old_index, it->swap_info.new_index);
        }
    }

    const auto& orthogonal_removed = m_events.get_queue<ComponentRemovedEvent<OrthogonalCamera>>().get_events();
    if (!orthogonal_removed.empty())
    {
        for (auto it = orthogonal_removed.rbegin(); it != orthogonal_removed.rend(); ++it)
        {
            invalidate(it->entity);
            m_orthogonal_cache.handle_swap(it->swap_info.exchanged, it->swap_info.old_index, it->swap_info.new_index);
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
    if (!m_entitites.is_alive(entity))
        return CameraData{};

    const PerspectiveCamera* perspective_camera = m_components.get<PerspectiveCamera>(entity);
    const OrthogonalCamera* orthogonal_camera = m_components.get<OrthogonalCamera>(entity);

    // A camera entity must have exactly one camera component
    if ((perspective_camera == nullptr) == (orthogonal_camera == nullptr))
        return CameraData{};

    auto& perspective_storage = m_components.get_storage<PerspectiveCamera>();
    auto& orthogonal_storage = m_components.get_storage<OrthogonalCamera>();

    // Reuse cached data when it is still valid
    uint32_t dense_idx = 0;
    bool is_perspective = (perspective_camera != nullptr);
    if (is_perspective)
    {
        dense_idx = perspective_storage.get_index(entity);
        if (!m_perspective_cache.is_dirty(dense_idx))
            return *m_perspective_cache.get(dense_idx);
    }
    else
    {
        dense_idx = orthogonal_storage.get_index(entity);
        if (!m_orthogonal_cache.is_dirty(dense_idx))
            return *m_orthogonal_cache.get(dense_idx);
    }

    // Calculate the view matrix from the entity's world transform
    Matrix4 world_matrix = m_transforms.get_world_matrix(entity);
    Matrix4 view_matrix = inverse(world_matrix);

    // Calculate the appropriate projection matrix
    Matrix4 projection_matrix;
    if (is_perspective)
        projection_matrix = mtx_perspective_projection(*perspective_camera);
    else
        projection_matrix = mtx_orthogonal_projection(*orthogonal_camera);

    // Calculate the view-projection matrix
    CameraData data;
    data.view = view_matrix;
    data.projection = projection_matrix;
    data.view_projection = projection_matrix * view_matrix;

    // Update the cache
    if (is_perspective)
        m_perspective_cache.set(dense_idx, data);
    else
        m_orthogonal_cache.set(dense_idx, data);

    return data;
}

void CameraSystem::invalidate(Entity entity)
{
    if (m_components.has<PerspectiveCamera>(entity))
    {
        auto& storage = m_components.get_storage<PerspectiveCamera>();
        m_perspective_cache.invalidate(storage.get_index(entity));
    }
    else if (m_components.has<OrthogonalCamera>(entity))
    {
        auto& storage = m_components.get_storage<OrthogonalCamera>();
        m_orthogonal_cache.invalidate(storage.get_index(entity));
    }
}

} // namespace omega
