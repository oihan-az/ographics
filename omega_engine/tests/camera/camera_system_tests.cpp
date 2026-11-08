#include <gtest/gtest.h>

#include <omega/camera/camera.hpp>
#include <omega/camera/matrix_camera.hpp>
#include <omega/camera/camera_system.hpp>

#include <omega/core/event/event_system.hpp>
#include <omega/core/math/matrix_projection.hpp>
#include <omega/core/entity/entity_system.hpp>
#include <omega/core/component/component_system.hpp>
#include <omega/core/hierarchy/hierarchy_system.hpp>

#include <omega/transform/transform.hpp>
#include <omega/transform/transform_system.hpp>
#include <omega/transform/matrix_transform.hpp>

#include <cmath>

namespace
{

constexpr float EPSILON = 0.0001f;

bool matrix_near(const omega::Matrix4& a, const omega::Matrix4& b)
{
    for (int column = 0; column < 4; ++column)
    {
        for (int row = 0; row < 4; ++row)
        {
            if (std::abs(a[column][row] - b[column][row]) > EPSILON)
                return false;
        }
    }

    return true;
}

class CameraSystemTests : public testing::Test
{
  protected:
    omega::EventSystem events;
    omega::EntitySystem entities{events};
    omega::ComponentSystem components{events, entities};
    omega::HierarchySystem hierarchy{events, entities};
    omega::TransformSystem transforms{events, components, hierarchy, entities};
    omega::CameraSystem cameras{events, components, hierarchy, entities, transforms};

    omega::Entity create_entity()
    {
        omega::Entity entity = entities.create_entity();
        events.swap_all_buffers();
        entities.update();
        return entity;
    }

