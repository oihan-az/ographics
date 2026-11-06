#include <gtest/gtest.h>

#include <omega/core/event/event_system.hpp>
#include <omega/core/math/matrix4.hpp>
#include <omega/core/scene/scene.hpp>
#include <omega/transform/transform.hpp>
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
    omega::Scene scene(events);

    auto entity = scene.create_entity();
    scene.flush_commands();

    omega::Transform transform;
    transform.position = omega::Vector3(1.0f, 2.0f, 3.0f);

    ASSERT_TRUE(scene.add(entity, transform));

    omega::TransformSystem transforms(scene, events);
    events.dispatch();

    ExpectMatrixNear(transforms.get_world_matrix(entity), transform.matrix());
}

// Checks that a child world matrix combines its parent's and its local matrix
TEST(TransformSystemTests, ChildWorldMatrixIncludesParentTransform)
{
    omega::EventSystem events;
    omega::Scene scene(events);

    auto parent = scene.create_entity();
    auto child = scene.create_entity();
    scene.flush_commands();

    omega::Transform parent_transform;
    parent_transform.position = omega::Vector3(10.0f, 0.0f, 0.0f);

    omega::Transform child_transform;
    child_transform.position = omega::Vector3(0.0f, 5.0f, 0.0f);

    ASSERT_TRUE(scene.add(parent, parent_transform));
    ASSERT_TRUE(scene.add(child, child_transform));
    ASSERT_TRUE(scene.set_parent(child, parent));

    omega::TransformSystem transforms(scene, events);
    events.dispatch();

    auto expected = parent_transform.matrix() * child_transform.matrix();

    ExpectMatrixNear(transforms.get_world_matrix(child), expected);
}

// Checks that world transforms include all ancestors
TEST(TransformSystemTests, NestedChildWorldMatrixIncludesAllAncestors)
{
    omega::EventSystem events;
    omega::Scene scene(events);

    auto grandparent = scene.create_entity();
    auto parent = scene.create_entity();
    auto child = scene.create_entity();
    scene.flush_commands();

    omega::Transform grandparent_transform;
    grandparent_transform.position = omega::Vector3(10.0f, 0.0f, 0.0f);

    omega::Transform parent_transform;
    parent_transform.position = omega::Vector3(0.0f, 5.0f, 0.0f);

    omega::Transform child_transform;
    child_transform.position = omega::Vector3(0.0f, 0.0f, 3.0f);

    ASSERT_TRUE(scene.add(grandparent, grandparent_transform));
    ASSERT_TRUE(scene.add(parent, parent_transform));
    ASSERT_TRUE(scene.add(child, child_transform));

    ASSERT_TRUE(scene.set_parent(parent, grandparent));
    ASSERT_TRUE(scene.set_parent(child, parent));

    omega::TransformSystem transforms(scene, events);
    events.dispatch();

    auto expected = grandparent_transform.matrix() * parent_transform.matrix() * child_transform.matrix();

    ExpectMatrixNear(transforms.get_world_matrix(child), expected);
}

// Checks that changing a local transform invalidates its cached world matrix
TEST(TransformSystemTests, ChangedTransformInvalidatesWorldMatrix)
{
    omega::EventSystem events;
    omega::Scene scene(events);

    auto entity = scene.create_entity();
    scene.flush_commands();

    omega::Transform transform;
    ASSERT_TRUE(scene.add(entity, transform));

    omega::TransformSystem transforms(scene, events);
    events.dispatch();

    auto initial = transforms.get_world_matrix(entity);

    auto* stored_transform = scene.get<omega::Transform>(entity);
    ASSERT_NE(stored_transform, nullptr);

    stored_transform->position = omega::Vector3(5.0f, 2.0f, 1.0f);

    ASSERT_TRUE(scene.mark_dirty<omega::Transform>(entity));
    events.dispatch();

    auto updated = transforms.get_world_matrix(entity);

    EXPECT_FALSE(updated == initial);
    ExpectMatrixNear(updated, stored_transform->matrix());
}

// Checks that changing a parent's transform invalidates its descendants
TEST(TransformSystemTests, ChangedParentTransformInvalidatesChildren)
{
    omega::EventSystem events;
    omega::Scene scene(events);

    auto parent = scene.create_entity();
    auto child = scene.create_entity();
    scene.flush_commands();

    omega::Transform parent_transform;
    omega::Transform child_transform;

    child_transform.position = omega::Vector3(0.0f, 5.0f, 0.0f);

    ASSERT_TRUE(scene.add(parent, parent_transform));
    ASSERT_TRUE(scene.add(child, child_transform));
    ASSERT_TRUE(scene.set_parent(child, parent));

    omega::TransformSystem transforms(scene, events);
    events.dispatch();

    auto initial = transforms.get_world_matrix(child);

    auto* stored_parent = scene.get<omega::Transform>(parent);
    ASSERT_NE(stored_parent, nullptr);

    stored_parent->position = omega::Vector3(10.0f, 0.0f, 0.0f);

    ASSERT_TRUE(scene.mark_dirty<omega::Transform>(parent));
    events.dispatch();

    auto updated = transforms.get_world_matrix(child);
    auto expected = stored_parent->matrix() * child_transform.matrix();

    EXPECT_FALSE(updated == initial);
    ExpectMatrixNear(updated, expected);
}

