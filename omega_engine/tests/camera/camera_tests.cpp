#include <gtest/gtest.h>

#include <omega/camera/camera.hpp>
#include <omega/core/scene/scene.hpp>
#include <omega/core/math/matrix_projection.hpp>

TEST(CameraTests, PerspectiveCameraHasDefaultProjection)
{
    omega::PerspectiveCamera camera;

    EXPECT_FLOAT_EQ(camera.proj.field_of_view, omega::radians(60.0f));
    EXPECT_FLOAT_EQ(camera.proj.aspect_ratio, 16.0f / 9.0f);
    EXPECT_FLOAT_EQ(camera.proj.near_plane, 0.1f);
    EXPECT_FLOAT_EQ(camera.proj.far_plane, 1000.0f);
}

TEST(CameraTests, OrthogonalCameraHasDefaultProjection)
{
    omega::OrthogonalCamera camera;

    EXPECT_FLOAT_EQ(camera.proj.left, -10.0f);
    EXPECT_FLOAT_EQ(camera.proj.right, 10.0f);
    EXPECT_FLOAT_EQ(camera.proj.bottom, -10.0f);
    EXPECT_FLOAT_EQ(camera.proj.top, 10.0f);
    EXPECT_FLOAT_EQ(camera.proj.near_plane, 0.1f);
    EXPECT_FLOAT_EQ(camera.proj.far_plane, 1000.0f);
}

TEST(CameraTests, PerspectiveCameraCanBeAddedToScene)
{
    omega::Scene scene;
    const auto entity = scene.create_entity();
    scene.flush_commands();

    EXPECT_TRUE(scene.add(entity, omega::PerspectiveCamera{}));
    EXPECT_TRUE(scene.has<omega::PerspectiveCamera>(entity));
}

TEST(CameraTests, OrthogonalCameraCanBeAddedToScene)
{
    omega::Scene scene;
    const auto entity = scene.create_entity();
    scene.flush_commands();

    EXPECT_TRUE(scene.add(entity, omega::OrthogonalCamera{}));
    EXPECT_TRUE(scene.has<omega::OrthogonalCamera>(entity));
}

TEST(MatrixProjectionTests, CreatesPerspectiveProjection)
{
    omega::PerspectiveProjection projection;
    projection.field_of_view = omega::radians(90.0f);
    projection.aspect_ratio = 1.0f;
    projection.near_plane = 1.0f;
    projection.far_plane = 10.0f;

    const omega::Matrix4 matrix = omega::perspective_projection(projection);

    EXPECT_NEAR(matrix[0][0], 1.0f, 0.0001f);
    EXPECT_NEAR(matrix[1][1], 1.0f, 0.0001f);

#if OMEGA_CLIP_DEPTH_ZERO_TO_ONE
#if OMEGA_PROJECTION_LEFT_HANDED
    EXPECT_NEAR(matrix[2][2], 10.0f / 9.0f, 0.0001f);
    EXPECT_NEAR(matrix[3][2], -10.0f / 9.0f, 0.0001f);
#else
    EXPECT_NEAR(matrix[2][2], -10.0f / 9.0f, 0.0001f);
    EXPECT_NEAR(matrix[3][2], -10.0f / 9.0f, 0.0001f);
#endif
#else
#if OMEGA_PROJECTION_LEFT_HANDED
    EXPECT_NEAR(matrix[2][2], 11.0f / 9.0f, 0.0001f);
    EXPECT_NEAR(matrix[3][2], -20.0f / 9.0f, 0.0001f);
#else
    EXPECT_NEAR(matrix[2][2], -11.0f / 9.0f, 0.0001f);
    EXPECT_NEAR(matrix[3][2], -20.0f / 9.0f, 0.0001f);
#endif
#endif

    EXPECT_NEAR(matrix[2][3], OMEGA_PROJECTION_LEFT_HANDED ? 1.0f : -1.0f, 0.0001f);
}

TEST(MatrixProjectionTests, CreatesOrthogonalProjection)
{
    omega::OrthogonalProjection projection;
    projection.left = -2.0f;
    projection.right = 2.0f;
    projection.bottom = -2.0f;
    projection.top = 2.0f;
    projection.near_plane = 1.0f;
    projection.far_plane = 10.0f;

    const omega::Matrix4 matrix = omega::orthogonal_projection(projection);

    EXPECT_NEAR(matrix[0][0], 0.5f, 0.0001f);
    EXPECT_NEAR(matrix[1][1], 0.5f, 0.0001f);

#if OMEGA_CLIP_DEPTH_ZERO_TO_ONE
#if OMEGA_PROJECTION_LEFT_HANDED
    EXPECT_NEAR(matrix[2][2], 1.0f / 9.0f, 0.0001f);
    EXPECT_NEAR(matrix[3][2], -1.0f / 9.0f, 0.0001f);
#else
    EXPECT_NEAR(matrix[2][2], -1.0f / 9.0f, 0.0001f);
    EXPECT_NEAR(matrix[3][2], -1.0f / 9.0f, 0.0001f);
#endif
#else
    EXPECT_NEAR(matrix[2][2], OMEGA_PROJECTION_LEFT_HANDED ? 2.0f / 9.0f : -2.0f / 9.0f, 0.0001f);
    EXPECT_NEAR(matrix[3][2], -11.0f / 9.0f, 0.0001f);
#endif

    EXPECT_FLOAT_EQ(matrix[3][3], 1.0f);
}