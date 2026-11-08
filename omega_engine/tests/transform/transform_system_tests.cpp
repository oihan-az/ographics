#include <gtest/gtest.h>

#include <omega/core/event/event_system.hpp>
#include <omega/core/math/matrix4.hpp>
#include <omega/core/entity/entity_system.hpp>
#include <omega/core/component/component_system.hpp>
#include <omega/core/hierarchy/hierarchy_system.hpp>
#include <omega/transform/transform.hpp>
#include <omega/transform/matrix_transform.hpp>
#include <omega/transform/transform_system.hpp>

namespace
{

// Compares two matrices element by element
void ExpectMatrixNear(const omega::Matrix4& actual, const omega::Matrix4& expected)
{
    for (int column = 0; column < 4; ++column)
    {
        for (int row = 0; row < 4; ++row)
        {
            EXPECT_NEAR(actual[column][row], expected[column][row], 0.0001f);
        }
    }
}

} // namespace

// Checks that a root entity's world matrix equals its local matrix
TEST(TransformSystemTests, RootWorldMatrixEqualsLocalMatrix)
{
    omega::EventSystem events;
    omega::EntitySystem entities(events);
    omega::ComponentSystem components(events, entities);
    omega::HierarchySystem hierarchy(events, entities);

    auto entity = entities.create_entity();
    events.swap_all_buffers();
    entities.update();

    omega::Transform transform;
    transform.position = omega::Vector3(1.0f, 2.0f, 3.0f);

    ASSERT_TRUE(components.add(entity, transform));

    omega::TransformSystem transforms(events, components, hierarchy, entities);

    // Swap the data-driven double buffers and update the transform processing pipeline
    events.swap_all_buffers();
    transforms.update();

    ExpectMatrixNear(transforms.get_world_matrix(entity), mtx_transform(transform));
}

// Checks that a child world matrix combines its parent's and its local matrix
TEST(TransformSystemTests, ChildWorldMatrixIncludesParentTransform)
{
    omega::EventSystem events;
    omega::EntitySystem entities(events);
    omega::ComponentSystem components(events, entities);
    omega::HierarchySystem hierarchy(events, entities);

    auto parent = entities.create_entity();
    auto child = entities.create_entity();
    events.swap_all_buffers();
    entities.update();

    omega::Transform parent_transform;
    parent_transform.position = omega::Vector3(10.0f, 0.0f, 0.0f);

    omega::Transform child_transform;
    child_transform.position = omega::Vector3(0.0f, 5.0f, 0.0f);

    ASSERT_TRUE(components.add(parent, parent_transform));
    ASSERT_TRUE(components.add(child, child_transform));
    ASSERT_TRUE(hierarchy.set_parent(child, parent));

    omega::TransformSystem transforms(events, components, hierarchy, entities);

    // Discharging pending addition and structural hierarchy events sequentially
    events.swap_all_buffers();
    hierarchy.update();
    transforms.update();

    auto expected = mtx_transform(parent_transform) * mtx_transform(child_transform);

    ExpectMatrixNear(transforms.get_world_matrix(child), expected);
}

// Checks that world transforms include all ancestors
TEST(TransformSystemTests, NestedChildWorldMatrixIncludesAllAncestors)
{
    omega::EventSystem events;
    omega::EntitySystem entities(events);
    omega::ComponentSystem components(events, entities);
    omega::HierarchySystem hierarchy(events, entities);

    auto grandparent = entities.create_entity();
    auto parent = entities.create_entity();
    auto child = entities.create_entity();
    events.swap_all_buffers();
    entities.update();

    omega::Transform grandparent_transform;
    grandparent_transform.position = omega::Vector3(10.0f, 0.0f, 0.0f);

    omega::Transform parent_transform;
    parent_transform.position = omega::Vector3(0.0f, 5.0f, 0.0f);

    omega::Transform child_transform;
    child_transform.position = omega::Vector3(0.0f, 0.0f, 3.0f);

    ASSERT_TRUE(components.add(grandparent, grandparent_transform));
    ASSERT_TRUE(components.add(parent, parent_transform));
    ASSERT_TRUE(components.add(child, child_transform));

    ASSERT_TRUE(hierarchy.set_parent(parent, grandparent));
    ASSERT_TRUE(hierarchy.set_parent(child, parent));

    omega::TransformSystem transforms(events, components, hierarchy, entities);

    // Ingest data state from previous frame pipeline
    events.swap_all_buffers();
    hierarchy.update();
    transforms.update();

    auto expected =
        mtx_transform(grandparent_transform) * mtx_transform(parent_transform) * mtx_transform(child_transform);

    ExpectMatrixNear(transforms.get_world_matrix(child), expected);
}

