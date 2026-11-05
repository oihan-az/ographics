#include <omega/scene/scene.hpp>

#include <gtest/gtest.h>

TEST(SceneTests, EntityLifecycle)
{
    omega::Scene scene;

    // Create an entity (creation is deferred)
    omega::Entity entity = scene.create_entity();

    EXPECT_TRUE(entity.is_valid());
    EXPECT_FALSE(scene.is_alive(entity));
    EXPECT_EQ(scene.entity_count(), 0);

    // Flush creation
    scene.flush_commands();

    EXPECT_TRUE(scene.is_alive(entity));
    EXPECT_EQ(scene.entity_count(), 1);

    // Destroy an entity (destruction is deferred)
    EXPECT_TRUE(scene.destroy_entity(entity));

    EXPECT_TRUE(scene.is_alive(entity));
    EXPECT_EQ(scene.entity_count(), 1);

    // Flush destruction
    scene.flush_commands();

    EXPECT_FALSE(scene.is_alive(entity));
    EXPECT_EQ(scene.entity_count(), 0);

    // Reuse its slot
    omega::Entity reused = scene.create_entity();

    EXPECT_EQ(reused.index, entity.index);
    EXPECT_NE(reused.generation, entity.generation);
    EXPECT_FALSE(scene.is_alive(reused));

    scene.flush_commands();

    EXPECT_TRUE(scene.is_alive(reused));
    EXPECT_FALSE(scene.is_alive(entity));
    EXPECT_EQ(scene.entity_count(), 1);

    // Create and destroy before flushing
    omega::Entity transient = scene.create_entity();

    EXPECT_TRUE(scene.destroy_entity(transient));

    scene.flush_commands();

    EXPECT_FALSE(scene.is_alive(transient));
    EXPECT_EQ(scene.entity_count(), 1);

    // Destroying an already destroyed entity must fail
    EXPECT_FALSE(scene.destroy_entity(transient));
}