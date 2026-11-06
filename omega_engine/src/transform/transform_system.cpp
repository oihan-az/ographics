#include <omega/transform/transform_system.hpp>

namespace omega
{

TransformSystem::TransformSystem(Scene& scene, EventSystem& events) : m_scene(scene), m_events(events)
{
    m_component_subscription = m_events.subscribe<ComponentEvent<Transform>>
    (
        [this](const ComponentEvent<Transform>& event) 
        { 
            on_component_event(event); 
        }
    );

    m_hierarchy_subscription =m_events.subscribe<HierarchyEvent>
    (
        [this](const HierarchyEvent& event) 
        { 
            on_hierarchy_event(event); 
        }
    );
}

TransformSystem::~TransformSystem()
{
    m_events.unsubscribe(m_component_subscription);
    m_events.unsubscribe(m_hierarchy_subscription);
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

    Matrix4 local_matrix = transform->matrix();

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

    // Invalidate descendants because their world transforms depend on this entity
    for (Entity child : m_scene.get_children(entity))
        invalidate(child);
}

void TransformSystem::on_component_event(const ComponentEvent<Transform>& event)
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

void TransformSystem::on_hierarchy_event(const HierarchyEvent& event)
{
    invalidate(event.entity);
}

} // namespace omega