// Checks that changing a local transform invalidates its cached world matrix
TEST(TransformSystemTests, ChangedTransformInvalidatesWorldMatrix)
{
    omega::EventSystem events;
    omega::EntitySystem entities(events);
    omega::ComponentSystem components(events, entities);
    omega::HierarchySystem hierarchy(events, entities);

    auto entity = entities.create_entity();
    events.swap_all_buffers();
    entities.update();

    omega::Transform transform;
    ASSERT_TRUE(components.add(entity, transform));

    omega::TransformSystem transforms(events, components, hierarchy, entities);

    events.swap_all_buffers();
    transforms.update();

    auto initial = transforms.get_world_matrix(entity);

    auto* stored_transform = components.get<omega::Transform>(entity);
    ASSERT_NE(stored_transform, nullptr);

    stored_transform->position = omega::Vector3(5.0f, 2.0f, 1.0f);

    ASSERT_TRUE(components.mark_dirty<omega::Transform>(entity));

    // Batch process modification and mutation events on demand
    events.swap_all_buffers();
    hierarchy.update();
    transforms.update();

    auto updated = transforms.get_world_matrix(entity);

    EXPECT_FALSE(updated == initial);
    ExpectMatrixNear(updated, mtx_transform(*stored_transform));
}

// Checks that changing a parent's transform invalidates its descendants
TEST(TransformSystemTests, ChangedParentTransformInvalidatesChildren)
{
    omega::EventSystem events;
    omega::EntitySystem entities(events);
    omega::ComponentSystem components(events, entities);
    omega::HierarchySystem hierarchy(events, entities);

    auto parent = entities.create_entity();
    auto child = entities.create_entity();
    events.swap_all_buffers();
    entities.update();

    omega::Transform parent_transform;
    omega::Transform child_transform;

    child_transform.position = omega::Vector3(0.0f, 5.0f, 0.0f);

    ASSERT_TRUE(components.add(parent, parent_transform));
    ASSERT_TRUE(components.add(child, child_transform));
    ASSERT_TRUE(hierarchy.set_parent(child, parent));

    omega::TransformSystem transforms(events, components, hierarchy, entities);
    
    events.swap_all_buffers();
    hierarchy.update();
    transforms.update();

    auto initial = transforms.get_world_matrix(child);

    auto* stored_parent = components.get<omega::Transform>(parent);
    ASSERT_NE(stored_parent, nullptr);

    stored_parent->position = omega::Vector3(10.0f, 0.0f, 0.0f);

    ASSERT_TRUE(components.mark_dirty<omega::Transform>(parent));
    
    // Push the mutation event and process it linearly through the system
    events.swap_all_buffers();
    hierarchy.update();
    transforms.update();

    auto updated = transforms.get_world_matrix(child);
    auto expected = mtx_transform(*stored_parent) * mtx_transform(child_transform);

    EXPECT_FALSE(updated == initial);
    ExpectMatrixNear(updated, expected);
}

// Checks that reparenting invalidates the child's world matrix
TEST(TransformSystemTests, ReparentingInvalidatesWorldMatrix)
{
    omega::EventSystem events;
    omega::EntitySystem entities(events);
    omega::ComponentSystem components(events, entities);
    omega::HierarchySystem hierarchy(events, entities);

    auto first_parent = entities.create_entity();
    auto second_parent = entities.create_entity();
    auto child = entities.create_entity();
    events.swap_all_buffers();
    entities.update();

    omega::Transform first_transform;
    first_transform.position = omega::Vector3(10.0f, 0.0f, 0.0f);

    omega::Transform second_transform;
    second_transform.position = omega::Vector3(20.0f, 0.0f, 0.0f);

    omega::Transform child_transform;
    child_transform.position = omega::Vector3(0.0f, 5.0f, 0.0f);

    ASSERT_TRUE(components.add(first_parent, first_transform));
    ASSERT_TRUE(components.add(second_parent, second_transform));
    ASSERT_TRUE(components.add(child, child_transform));

    ASSERT_TRUE(hierarchy.set_parent(child, first_parent));

    omega::TransformSystem transforms(events, components, hierarchy, entities);

    events.swap_all_buffers();
    hierarchy.update();
    transforms.update();

    auto initial = transforms.get_world_matrix(child);

    ASSERT_TRUE(hierarchy.set_parent(child, second_parent));

    // Expose structural change events via the new framework dispatch paradigm
    events.swap_all_buffers();
    hierarchy.update();
    transforms.update();

    auto updated = transforms.get_world_matrix(child);
    auto expected = mtx_transform(second_transform) * mtx_transform(child_transform);

    EXPECT_FALSE(updated == initial);
    ExpectMatrixNear(updated, expected);
}

