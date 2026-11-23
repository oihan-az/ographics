
#include <omega/transform/matrix_transform.hpp>
#include <omega/transform/transform_event.hpp>
#include <omega/transform/transform_system.hpp>

#include <omega/core/component/component_event.hpp>
#include <omega/core/component/component_system.hpp>
#include <omega/core/entity/entity_event.hpp>
#include <omega/core/entity/entity_system.hpp>
#include <omega/core/event/event_system.hpp>
#include <omega/core/hierarchy/hierarchy_event.hpp>
#include <omega/core/hierarchy/hierarchy_system.hpp>
#include <omega/core/math/quaternion.hpp>

#include <utility>

namespace omega
{

TransformSystem::TransformSystem(EventSystem& events, ComponentSystem& components, HierarchySystem& hierarchy,
                                 EntitySystem& entities)
    : m_events(events), m_components(components), m_hierarchy(hierarchy), m_entities(entities)
{
}

void TransformSystem::prepare_hierarchy_changes()
{
    m_hierarchy_snapshots.clear();

    // These events describe requested changes that the hierarchy
    // system will apply later in this same frame
    const auto& parent_events = m_events.get_queue<HierarchyParentChangedEvent>().get_events();
    for (const auto& event : parent_events)
    {
        const Entity entity = event.child;
        if (!m_entities.is_alive(entity))
            continue;

        // Capture the original world transform only once
        if (m_hierarchy_snapshots.contains(entity))
            continue;

        m_hierarchy_snapshots.emplace(entity, HierarchySnapshot{.entity = entity,
                                                                .old_parent = m_hierarchy.get_parent(entity),
                                                                .world = calculate_world_uncached(entity)});
    }

    // Track surviving children affected by parent destruction
    const auto& removed_events = m_events.get_queue<EntityRemovedEvent>().get_events();
    for (const auto& event : removed_events)
    {
        for (Entity child : m_hierarchy.get_children(event.entity))
        {
            if (!m_entities.is_alive(child))
                continue;

            if (m_hierarchy_snapshots.contains(child))
                continue;

            m_hierarchy_snapshots.emplace(child, HierarchySnapshot{.entity = child,
                                                                   .old_parent = event.entity,
                                                                   .world = calculate_world_uncached(child)});
        }
    }
}

void TransformSystem::update()
{
    auto& storage = m_components.get_storage<Transform>();

    // Process transform component events
    const auto& events = m_events.get_queue<ComponentEvent<Transform>>().get_events();
    for (const auto& event : events)
    {
        switch (event.type)
        {
        case ComponentEventType::Added:
        {
            m_cache.resize(storage.size());

            invalidate(event.entity);
            mark_descendants_dirty(event.entity);

            break;
        }

        case ComponentEventType::Modified:
        {
            if (event.origin == ComponentModifiedOrigin::Direct)
            {
                // Direct modifications affect this entity and
                // every descendant in the current frame
                invalidate(event.entity);
                mark_descendants_dirty(event.entity);
            }
            else
            {
                // Propagated modifications do not start another
                // descendant traversal
                invalidate_cache(event.entity);
            }

            break;
        }

        case ComponentEventType::Removed:
        {
            invalidate(event.entity);

            m_cache.handle_swap(event.swap_info.exchanged, event.swap_info.old_index, event.swap_info.new_index);

            mark_descendants_dirty(event.entity);

            break;
        }
        }
    }

    // Reconcile all pending hierarchy changes after the
    // hierarchy system has applied their final relationships
    apply_hierarchy_changes();
}

Matrix4 TransformSystem::get_world_matrix(Entity entity)
{
    if (!m_entities.is_alive(entity))
        return Matrix4(1.0f);

    const Transform* transform = m_components.get<Transform>(entity);
    const Entity parent = m_hierarchy.get_parent(entity);

    // Without a local transform, inherit the parent's world matrix.
    if (!transform)
    {
        return parent.is_valid() ? get_world_matrix(parent) : Matrix4(1.0f);
    }

    auto& storage = m_components.get_storage<Transform>();
    m_cache.resize(storage.size());

    const uint32_t dense_idx = storage.get_index(entity);
    if (!m_cache.is_dirty(dense_idx))
        return *m_cache.get(dense_idx);

    const Matrix4 local_matrix = mtx_transform(*transform);
    Matrix4 world_matrix = local_matrix;
    if (parent.is_valid())
        world_matrix = get_world_matrix(parent) * local_matrix;

    m_cache.set(dense_idx, world_matrix);

    return world_matrix;
}

Matrix4 TransformSystem::calculate_world_uncached(Entity entity)
{
    Matrix4 local_matrix{1.0f};
    if (const auto* transform = m_components.get<Transform>(entity))
        local_matrix = mtx_transform(*transform);

    const Entity parent = m_hierarchy.get_parent(entity);
    if (!parent.is_valid())
        return local_matrix;

    return calculate_world_uncached(parent) * local_matrix;
}

void TransformSystem::apply_hierarchy_changes()
{
    std::unordered_set<Entity, EntityHash> processed;
    for (const auto& [key, snapshot] : m_hierarchy_snapshots)
    {
        apply_hierarchy_change(snapshot.entity, processed);
    }

    m_hierarchy_snapshots.clear();
}

void TransformSystem::apply_hierarchy_change(Entity entity, std::unordered_set<Entity, EntityHash>& processed)
{
    if (!processed.insert(entity).second)
        return;

    const auto it = m_hierarchy_snapshots.find(entity);
    if (it == m_hierarchy_snapshots.end())
        return;

    if (!m_entities.is_alive(entity))
        return;

    const HierarchySnapshot& snapshot = it->second;

    // Read the final parent after
    const Entity new_parent = m_hierarchy.get_parent(entity);

    // Resolve the new parent first if it was also reparented
    // This ensures the new parent's local transform has already
    // been adjusted before calculating the child's new local
    if (new_parent.is_valid() && m_hierarchy_snapshots.contains(new_parent))
    {
        apply_hierarchy_change(new_parent, processed);
    }

    // The hierarchy might have received multiple requests
    // that ultimately left the original parent unchanged
    if (snapshot.old_parent == new_parent)
        return;

    Transform* transform = m_components.get<Transform>(entity);
    if (!transform)
    {
        // An entity without a local transform cannot compensate
        // for a new parent, so its inherited world matrix changes
        invalidate(entity);
        mark_descendants_dirty(entity);
        return;
    }

    const Matrix4 parent_world = new_parent.is_valid() ? calculate_world_uncached(new_parent) : Matrix4(1.0f);

    // Preserve the entity's original world transform
    const Matrix4 new_local = omega::inverse(parent_world) * snapshot.world;

    Vector3 position;
    Quaternion rotation;
    Vector3 scale;
    Vector3 skew;
    Vector4 perspective;

    if (!omega::decompose(new_local, scale, rotation, position, skew, perspective))
    {
        // If the new local matrix cannot be decomposed,
        // preserve cache correctness even though world
        // preservation could not be completed
        invalidate(entity);
        mark_descendants_dirty(entity);
        return;
    }

    transform->position = position;
    transform->rotation = omega::normalize(rotation);
    transform->scale = scale;

    // The local transform was adjusted by the hierarchy system's
    // structural change, not directly by the user
    m_components.mark_dirty<Transform>(entity, ComponentModifiedOrigin::Propagated);

    // Invalidate immediately. The next query recomputes the
    // same world transform from the new parent and local matrix
    invalidate_cache(entity);

    // No descendant propagation is needed on success because
    // the entity's world transform was preserved
}

void TransformSystem::invalidate_cache(Entity entity)
{
    if (!m_entities.is_alive(entity))
        return;

    if (!m_components.has<Transform>(entity))
        return;

    auto& storage = m_components.get_storage<Transform>();
    m_cache.resize(storage.size());

    const uint32_t dense_idx = storage.get_index(entity);
    m_cache.invalidate(dense_idx);
}

void TransformSystem::invalidate(Entity entity)
{
    if (!m_entities.is_alive(entity))
        return;

    invalidate_cache(entity);

    // Notify systems interested in world-transform changes
    m_events.publish(WorldTransformChangedEvent{entity});
}

// -----------------------------------------------------------------------------
// Descendant propagation
// -----------------------------------------------------------------------------

void TransformSystem::mark_hierarchy_dirty(Entity entity)
{
    if (!m_entities.is_alive(entity))
        return;

    // Immediately invalidate the world-matrix cache
    invalidate(entity);

    // Publish the propagated component modification
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
