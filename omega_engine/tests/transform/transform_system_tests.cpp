
#include <gtest/gtest.h>

#include <omega/core/component/component_system.hpp>
#include <omega/core/entity/entity_system.hpp>
#include <omega/core/event/event_system.hpp>
#include <omega/core/hierarchy/hierarchy_system.hpp>
#include <omega/core/math/matrix4.hpp>

#include <omega/transform/matrix_transform.hpp>
#include <omega/transform/transform.hpp>
#include <omega/transform/transform_system.hpp>

namespace
{

constexpr float kEpsilon = 0.0001f;

void ExpectMatrixNear(const omega::Matrix4& actual, const omega::Matrix4& expected)
{
    for (int column = 0; column < 4; ++column)
    {
        for (int row = 0; row < 4; ++row)
        {
            EXPECT_NEAR(actual[column][row], expected[column][row], kEpsilon)
                << "Matrix element [" << column << "][" << row << "]";
        }
    }
}

void ExpectPositionNear(const omega::Matrix4& matrix, float x, float y, float z)
{
    EXPECT_NEAR(matrix[3][0], x, kEpsilon);
    EXPECT_NEAR(matrix[3][1], y, kEpsilon);
    EXPECT_NEAR(matrix[3][2], z, kEpsilon);
}

omega::Transform MakeTransform(float x = 0.0f, float y = 0.0f, float z = 0.0f)
{
    omega::Transform transform;
    transform.position = {x, y, z};
    return transform;
}

class TransformSystemTests : public ::testing::Test
{
  protected:
    omega::EventSystem events;
    omega::EntitySystem entities{events};
    omega::ComponentSystem components{events, entities};
    omega::HierarchySystem hierarchy{events, entities};
    omega::TransformSystem transforms{events, components, hierarchy, entities};

    omega::Entity create_entity()
    {
        const auto entity = entities.create_entity();

        events.swap_all_buffers();
        entities.update();

        return entity;
    }

    void update()
    {
        events.swap_all_buffers();

        entities.update();
        transforms.prepare_hierarchy_changes();
        hierarchy.update();
        components.update();
        transforms.update();
    }

    void add_transform(omega::Entity entity, float x = 0.0f, float y = 0.0f, float z = 0.0f)
    {
        ASSERT_TRUE(components.add(entity, MakeTransform(x, y, z)));
    }

    void set_position(omega::Entity entity, float x, float y, float z)
    {
        auto* transform = components.get<omega::Transform>(entity);

        ASSERT_NE(transform, nullptr);

        transform->position = {x, y, z};

        ASSERT_TRUE(components.mark_dirty<omega::Transform>(entity, omega::ComponentModifiedOrigin::Direct));
    }

    omega::Matrix4 world(omega::Entity entity)
    {
        return transforms.get_world_matrix(entity);
    }