// Checks that clearing a parent invalidates the child's world matrix
TEST(TransformSystemTests, ClearingParentInvalidatesWorldMatrix)
{
    omega::EventSystem events;
    omega::EntitySystem entities(events);
    omega::ComponentSystem components(events, entities);
    omega::HierarchySystem hierarchy(events, entities);

    auto parent = entities.create_entity();
    auto child = entities.create_entity();
    events.swap_all_buffers();
    entities.update();

    omega::Transform parent_transform;
    parent_transform.position = omega::Vector3(10.0f, 0.0f, 0.0f);

    omega::Transform child_transform;
    child_transform.position = omega::Vector3(0.0f, 5.0f, 0.0f);

    ASSERT_TRUE(components.add(parent, parent_transform));
    ASSERT_TRUE(components.add(child, child_transform));
    ASSERT_TRUE(hierarchy.set_parent(child, parent));

    omega::TransformSystem transforms(events, components, hierarchy, entities);

    events.swap_all_buffers();
    hierarchy.update();
    transforms.update();

    transforms.get_world_matrix(child);

    ASSERT_TRUE(hierarchy.clear_parent(child));

    // Flush detaching structural events to execute cache invalidations
    events.swap_all_buffers();
    hierarchy.update();
    transforms.update();

    ExpectMatrixNear(transforms.get_world_matrix(child), mtx_transform(child_transform));
}

// Checks that removing a Transform invalidates its cached matrix
TEST(TransformSystemTests, RemovingTransformResetsWorldMatrix)
{
    omega::EventSystem events;
    omega::EntitySystem entities(events);
    omega::ComponentSystem components(events, entities);
    omega::HierarchySystem hierarchy(events, entities);

    auto entity = entities.create_entity();
    events.swap_all_buffers();
    entities.update();

    omega::Transform transform;
    transform.position = omega::Vector3(5.0f, 0.0f, 0.0f);

    ASSERT_TRUE(components.add(entity, transform));

    omega::TransformSystem transforms(events, components, hierarchy, entities);

    events.swap_all_buffers();
    hierarchy.update();
    transforms.update();

    transforms.get_world_matrix(entity);

    ASSERT_TRUE(components.remove<omega::Transform>(entity));

    // Purge cached transforms during systemic lifecycle removal batching
    events.swap_all_buffers();
    hierarchy.update();
    transforms.update();

    ExpectMatrixNear(transforms.get_world_matrix(entity), omega::Matrix4(1.0f));
}

// Checks that an entity without a Transform uses an identity local matrix
TEST(TransformSystemTests, EntityWithoutTransformUsesIdentityMatrix)
{
    omega::EventSystem events;
    omega::EntitySystem entities(events);
    omega::ComponentSystem components(events, entities);
    omega::HierarchySystem hierarchy(events, entities);

    auto entity = entities.create_entity();
    events.swap_all_buffers();
    entities.update();

    omega::TransformSystem transforms(events, components, hierarchy, entities);

    // Swap the data buffers and execute the frame synchronization pass
    events.swap_all_buffers();
    transforms.update();

    ExpectMatrixNear(transforms.get_world_matrix(entity), omega::Matrix4(1.0f));
}

// Checks that destroying a parent invalidates its children's world matrices
TEST(TransformSystemTests, DestroyingParentInvalidatesChildWorldMatrix)
{
    omega::EventSystem events;
    omega::EntitySystem entities(events);
    omega::ComponentSystem components(events, entities);
    omega::HierarchySystem hierarchy(events, entities);

    auto parent = entities.create_entity();
    auto child = entities.create_entity();
    events.swap_all_buffers();
    entities.update();

    omega::Transform parent_transform;
    parent_transform.position = omega::Vector3(10.0f, 0.0f, 0.0f);

    omega::Transform child_transform;
    child_transform.position = omega::Vector3(0.0f, 5.0f, 0.0f);

    ASSERT_TRUE(components.add(parent, parent_transform));
    ASSERT_TRUE(components.add(child, child_transform));
    ASSERT_TRUE(hierarchy.set_parent(child, parent));

    omega::TransformSystem transforms(events, components, hierarchy, entities);

    // Process the initial creation and hierarchy setups
    events.swap_all_buffers();
    transforms.update();

    transforms.get_world_matrix(child);

    // Destroy the parent entity and flush structural command streams
    entities.destroy_entity(parent);
    entities.update();

    // Process component removal and hierarchy detachment events in batch
    events.swap_all_buffers();
    transforms.update();

    EXPECT_TRUE(entities.is_alive(child));

    // The child should now fallback onto its standalone local matrix since the parent is gone
    ExpectMatrixNear(transforms.get_world_matrix(child), mtx_transform(child_transform));
}