// Checks that reparenting invalidates the child's world matrix
TEST(TransformSystemTests, ReparentingInvalidatesWorldMatrix)
{
    omega::EventSystem events;
    omega::Scene scene(events);

    auto first_parent = scene.create_entity();
    auto second_parent = scene.create_entity();
    auto child = scene.create_entity();
    scene.flush_commands();

    omega::Transform first_transform;
    first_transform.position = omega::Vector3(10.0f, 0.0f, 0.0f);

    omega::Transform second_transform;
    second_transform.position = omega::Vector3(20.0f, 0.0f, 0.0f);

    omega::Transform child_transform;
    child_transform.position = omega::Vector3(0.0f, 5.0f, 0.0f);

    ASSERT_TRUE(scene.add(first_parent, first_transform));
    ASSERT_TRUE(scene.add(second_parent, second_transform));
    ASSERT_TRUE(scene.add(child, child_transform));

    ASSERT_TRUE(scene.set_parent(child, first_parent));

    omega::TransformSystem transforms(scene, events);
    events.dispatch();

    auto initial = transforms.get_world_matrix(child);

    ASSERT_TRUE(scene.set_parent(child, second_parent));
    events.dispatch();

    auto updated = transforms.get_world_matrix(child);
    auto expected = second_transform.matrix() * child_transform.matrix();

    EXPECT_FALSE(updated == initial);
    ExpectMatrixNear(updated, expected);
}

// Checks that clearing a parent invalidates the child's world matrix
TEST(TransformSystemTests, ClearingParentInvalidatesWorldMatrix)
{
    omega::EventSystem events;
    omega::Scene scene(events);

    auto parent = scene.create_entity();
    auto child = scene.create_entity();
    scene.flush_commands();

    omega::Transform parent_transform;
    parent_transform.position = omega::Vector3(10.0f, 0.0f, 0.0f);

    omega::Transform child_transform;
    child_transform.position = omega::Vector3(0.0f, 5.0f, 0.0f);

    ASSERT_TRUE(scene.add(parent, parent_transform));
    ASSERT_TRUE(scene.add(child, child_transform));
    ASSERT_TRUE(scene.set_parent(child, parent));

    omega::TransformSystem transforms(scene, events);
    events.dispatch();

    transforms.get_world_matrix(child);

    ASSERT_TRUE(scene.clear_parent(child));
    events.dispatch();

    ExpectMatrixNear(transforms.get_world_matrix(child), child_transform.matrix());
}

// Checks that removing a Transform invalidates its cached matrix
TEST(TransformSystemTests, RemovingTransformResetsWorldMatrix)
{
    omega::EventSystem events;
    omega::Scene scene(events);

    auto entity = scene.create_entity();
    scene.flush_commands();

    omega::Transform transform;
    transform.position = omega::Vector3(5.0f, 0.0f, 0.0f);

    ASSERT_TRUE(scene.add(entity, transform));

    omega::TransformSystem transforms(scene, events);
    events.dispatch();

    transforms.get_world_matrix(entity);

    ASSERT_TRUE(scene.remove<omega::Transform>(entity));
    events.dispatch();

    ExpectMatrixNear(transforms.get_world_matrix(entity), omega::Matrix4(1.0f));
}

// Checks that an entity without a Transform uses an identity local matrix
TEST(TransformSystemTests, EntityWithoutTransformUsesIdentityMatrix)
{
    omega::EventSystem events;
    omega::Scene scene(events);

    auto entity = scene.create_entity();
    scene.flush_commands();

    omega::TransformSystem transforms(scene, events);
    events.dispatch();

    ExpectMatrixNear(transforms.get_world_matrix(entity), omega::Matrix4(1.0f));
}

// Checks that destroying a parent invalidates its children's world matrices
TEST(TransformSystemTests, DestroyingParentInvalidatesChildWorldMatrix)
{
    omega::EventSystem events;
    omega::Scene scene(events);

    auto parent = scene.create_entity();
    auto child = scene.create_entity();
    scene.flush_commands();

    omega::Transform parent_transform;
    parent_transform.position = omega::Vector3(10.0f, 0.0f, 0.0f);

    omega::Transform child_transform;
    child_transform.position = omega::Vector3(0.0f, 5.0f, 0.0f);

    ASSERT_TRUE(scene.add(parent, parent_transform));
    ASSERT_TRUE(scene.add(child, child_transform));
    ASSERT_TRUE(scene.set_parent(child, parent));

    omega::TransformSystem transforms(scene, events);
    events.dispatch();

    transforms.get_world_matrix(child);

    scene.destroy_entity(parent);
    scene.flush_commands();
    events.dispatch();

    EXPECT_TRUE(scene.is_alive(child));

    ExpectMatrixNear(transforms.get_world_matrix(child), child_transform.matrix());
}