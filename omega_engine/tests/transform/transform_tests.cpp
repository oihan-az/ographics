#include <gtest/gtest.h>

#include <omega/transform/transform.hpp>
#include <omega/scene/scene.hpp>
#include <omega/math/constants.hpp>

namespace omega
{

TEST(TransformTests, DefaultValues)
{
    Transform transform;

    EXPECT_EQ(transform.position, Vector3(0.0f));
    EXPECT_EQ(transform.rotation, Quaternion(1.0f, 0.0f, 0.0f, 0.0f));
    EXPECT_EQ(transform.scale, Vector3(1.0f));
}

TEST(TransformTests, CustomValues)
{
    Transform transform{Vector3(1.0f, 2.0f, 3.0f), Quaternion(1.0f, 0.0f, 0.0f, 0.0f), Vector3(2.0f, 2.0f, 2.0f)};

    EXPECT_EQ(transform.position, Vector3(1.0f, 2.0f, 3.0f));
    EXPECT_EQ(transform.scale, Vector3(2.0f));
}

TEST(TransformTests, TransformComponent)
{
    Scene scene;

    Entity entity = scene.create_entity();
    scene.flush_commands();

    Transform transform;
    transform.position = Vector3(1.0f, 2.0f, 3.0f);

    ASSERT_TRUE(scene.add<Transform>(entity, transform));

    ASSERT_TRUE(scene.has<Transform>(entity));

    const auto* result = scene.get<Transform>(entity);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->position,Vector3(1.0f, 2.0f, 3.0f));
}

TEST(TransformTests, MatrixContainsTranslationAndScale)
{
    Transform transform;
    transform.position = Vector3(1.0f, 2.0f, 3.0f);
    transform.scale = Vector3(2.0f, 3.0f, 4.0f);

    Matrix4 matrix = transform.matrix();

    EXPECT_EQ(Vector3(matrix[3]), transform.position);
    EXPECT_EQ(Vector3(matrix[0]), Vector3(2.0f, 0.0f, 0.0f));
    EXPECT_EQ(Vector3(matrix[1]), Vector3(0.0f, 3.0f, 0.0f));
    EXPECT_EQ(Vector3(matrix[2]), Vector3(0.0f, 0.0f, 4.0f));
}

TEST(TransformTests, MatrixContainsRotation)
{
    Transform transform;
    transform.rotation = angle_axis(half_pi<float>(), Vector3(0.0f, 0.0f, 1.0f));

    Matrix4 matrix = transform.matrix();

    EXPECT_NEAR(matrix[0][0], 0.0f, 0.0001f);
    EXPECT_NEAR(matrix[0][1], 1.0f, 0.0001f);
    EXPECT_NEAR(matrix[1][0], -1.0f, 0.0001f);
    EXPECT_NEAR(matrix[1][1], 0.0f, 0.0001f);
}

TEST(TransformTests, MatrixCombinesTranslationRotationAndScale)
{
    Transform transform;
    transform.position = Vector3(1.0f, 2.0f, 3.0f);
    transform.rotation = angle_axis(pi<float>() / 2.0f, Vector3(0.0f, 0.0f, 1.0f));
    transform.scale = Vector3(2.0f, 3.0f, 4.0f);

    Matrix4 matrix = transform.matrix();

    EXPECT_NEAR(matrix[0][0], 0.0f, 0.0001f);
    EXPECT_NEAR(matrix[0][1], 2.0f, 0.0001f);
    EXPECT_NEAR(matrix[1][0], -3.0f, 0.0001f);
    EXPECT_NEAR(matrix[1][1], 0.0f, 0.0001f);

    EXPECT_NEAR(matrix[3][0], 1.0f, 0.0001f);
    EXPECT_NEAR(matrix[3][1], 2.0f, 0.0001f);
    EXPECT_NEAR(matrix[3][2], 3.0f, 0.0001f);
}

} // namespace omega