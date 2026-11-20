#include <omega/transform/transform_system.hpp>
#include <omega/transform/transform_event.hpp>
#include <omega/transform/matrix_transform.hpp>

#include <omega/core/event/event_system.hpp>
#include <omega/core/component/component_event.hpp>
#include <omega/core/component/component_system.hpp>
#include <omega/core/hierarchy/hierarchy_event.hpp>
#include <omega/core/hierarchy/hierarchy_system.hpp>

namespace omega
{

TransformSystem::TransformSystem(EventSystem& events, ComponentSystem& components, 
                                 HierarchySystem& hierarchy, EntitySystem& entities)
    : m_events(events), m_components(components), m_hierarchy(hierarchy), m_entitites(entities)
{}

void TransformSystem::update()
{
    auto& transform_storage = m_components.get_storage<Transform>();

    // Process transform events
    const auto& events = m_events.get_queue<ComponentEvent<Transform>>().get_events();

    for (const auto& event : events)
    {
        switch (event.type)
        {
        case ComponentEventType::Added:
            m_cache.resize(transform_storage.size());
            invalidate(event.entity);
            break;

        case ComponentEventType::Modified:
            invalidate(event.entity);

            if (event.origin == ComponentModifiedOrigin::Direct)
                mark_descendants_dirty(event.entity);
            break;

        case ComponentEventType::Removed:
            invalidate(event.entity);
            m_cache.handle_swap(event.swap_info.exchanged, event.swap_info.old_index, event.swap_info.new_index);
            break;
        }
    }

    // Process hierarchy events
    const auto& hierarchy_events = m_events.get_queue<HierarchyModifiedEvent>().get_events();
    for (const auto& event : hierarchy_events)
    {
        invalidate(event.entity);
    }
}

Matrix4 TransformSystem::get_world_matrix(Entity entity)
{
    // Return identity for entities that are no longer alive
    if (!m_entitites.is_alive(entity))
        return Matrix4(1.0f);

    const Transform* transform = m_components.get<Transform>(entity);
    const Entity parent = m_hierarchy.get_parent(entity);

    // Entities without a local Transform inherit their parent's matrix
    if (!transform)
    {
        return parent.is_valid() ? get_world_matrix(parent) : Matrix4(1.0f);
    }

    auto& storage = m_components.get_storage<Transform>();
    const uint32_t dense_idx = storage.get_index(entity);

    // Use the cached matrix when valid.
    if (!m_cache.is_dirty(dense_idx))
        return *m_cache.get(dense_idx);

    const Matrix4 local_matrix = mtx_transform(*transform);

    // Combine the local matrix with the parent's world matrix
    Matrix4 world_matrix = local_matrix;
    if (parent.is_valid())
        world_matrix = get_world_matrix(parent) * local_matrix;

    // Update the cache
    m_cache.set(dense_idx, world_matrix);

    return world_matrix;
}

void TransformSystem::invalidate(Entity entity)
{
    if (!m_entitites.is_alive(entity))
        return;

    // Invalidate this entity's cached world transform
    if (m_components.has<Transform>(entity))
    {
        auto& storage = m_components.get_storage<Transform>();

        m_cache.invalidate(storage.get_index(entity));
    }

    // Notify dependent systems that this world transform may have changed
    m_events.publish(WorldTransformChangedEvent{entity});
}

void TransformSystem::mark_hierarchy_dirty(Entity entity)
{
    if (!m_entitites.is_alive(entity))
        return;

    if (m_components.has<Transform>(entity))
    {
        m_components.mark_dirty<Transform>(entity, ComponentModifiedOrigin::Propagated);
    }

    for (Entity child : m_hierarchy.get_children(entity))
        mark_hierarchy_dirty(child);
}

void TransformSystem::mark_descendants_dirty(Entity entity)
{
    for (Entity child : m_hierarchy.get_children(entity))
        mark_hierarchy_dirty(child);
}

} // namespace omega