    omega::Matrix4 local(omega::Entity entity)
    {
        const auto* transform = components.get<omega::Transform>(entity);

        if (!transform)
            return omega::Matrix4(1.0f);

        return omega::mtx_transform(*transform);
    }
};

// -----------------------------------------------------------------------------
// Basic transforms
// -----------------------------------------------------------------------------

TEST_F(TransformSystemTests, RootWorldMatrixEqualsLocalMatrix)
{
    const auto entity = create_entity();

    add_transform(entity, 1.0f, 2.0f, 3.0f);
    update();

    ExpectMatrixNear(world(entity), local(entity));
    ExpectPositionNear(world(entity), 1.0f, 2.0f, 3.0f);
}

TEST_F(TransformSystemTests, EntityWithoutTransformUsesIdentityMatrix)
{
    const auto entity = create_entity();

    update();

    ExpectMatrixNear(world(entity), omega::Matrix4(1.0f));
}

TEST_F(TransformSystemTests, InvalidEntityUsesIdentityMatrix)
{
    update();

    ExpectMatrixNear(world(omega::Entity::invalid()), omega::Matrix4(1.0f));
}

TEST_F(TransformSystemTests, RepeatedWorldQueriesReturnSameMatrix)
{
    const auto entity = create_entity();

    add_transform(entity, 4.0f, 5.0f, 6.0f);
    update();

    const auto expected = world(entity);

    for (int i = 0; i < 5; ++i)
        ExpectMatrixNear(world(entity), expected);
}

TEST_F(TransformSystemTests, MultipleEntitiesHaveIndependentWorldMatrices)
{
    const auto first = create_entity();
    const auto second = create_entity();

    add_transform(first, 1.0f, 2.0f, 3.0f);
    add_transform(second, 4.0f, 5.0f, 6.0f);

    update();

    ExpectPositionNear(world(first), 1.0f, 2.0f, 3.0f);
    ExpectPositionNear(world(second), 4.0f, 5.0f, 6.0f);
}

// -----------------------------------------------------------------------------
// Direct transform modifications
// -----------------------------------------------------------------------------

TEST_F(TransformSystemTests, DirectModificationUpdatesWorldMatrixInOneFrame)
{
    const auto entity = create_entity();

    add_transform(entity, 1.0f, 2.0f, 3.0f);
    update();

    const auto initial = world(entity);

    set_position(entity, 5.0f, 6.0f, 7.0f);
    update();

    EXPECT_FALSE(world(entity) == initial);

    ExpectPositionNear(world(entity), 5.0f, 6.0f, 7.0f);
    ExpectMatrixNear(world(entity), local(entity));
}

TEST_F(TransformSystemTests, ModifyingOneEntityDoesNotAffectAnother)
{
    const auto first = create_entity();
    const auto second = create_entity();

    add_transform(first, 1.0f, 0.0f, 0.0f);
    add_transform(second, 5.0f, 0.0f, 0.0f);

    update();

    const auto second_world = world(second);

    set_position(first, 10.0f, 0.0f, 0.0f);
    update();

    ExpectPositionNear(world(first), 10.0f, 0.0f, 0.0f);
    ExpectMatrixNear(world(second), second_world);
}

// -----------------------------------------------------------------------------
// Transform propagation
// -----------------------------------------------------------------------------

TEST_F(TransformSystemTests, ParentModificationUpdatesChildInOneFrame)
{
    const auto parent = create_entity();
    const auto child = create_entity();

    add_transform(parent);
    add_transform(child, 0.0f, 5.0f, 0.0f);

    update();

    ASSERT_TRUE(hierarchy.set_parent(child, parent));
    update();

    const auto initial_world = world(child);
    const auto initial_local = local(child);

    set_position(parent, 10.0f, 0.0f, 0.0f);
    update();

    EXPECT_FALSE(world(child) == initial_world);

    // Moving the parent changes the child's world transform,
    // but must not modify the child's local transform.
    ExpectMatrixNear(local(child), initial_local);

    ExpectPositionNear(world(parent), 10.0f, 0.0f, 0.0f);
    ExpectPositionNear(world(child), 10.0f, 5.0f, 0.0f);
}

TEST_F(TransformSystemTests, ParentModificationUpdatesAllDescendantsInOneFrame)
{
    const auto root = create_entity();
    const auto child = create_entity();
    const auto grandchild = create_entity();

    add_transform(root);
    add_transform(child, 0.0f, 2.0f, 0.0f);
    add_transform(grandchild, 0.0f, 0.0f, 3.0f);

    update();

    ASSERT_TRUE(hierarchy.set_parent(child, root));
    ASSERT_TRUE(hierarchy.set_parent(grandchild, child));

    update();

    // Ensure all world matrices are cached before modification.
    world(root);
    world(child);
    world(grandchild);

    set_position(root, 10.0f, 0.0f, 0.0f);
    update();

    ExpectPositionNear(world(root), 10.0f, 0.0f, 0.0f);
    ExpectPositionNear(world(child), 10.0f, 2.0f, 0.0f);
    ExpectPositionNear(world(grandchild), 10.0f, 0.0f, 3.0f);
}

TEST_F(TransformSystemTests, ChildModificationDoesNotAffectParent)
{
    const auto parent = create_entity();
    const auto child = create_entity();

    add_transform(parent, 10.0f, 0.0f, 0.0f);
    add_transform(child, 2.0f, 3.0f, 0.0f);

    update();

    ASSERT_TRUE(hierarchy.set_parent(child, parent));
    update();

    const auto parent_world = world(parent);

    set_position(child, 1.0f, 5.0f, 0.0f);
    update();

    ExpectMatrixNear(world(parent), parent_world);

    ExpectMatrixNear(world(child), world(parent) * local(child));
}

TEST_F(TransformSystemTests, ParentModificationDoesNotAffectUnrelatedHierarchy)
{
    const auto first_parent = create_entity();
    const auto first_child = create_entity();

    const auto second_parent = create_entity();
    const auto second_child = create_entity();

    add_transform(first_parent);
    add_transform(first_child, 0.0f, 2.0f, 0.0f);

    add_transform(second_parent, 20.0f, 0.0f, 0.0f);
    add_transform(second_child, 0.0f, 4.0f, 0.0f);

    update();

    ASSERT_TRUE(hierarchy.set_parent(first_child, first_parent));
    ASSERT_TRUE(hierarchy.set_parent(second_child, second_parent));
    update();

    const auto original_second_world = world(second_child);

    set_position(first_parent, 10.0f, 0.0f, 0.0f);
    update();

    ExpectPositionNear(world(first_child), 10.0f, 2.0f, 0.0f);
    ExpectMatrixNear(world(second_child), original_second_world);
}

// -----------------------------------------------------------------------------
// World-preserving parenting
// -----------------------------------------------------------------------------

TEST_F(TransformSystemTests, ParentingPreservesWorldTransform)
{
    const auto parent = create_entity();
    const auto child = create_entity();

    add_transform(parent, 10.0f, 0.0f, 0.0f);
    add_transform(child, 2.0f, 5.0f, 3.0f);

    update();

    const auto original_world = world(child);

    ASSERT_TRUE(hierarchy.set_parent(child, parent));
    update();

    EXPECT_EQ(hierarchy.get_parent(child), parent);

    ExpectMatrixNear(world(child), original_world);

    const auto expected_local = omega::inverse(world(parent)) * original_world;

    ExpectMatrixNear(local(child), expected_local);
}

TEST_F(TransformSystemTests, ParentingWithoutCachedWorldMatrixPreservesWorldTransform)
{
    const auto parent = create_entity();
    const auto child = create_entity();

    add_transform(parent, 10.0f, 0.0f, 0.0f);
    add_transform(child, 2.0f, 5.0f, 3.0f);

    update();

    // Deliberately do not call world(child) before parenting.
    const auto expected_world = local(child);

    ASSERT_TRUE(hierarchy.set_parent(child, parent));
    update();

    ExpectMatrixNear(world(child), expected_world);
    ExpectPositionNear(world(child), 2.0f, 5.0f, 3.0f);
}

TEST_F(TransformSystemTests, ParentingWithDirtyCachePreservesWorldTransform)
{
    const auto parent = create_entity();
    const auto child = create_entity();

    add_transform(parent, 10.0f, 0.0f, 0.0f);
    add_transform(child, 2.0f, 0.0f, 0.0f);

    update();

    // Cache the old world matrix.
    world(child);

    // Change local data without processing the dirty event yet.
    set_position(child, 7.0f, 3.0f, 0.0f);

    ASSERT_TRUE(hierarchy.set_parent(child, parent));
    update();

    // Snapshotting must use current local values, not a stale cache.
    ExpectPositionNear(world(child), 7.0f, 3.0f, 0.0f);
}

TEST_F(TransformSystemTests, ReparentingPreservesWorldTransform)
{
    const auto first_parent = create_entity();
    const auto second_parent = create_entity();
    const auto child = create_entity();

    add_transform(first_parent, 10.0f, 0.0f, 0.0f);
    add_transform(second_parent, 20.0f, 0.0f, 0.0f);
    add_transform(child, 2.0f, 5.0f, 0.0f);

    update();

    ASSERT_TRUE(hierarchy.set_parent(child, first_parent));
    update();

    const auto original_world = world(child);
    const auto original_local = local(child);

    ASSERT_TRUE(hierarchy.set_parent(child, second_parent));
    update();

    EXPECT_EQ(hierarchy.get_parent(child), second_parent);

    ExpectMatrixNear(world(child), original_world);

    const auto expected_local = omega::inverse(world(second_parent)) * original_world;

    ExpectMatrixNear(local(child), expected_local);

    EXPECT_FALSE(local(child) == original_local);
}

TEST_F(TransformSystemTests, ClearingParentPreservesWorldTransform)
{
    const auto parent = create_entity();
    const auto child = create_entity();

    add_transform(parent, 10.0f, 0.0f, 0.0f);
    add_transform(child, 2.0f, 5.0f, 0.0f);

    update();

    ASSERT_TRUE(hierarchy.set_parent(child, parent));
    update();

    const auto original_world = world(child);

    ASSERT_TRUE(hierarchy.clear_parent(child));
    update();

    EXPECT_FALSE(hierarchy.has_parent(child));

    ExpectMatrixNear(world(child), original_world);
    ExpectMatrixNear(local(child), original_world);
}

TEST_F(TransformSystemTests, ClearingRootParentDoesNotChangeWorldTransform)
{
    const auto entity = create_entity();

    add_transform(entity, 3.0f, 4.0f, 5.0f);
    update();

    const auto original_world = world(entity);

    ASSERT_TRUE(hierarchy.clear_parent(entity));
    update();

    EXPECT_FALSE(hierarchy.has_parent(entity));

    ExpectMatrixNear(world(entity), original_world);
    ExpectMatrixNear(local(entity), original_world);
}

TEST_F(TransformSystemTests, SettingSameParentDoesNotChangeTransform)
{
    const auto parent = create_entity();
    const auto child = create_entity();

    add_transform(parent, 10.0f, 0.0f, 0.0f);
    add_transform(child, 2.0f, 0.0f, 0.0f);

    update();

    ASSERT_TRUE(hierarchy.set_parent(child, parent));
    update();

    const auto original_world = world(child);
    const auto original_local = local(child);

    ASSERT_TRUE(hierarchy.set_parent(child, parent));
    update();

    ExpectMatrixNear(world(child), original_world);
    ExpectMatrixNear(local(child), original_local);
}

TEST_F(TransformSystemTests, ReparentingPreservesDescendantWorldTransforms)
{
    const auto first_parent = create_entity();
    const auto second_parent = create_entity();
    const auto child = create_entity();
    const auto grandchild = create_entity();

    add_transform(first_parent, 10.0f, 0.0f, 0.0f);
    add_transform(second_parent, -5.0f, 0.0f, 0.0f);

    add_transform(child, 2.0f, 3.0f, 0.0f);
    add_transform(grandchild, 0.0f, 4.0f, 0.0f);

    update();

    ASSERT_TRUE(hierarchy.set_parent(child, first_parent));
    ASSERT_TRUE(hierarchy.set_parent(grandchild, child));
    update();

    const auto child_world = world(child);
    const auto grandchild_world = world(grandchild);
    const auto grandchild_local = local(grandchild);

    ASSERT_TRUE(hierarchy.set_parent(child, second_parent));
    update();

    ExpectMatrixNear(world(child), child_world);
    ExpectMatrixNear(world(grandchild), grandchild_world);

    // The child's local matrix compensates for reparenting.
    // The grandchild's local matrix should not change.
    ExpectMatrixNear(local(grandchild), grandchild_local);
}

TEST_F(TransformSystemTests, ReparentingToDescendantIsRejected)
{
    const auto parent = create_entity();
    const auto child = create_entity();

    add_transform(parent, 1.0f, 0.0f, 0.0f);
    add_transform(child, 2.0f, 0.0f, 0.0f);

    update();

    ASSERT_TRUE(hierarchy.set_parent(child, parent));
    update();

    const auto parent_world = world(parent);
    const auto child_world = world(child);

    EXPECT_FALSE(hierarchy.set_parent(parent, child));

    update();

    ExpectMatrixNear(world(parent), parent_world);
    ExpectMatrixNear(world(child), child_world);
}

// -----------------------------------------------------------------------------
// Batched hierarchy operations
// -----------------------------------------------------------------------------

TEST_F(TransformSystemTests, MultipleReparentsInOneFramePreserveWorld)
{
    const auto first_parent = create_entity();
    const auto second_parent = create_entity();
    const auto child = create_entity();

    add_transform(first_parent, 10.0f, 0.0f, 0.0f);
    add_transform(second_parent, -5.0f, 0.0f, 0.0f);
    add_transform(child, 2.0f, 3.0f, 0.0f);

    update();

    const auto original_world = world(child);

    ASSERT_TRUE(hierarchy.set_parent(child, first_parent));
    ASSERT_TRUE(hierarchy.set_parent(child, second_parent));

    update();

    EXPECT_EQ(hierarchy.get_parent(child), second_parent);

    ExpectMatrixNear(world(child), original_world);

    const auto expected_local = omega::inverse(world(second_parent)) * original_world;

    ExpectMatrixNear(local(child), expected_local);
}

TEST_F(TransformSystemTests, ParentAndChildReparentedInSameFramePreserveWorld)
{
    const auto first_root = create_entity();
    const auto second_root = create_entity();
    const auto parent = create_entity();
    const auto child = create_entity();

    add_transform(first_root, 10.0f, 0.0f, 0.0f);
    add_transform(second_root, -10.0f, 0.0f, 0.0f);

    add_transform(parent, 2.0f, 3.0f, 0.0f);
    add_transform(child, 1.0f, 5.0f, 0.0f);

    update();

    ASSERT_TRUE(hierarchy.set_parent(parent, first_root));
    ASSERT_TRUE(hierarchy.set_parent(child, parent));

    update();

    const auto original_parent_world = world(parent);
    const auto original_child_world = world(child);

    // Both changes are requested before the next update.
    ASSERT_TRUE(hierarchy.set_parent(parent, second_root));
    ASSERT_TRUE(hierarchy.clear_parent(child));

    update();

    ExpectMatrixNear(world(parent), original_parent_world);
    ExpectMatrixNear(world(child), original_child_world);

    EXPECT_EQ(hierarchy.get_parent(parent), second_root);
    EXPECT_FALSE(hierarchy.has_parent(child));
}

TEST_F(TransformSystemTests, ReparentingAndParentMovementInSameFrame)
{
    const auto first_parent = create_entity();
    const auto second_parent = create_entity();
    const auto child = create_entity();

    add_transform(first_parent, 10.0f, 0.0f, 0.0f);
    add_transform(second_parent, -5.0f, 0.0f, 0.0f);
    add_transform(child, 2.0f, 3.0f, 0.0f);

    update();

    ASSERT_TRUE(hierarchy.set_parent(child, first_parent));
    update();

    // Modify a prospective parent and reparent in the same frame.
    set_position(second_parent, 20.0f, 0.0f, 0.0f);

    // The child's current world transform is still independent
    // of second_parent before reparenting.
    const auto original_world = world(child);

    ASSERT_TRUE(hierarchy.set_parent(child, second_parent));
    update();

    ExpectMatrixNear(world(child), original_world);

    const auto expected_local = omega::inverse(world(second_parent)) * original_world;

    ExpectMatrixNear(local(child), expected_local);
}

// -----------------------------------------------------------------------------
// Missing Transform components
// -----------------------------------------------------------------------------

TEST_F(TransformSystemTests, EntityWithoutTransformInheritsParentWorld)
{
    const auto parent = create_entity();
    const auto child = create_entity();

    add_transform(parent, 10.0f, 5.0f, 2.0f);

    update();

    ASSERT_TRUE(hierarchy.set_parent(child, parent));
    update();

    ExpectMatrixNear(world(child), world(parent));
}

TEST_F(TransformSystemTests, EntityWithoutTransformPropagatesAncestorMovement)
{
    const auto root = create_entity();
    const auto intermediate = create_entity();
    const auto child = create_entity();

    add_transform(root);
    // intermediate intentionally has no Transform.
    add_transform(child, 0.0f, 2.0f, 0.0f);

    update();

    ASSERT_TRUE(hierarchy.set_parent(intermediate, root));
    ASSERT_TRUE(hierarchy.set_parent(child, intermediate));
    update();

    world(child);

    set_position(root, 10.0f, 0.0f, 0.0f);
    update();

    ExpectPositionNear(world(intermediate), 10.0f, 0.0f, 0.0f);
    ExpectPositionNear(world(child), 10.0f, 2.0f, 0.0f);
}

TEST_F(TransformSystemTests, ReparentingEntityWithoutTransformUpdatesInheritedWorld)
{
    const auto first_parent = create_entity();
    const auto second_parent = create_entity();
    const auto child = create_entity();

    add_transform(first_parent, 10.0f, 0.0f, 0.0f);
    add_transform(second_parent, 20.0f, 0.0f, 0.0f);

    update();

    ASSERT_TRUE(hierarchy.set_parent(child, first_parent));
    update();

    ExpectPositionNear(world(child), 10.0f, 0.0f, 0.0f);

    ASSERT_TRUE(hierarchy.set_parent(child, second_parent));
    update();

    // Without a local Transform, the entity cannot compensate
    // for a new parent and inherits its new parent's matrix.
    ExpectPositionNear(world(child), 20.0f, 0.0f, 0.0f);
}

// -----------------------------------------------------------------------------
// Component removal
// -----------------------------------------------------------------------------

TEST_F(TransformSystemTests, RemovingTransformResetsWorldMatrix)
{
    const auto entity = create_entity();

    add_transform(entity, 5.0f, 0.0f, 0.0f);
    update();

    world(entity);

    ASSERT_TRUE(components.remove<omega::Transform>(entity));
    update();

    ExpectMatrixNear(world(entity), omega::Matrix4(1.0f));
}

TEST_F(TransformSystemTests, RemovingParentTransformUpdatesChildInOneFrame)
{
    const auto parent = create_entity();
    const auto child = create_entity();

    add_transform(parent, 10.0f, 0.0f, 0.0f);
    add_transform(child, 2.0f, 5.0f, 0.0f);

    update();

    ASSERT_TRUE(hierarchy.set_parent(child, parent));
    update();

    // Give the child a known local transform while parented.
    set_position(child, 2.0f, 5.0f, 0.0f);
    update();

    ExpectPositionNear(world(child), 12.0f, 5.0f, 0.0f);

    ASSERT_TRUE(components.remove<omega::Transform>(parent));
    update();

    // The parent still exists, but now contributes identity.
    ExpectPositionNear(world(child), 2.0f, 5.0f, 0.0f);
}

TEST_F(TransformSystemTests, RemovingOneTransformKeepsOtherCacheEntriesValid)
{
    const auto first = create_entity();
    const auto second = create_entity();
    const auto third = create_entity();

    add_transform(first, 1.0f, 0.0f, 0.0f);
    add_transform(second, 2.0f, 0.0f, 0.0f);
    add_transform(third, 3.0f, 0.0f, 0.0f);

    update();

    // Populate all cache entries.
    world(first);
    world(second);
    world(third);

    ASSERT_TRUE(components.remove<omega::Transform>(second));
    update();

    ExpectPositionNear(world(first), 1.0f, 0.0f, 0.0f);
    ExpectPositionNear(world(third), 3.0f, 0.0f, 0.0f);

    ExpectMatrixNear(world(second), omega::Matrix4(1.0f));
}

// -----------------------------------------------------------------------------
// Entity destruction
// -----------------------------------------------------------------------------

TEST_F(TransformSystemTests, DestroyedEntityReturnsIdentityMatrix)
{
    const auto entity = create_entity();

    add_transform(entity, 5.0f, 0.0f, 0.0f);
    update();

    world(entity);

    ASSERT_TRUE(entities.destroy_entity(entity));
    update();

    ExpectMatrixNear(world(entity), omega::Matrix4(1.0f));
}

TEST_F(TransformSystemTests, DestroyingParentPreservesChildWorld)
{
    const auto parent = create_entity();
    const auto child = create_entity();

    add_transform(parent, 10.0f, 0.0f, 0.0f);
    add_transform(child, 2.0f, 5.0f, 0.0f);

    update();

    ASSERT_TRUE(hierarchy.set_parent(child, parent));
    update();

    const auto original_world = world(child);

    ASSERT_TRUE(entities.destroy_entity(parent));
    update();

    ASSERT_TRUE(entities.is_alive(child));
    EXPECT_FALSE(hierarchy.has_parent(child));

    ExpectMatrixNear(world(child), original_world);
    ExpectMatrixNear(local(child), original_world);
}

TEST_F(TransformSystemTests, DestroyingParentPreservesUncachedChildWorld)
{
    const auto parent = create_entity();
    const auto child = create_entity();

    add_transform(parent, 10.0f, 0.0f, 0.0f);
    add_transform(child, 2.0f, 5.0f, 0.0f);

    update();

    ASSERT_TRUE(hierarchy.set_parent(child, parent));
    update();

    // Change the child's local transform and leave its world
    // cache untouched before destruction.
    set_position(child, 3.0f, 4.0f, 0.0f);

    const auto expected_world = world(parent) * local(child);

    ASSERT_TRUE(entities.destroy_entity(parent));
    update();

    ASSERT_TRUE(entities.is_alive(child));

    ExpectMatrixNear(world(child), expected_world);
    ExpectMatrixNear(local(child), expected_world);
}

TEST_F(TransformSystemTests, DestroyingMiddleAncestorPreservesWorld)
{
    const auto root = create_entity();
    const auto middle = create_entity();
    const auto child = create_entity();

    add_transform(root, 10.0f, 0.0f, 0.0f);
    add_transform(middle, 0.0f, 5.0f, 0.0f);
    add_transform(child, 0.0f, 0.0f, 3.0f);

    update();

    ASSERT_TRUE(hierarchy.set_parent(middle, root));
    ASSERT_TRUE(hierarchy.set_parent(child, middle));
    update();

    const auto original_world = world(child);

    ASSERT_TRUE(entities.destroy_entity(middle));
    update();

    ASSERT_TRUE(entities.is_alive(child));

    // HierarchySystem moves surviving children to the nearest
    // surviving ancestor.
    EXPECT_EQ(hierarchy.get_parent(child), root);

    ExpectMatrixNear(world(child), original_world);

    const auto expected_local = omega::inverse(world(root)) * original_world;

    ExpectMatrixNear(local(child), expected_local);
}

TEST_F(TransformSystemTests, DestroyingParentPreservesGrandchildWorld)
{
    const auto parent = create_entity();
    const auto child = create_entity();
    const auto grandchild = create_entity();

    add_transform(parent, 10.0f, 0.0f, 0.0f);
    add_transform(child, 0.0f, 5.0f, 0.0f);
    add_transform(grandchild, 0.0f, 0.0f, 3.0f);

    update();

    ASSERT_TRUE(hierarchy.set_parent(child, parent));
    ASSERT_TRUE(hierarchy.set_parent(grandchild, child));
    update();

    const auto child_world = world(child);
    const auto grandchild_world = world(grandchild);
    const auto grandchild_local = local(grandchild);

    ASSERT_TRUE(entities.destroy_entity(parent));
    update();

    ASSERT_TRUE(entities.is_alive(child));
    ASSERT_TRUE(entities.is_alive(grandchild));

    ExpectMatrixNear(world(child), child_world);
    ExpectMatrixNear(world(grandchild), grandchild_world);

    ExpectMatrixNear(local(grandchild), grandchild_local);
}

TEST_F(TransformSystemTests, DestroyingMultipleAncestorsPreservesSurvivorWorld)
{
    const auto grandparent = create_entity();
    const auto parent = create_entity();
    const auto child = create_entity();

    add_transform(grandparent, 10.0f, 0.0f, 0.0f);
    add_transform(parent, 0.0f, 5.0f, 0.0f);
    add_transform(child, 0.0f, 0.0f, 3.0f);

    update();

    ASSERT_TRUE(hierarchy.set_parent(parent, grandparent));
    ASSERT_TRUE(hierarchy.set_parent(child, parent));
    update();

    const auto original_world = world(child);

    ASSERT_TRUE(entities.destroy_entity(grandparent));
    ASSERT_TRUE(entities.destroy_entity(parent));

    update();

    ASSERT_TRUE(entities.is_alive(child));
    EXPECT_FALSE(hierarchy.has_parent(child));

    ExpectMatrixNear(world(child), original_world);
    ExpectMatrixNear(local(child), original_world);
}

TEST_F(TransformSystemTests, DestroyingUnrelatedEntityDoesNotChangeWorld)
{
    const auto first = create_entity();
    const auto second = create_entity();

    add_transform(first, 1.0f, 2.0f, 3.0f);
    add_transform(second, 4.0f, 5.0f, 6.0f);

    update();

    const auto original_second_world = world(second);

    ASSERT_TRUE(entities.destroy_entity(first));
    update();

    ASSERT_TRUE(entities.is_alive(second));

    ExpectMatrixNear(world(second), original_second_world);
}

} // namespace
