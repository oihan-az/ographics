#include <omega/transform/transform_system.hpp>
#include <omega/transform/transform_event.hpp>
#include <omega/transform/matrix_transform.hpp>

namespace omega
{

TransformSystem::TransformSystem(Scene& scene, EventSystem& events) : m_scene(scene), m_events(events)
{}

void TransformSystem::update()
{
    auto& transform_storage = m_scene.component_manager().get_storage<Transform>();

    // Process added
    const auto& added_queue = m_events.get_queue<ComponentAddedEvent<Transform>>().get_events();
    if (!added_queue.empty())
    {
        for (auto it = added_queue.rbegin(); it != added_queue.rend(); ++it)
        {
            m_cache.resize(transform_storage.size());
            invalidate(it->entity);
        }
    }

    // Process modified
    const auto& modified_queue = m_events.get_queue<ComponentModifiedEvent<Transform>>().get_events();
    if (!modified_queue.empty())
    {
        for (auto it = modified_queue.rbegin(); it != modified_queue.rend(); ++it)
        {
            invalidate(it->entity);
        }
    }

    // Process removed
    const auto& removed_queue = m_events.get_queue<ComponentRemovedEvent<Transform>>().get_events();
    if (!removed_queue.empty())
    {
        for (auto it = removed_queue.rbegin(); it != removed_queue.rend(); ++it)
        {
            invalidate(it->entity);
            m_cache.handle_swap(it->swap_info.exchanged, it->swap_info.old_index, it->swap_info.new_index);
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

    auto& transform_storage = m_scene.component_manager().get_storage<Transform>();
    uint32_t dense_idx = transform_storage.get_index(entity);

    // Reuse the cached matrix when it is still valid
    if (!m_cache.is_dirty(dense_idx))
        return *m_cache.get(dense_idx);

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
    m_cache.set(dense_idx, world_matrix);

    return world_matrix;
}

void TransformSystem::invalidate(Entity entity)
{
    auto& transform_storage = m_scene.component_manager().get_storage<Transform>();
    uint32_t dense_idx = transform_storage.get_index(entity);

    m_cache.invalidate(dense_idx);

    // Notify dependent systems that this world transform may have changed
    m_events.publish(WorldTransformChangedEvent{entity});

    // Invalidate descendants because their world transforms depend on this entity
    for (Entity child : m_scene.get_children(entity))
        invalidate(child);
}

} // namespace omega