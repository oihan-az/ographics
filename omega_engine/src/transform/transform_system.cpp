#include <omega/transform/transform_system.hpp>
#include <omega/transform/transform_event.hpp>
#include <omega/transform/matrix_transform.hpp>

namespace omega
{

TransformSystem::TransformSystem(Scene& scene, EventSystem& events) : m_scene(scene), m_events(events)
{}

void TransformSystem::update()
{
    // Process transform events
    const auto& component_events = m_events.get_queue<ComponentEvent<Transform>>().get_events();
    for (const auto& event : component_events)
    {
        switch (event.type)
        {
        case ComponentEventType::Added:
        case ComponentEventType::Changed:
            // Recalculate this entity and its descendants on next access
            invalidate(event.entity);
            break;

        case ComponentEventType::Removed:
            // Remove the cached matrix and invalidate descendants
            invalidate(event.entity);
            m_cache.erase(event.entity);
            break;
        }
    }

    // Process hierarchy events
    const auto& hierarchy_events = m_events.get_queue<HierarchyEvent>().get_events();
    for (const auto& event : hierarchy_events)
    {
        invalidate(event.entity);
    }
}

Matrix4 TransformSystem::get_world_matrix(Entity entity)
{
    // Return identity for entities that are no longer alive
    if (!m_scene.is_alive(entity))
        return Matrix4(1.0f);

    // Reuse the cached matrix when it is still valid
    if (!m_cache.is_dirty(entity))
        return *m_cache.get(entity);

    // Get the local matrix, or return the parent's world matrix if no Transform exists
    const Transform* transform = m_scene.get<Transform>(entity);
    if (!transform)
    {
        Entity parent = m_scene.get_parent(entity);
        return parent.is_valid() ? get_world_matrix(parent) : Matrix4(1.0f);
    }

    Matrix4 local_matrix = mtx_transform(*transform);

    // Combine the local matrix with the parent's world matrix
    Matrix4 world_matrix = local_matrix;
    Entity parent = m_scene.get_parent(entity);
    if (parent.is_valid())
        world_matrix = get_world_matrix(parent) * local_matrix;

    // Update the cache
    m_cache.set(entity, world_matrix);

    return world_matrix;
}

void TransformSystem::invalidate(Entity entity)
{
    m_cache.invalidate(entity);

    // Notify dependent systems that this world transform may have changed
    m_events.publish(WorldTransformChangedEvent{entity});

    // Invalidate descendants because their world transforms depend on this entity
    for (Entity child : m_scene.get_children(entity))
        invalidate(child);
}

} // namespace omega