    // Process the deterministic frame timeline sequence for all subsystems
    void dispatch_events()
    {
        events.swap_all_buffers();
        components.update();
        hierarchy.update();
        transforms.update();
        cameras.update();
    }
};

TEST_F(CameraSystemTests, DeadEntityReturnsIdentityData)
{
    omega::Entity entity;

    omega::CameraData data = cameras.get_camera_data(entity);

    EXPECT_TRUE(matrix_near(data.view, omega::Matrix4(1.0f)));
    EXPECT_TRUE(matrix_near(data.projection, omega::Matrix4(1.0f)));
    EXPECT_TRUE(matrix_near(data.view_projection, omega::Matrix4(1.0f)));
}

TEST_F(CameraSystemTests, EntityWithoutCameraReturnsIdentityData)
{
    omega::Entity entity = create_entity();

    omega::CameraData data = cameras.get_camera_data(entity);

    EXPECT_TRUE(matrix_near(data.view, omega::Matrix4(1.0f)));
    EXPECT_TRUE(matrix_near(data.projection, omega::Matrix4(1.0f)));
    EXPECT_TRUE(matrix_near(data.view_projection, omega::Matrix4(1.0f)));
}

TEST_F(CameraSystemTests, PerspectiveCameraCalculatesProjection)
{
    omega::Entity entity = create_entity();

    omega::PerspectiveCamera camera;
    camera.proj.field_of_view = omega::radians(75.0f);
    camera.proj.aspect_ratio = 1.5f;
    camera.proj.near_plane = 0.5f;
    camera.proj.far_plane = 500.0f;

    ASSERT_TRUE(components.add(entity, camera));
    dispatch_events();

    omega::CameraData data = cameras.get_camera_data(entity);

    EXPECT_TRUE(matrix_near(data.projection, omega::mtx_perspective_projection(camera)));
}

TEST_F(CameraSystemTests, OrthogonalCameraCalculatesProjection)
{
    omega::Entity entity = create_entity();

    omega::OrthogonalCamera camera;
    camera.proj.left = -20.0f;
    camera.proj.right = 20.0f;
    camera.proj.bottom = -15.0f;
    camera.proj.top = 15.0f;
    camera.proj.near_plane = 0.5f;
    camera.proj.far_plane = 200.0f;

    ASSERT_TRUE(components.add(entity, camera));
    dispatch_events();

    omega::CameraData data = cameras.get_camera_data(entity);

    EXPECT_TRUE(matrix_near(data.projection, omega::orthogonal_projection(camera.proj)));
}

TEST_F(CameraSystemTests, ViewMatrixIsIdentityWithoutTransform)
{
    omega::Entity entity = create_entity();

    ASSERT_TRUE(components.add(entity, omega::PerspectiveCamera{}));
    dispatch_events();

    omega::CameraData data = cameras.get_camera_data(entity);

    EXPECT_TRUE(matrix_near(data.view, omega::Matrix4(1.0f)));
}

TEST_F(CameraSystemTests, ViewMatrixIsInverseOfWorldTransform)
{
    omega::Entity entity = create_entity();

    omega::Transform transform;
    transform.position = omega::Vector3(5.0f, 3.0f, -8.0f);

    ASSERT_TRUE(components.add(entity, transform));
    ASSERT_TRUE(components.add(entity, omega::PerspectiveCamera{}));
    dispatch_events();

    omega::CameraData data = cameras.get_camera_data(entity);

    omega::Matrix4 world_matrix = omega::mtx_transform(transform);

    EXPECT_TRUE(matrix_near(data.view, omega::inverse(world_matrix)));
}

TEST_F(CameraSystemTests, ViewProjectionIsProjectionTimesView)
{
    omega::Entity entity = create_entity();

    omega::Transform transform;
    transform.position = omega::Vector3(2.0f, 4.0f, 6.0f);

    omega::PerspectiveCamera camera;

    ASSERT_TRUE(components.add(entity, transform));
    ASSERT_TRUE(components.add(entity, camera));
    dispatch_events();

    omega::CameraData data = cameras.get_camera_data(entity);

    EXPECT_TRUE(matrix_near(data.view_projection, data.projection * data.view));
}

TEST_F(CameraSystemTests, CameraUsesParentWorldTransform)
{
    omega::Entity parent = create_entity();
    omega::Entity camera_entity = create_entity();

    omega::Transform parent_transform;
    parent_transform.position = omega::Vector3(10.0f, 0.0f, 0.0f);

    ASSERT_TRUE(components.add(parent, parent_transform));
    ASSERT_TRUE(components.add(camera_entity, omega::PerspectiveCamera{}));
    ASSERT_TRUE(hierarchy.set_parent(camera_entity, parent));

    dispatch_events();

    omega::CameraData data = cameras.get_camera_data(camera_entity);

    omega::Matrix4 expected_world = omega::mtx_transform(parent_transform);

    EXPECT_TRUE(matrix_near(data.view, omega::inverse(expected_world)));
}

TEST_F(CameraSystemTests, ParentTransformChangeInvalidatesCameraCache)
{
    omega::Entity parent = create_entity();
    omega::Entity camera_entity = create_entity();

    omega::Transform parent_transform;
    parent_transform.position = omega::Vector3(5.0f, 0.0f, 0.0f);

    ASSERT_TRUE(components.add(parent, parent_transform));
    ASSERT_TRUE(components.add(camera_entity, omega::PerspectiveCamera{}));
    ASSERT_TRUE(hierarchy.set_parent(camera_entity, parent));

    dispatch_events();

    omega::CameraData first_data = cameras.get_camera_data(camera_entity);

    omega::Transform* stored_transform = components.get<omega::Transform>(parent);
    ASSERT_NE(stored_transform, nullptr);

    stored_transform->position.x = 15.0f;

    ASSERT_TRUE(components.mark_dirty<omega::Transform>(parent));
    dispatch_events();

    omega::CameraData second_data = cameras.get_camera_data(camera_entity);

    EXPECT_FALSE(matrix_near(first_data.view, second_data.view));

    omega::Matrix4 expected_world = omega::mtx_transform(*stored_transform);

    EXPECT_TRUE(matrix_near(second_data.view, omega::inverse(expected_world)));
}

TEST_F(CameraSystemTests, CameraTransformChangeInvalidatesCache)
{
    omega::Entity entity = create_entity();

    omega::Transform transform;

    ASSERT_TRUE(components.add(entity, transform));
    ASSERT_TRUE(components.add(entity, omega::PerspectiveCamera{}));
    dispatch_events();

    omega::CameraData first_data = cameras.get_camera_data(entity);

    omega::Transform* stored_transform = components.get<omega::Transform>(entity);
    ASSERT_NE(stored_transform, nullptr);

    stored_transform->position.x = 10.0f;

    ASSERT_TRUE(components.mark_dirty<omega::Transform>(entity));
    dispatch_events();

    omega::CameraData second_data = cameras.get_camera_data(entity);

    EXPECT_FALSE(matrix_near(first_data.view, second_data.view));
}

TEST_F(CameraSystemTests, CameraProjectionChangeInvalidatesCache)
{
    omega::Entity entity = create_entity();

    ASSERT_TRUE(components.add(entity, omega::PerspectiveCamera{}));
    dispatch_events();

    omega::CameraData first_data = cameras.get_camera_data(entity);

    omega::PerspectiveCamera* camera = components.get<omega::PerspectiveCamera>(entity);

    ASSERT_NE(camera, nullptr);

    camera->proj.field_of_view = omega::radians(90.0f);

    ASSERT_TRUE(components.mark_dirty<omega::PerspectiveCamera>(entity));
    dispatch_events();

    omega::CameraData second_data = cameras.get_camera_data(entity);

    EXPECT_FALSE(matrix_near(first_data.projection, second_data.projection));
}

TEST_F(CameraSystemTests, CameraRemovalInvalidatesCachedData)
{
    omega::Entity entity = create_entity();

    ASSERT_TRUE(components.add(entity, omega::PerspectiveCamera{}));
    dispatch_events();

    omega::CameraData first_data = cameras.get_camera_data(entity);

    EXPECT_FALSE(matrix_near(first_data.projection, omega::Matrix4(1.0f)));

    ASSERT_TRUE(components.remove<omega::PerspectiveCamera>(entity));
    dispatch_events();

    omega::CameraData second_data = cameras.get_camera_data(entity);

    EXPECT_TRUE(matrix_near(second_data.view, omega::Matrix4(1.0f)));
    EXPECT_TRUE(matrix_near(second_data.projection, omega::Matrix4(1.0f)));
    EXPECT_TRUE(matrix_near(second_data.view_projection, omega::Matrix4(1.0f)));
}

TEST_F(CameraSystemTests, ReparentingInvalidatesCameraCache)
{
    omega::Entity parent_a = create_entity();
    omega::Entity parent_b = create_entity();
    omega::Entity camera_entity = create_entity();

    omega::Transform transform_a;
    transform_a.position = omega::Vector3(5.0f, 0.0f, 0.0f);

    omega::Transform transform_b;
    transform_b.position = omega::Vector3(20.0f, 0.0f, 0.0f);

    ASSERT_TRUE(components.add(parent_a, transform_a));
    ASSERT_TRUE(components.add(parent_b, transform_b));
    ASSERT_TRUE(components.add(camera_entity, omega::PerspectiveCamera{}));

    ASSERT_TRUE(hierarchy.set_parent(camera_entity, parent_a));
    dispatch_events();

    omega::CameraData first_data = cameras.get_camera_data(camera_entity);

    ASSERT_TRUE(hierarchy.set_parent(camera_entity, parent_b));

    // Process the structural hierarchy change event through the data-driven systems
    dispatch_events();

    omega::CameraData second_data = cameras.get_camera_data(camera_entity);

    EXPECT_FALSE(matrix_near(first_data.view, second_data.view));

    EXPECT_TRUE(matrix_near(second_data.view, omega::inverse(omega::mtx_transform(transform_b))));
}

TEST_F(CameraSystemTests, DestroyingCameraEntityReturnsIdentityData)
{
    omega::Entity entity = create_entity();

    ASSERT_TRUE(components.add(entity, omega::PerspectiveCamera{}));
    dispatch_events();

    omega::CameraData first_data = cameras.get_camera_data(entity);

    EXPECT_FALSE(matrix_near(first_data.projection, omega::Matrix4(1.0f)));

    entities.destroy_entity(entity);
    entities.update();

    // Batch process entity destruction and component removal events across all pipelines
    dispatch_events();

    omega::CameraData second_data = cameras.get_camera_data(entity);

    EXPECT_TRUE(matrix_near(second_data.view, omega::Matrix4(1.0f)));
    EXPECT_TRUE(matrix_near(second_data.projection, omega::Matrix4(1.0f)));
    EXPECT_TRUE(matrix_near(second_data.view_projection, omega::Matrix4(1.0f)));
}


} // namespace
