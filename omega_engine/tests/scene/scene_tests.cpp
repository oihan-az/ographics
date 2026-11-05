#include <omega/scene/scene.hpp>

#include <gtest/gtest.h>

#include <typeindex>
#include <vector>

namespace
{

struct Position
{
    float x = 0.0f;
};

struct Velocity
{
    float x = 0.0f;
};

} // namespace

// Tests entity live cycle
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


// Tests adding, retrieving, modifying, and removing a component
TEST(SceneTests, ComponentLifecycle)
{
    omega::Scene scene;
    omega::Entity entity = scene.create_entity();

    scene.flush_commands();

    EXPECT_TRUE(scene.add<Position>(entity, {10.0f}));
    EXPECT_TRUE(scene.has<Position>(entity));

    Position* position = scene.get<Position>(entity);

    ASSERT_NE(position, nullptr);
    EXPECT_FLOAT_EQ(position->x, 10.0f);

    position->x = 20.0f;
    EXPECT_FLOAT_EQ(scene.get<Position>(entity)->x, 20.0f);

    EXPECT_TRUE(scene.remove<Position>(entity));
    EXPECT_FALSE(scene.has<Position>(entity));
    EXPECT_EQ(scene.get<Position>(entity), nullptr);
}

// Tests that pending entities cannot access components
TEST(SceneTests, RejectsPendingEntityComponents)
{
    omega::Scene scene;
    omega::Entity entity = scene.create_entity();

    EXPECT_FALSE(scene.add<Position>(entity, {10.0f}));
    EXPECT_FALSE(scene.has<Position>(entity));
    EXPECT_EQ(scene.get<Position>(entity), nullptr);
    EXPECT_FALSE(scene.remove<Position>(entity));

    scene.flush_commands();

    EXPECT_TRUE(scene.add<Position>(entity, {10.0f}));
}

// Tests that multiple component types can coexist on an entity
TEST(SceneTests, MultipleComponents)
{
    omega::Scene scene;
    omega::Entity entity = scene.create_entity();

    scene.flush_commands();

    EXPECT_TRUE(scene.add<Position>(entity, {10.0f}));
    EXPECT_TRUE(scene.add<Velocity>(entity, {5.0f}));

    EXPECT_TRUE(scene.has<Position>(entity));
    EXPECT_TRUE(scene.has<Velocity>(entity));

    EXPECT_FLOAT_EQ(scene.get<Position>(entity)->x, 10.0f);
    EXPECT_FLOAT_EQ(scene.get<Velocity>(entity)->x, 5.0f);
}

// Tests that destroying an entity removes all its components
TEST(SceneTests, DestroyRemovesAllComponents)
{
    omega::Scene scene;
    omega::Entity entity = scene.create_entity();

    scene.flush_commands();

    EXPECT_TRUE(scene.add<Position>(entity, {10.0f}));
    EXPECT_TRUE(scene.add<Velocity>(entity, {5.0f}));

    EXPECT_TRUE(scene.destroy_entity(entity));

    // Components remain accessible until destruction is flushed
    EXPECT_TRUE(scene.has<Position>(entity));
    EXPECT_TRUE(scene.has<Velocity>(entity));

    scene.flush_commands();

    EXPECT_FALSE(scene.has<Position>(entity));
    EXPECT_FALSE(scene.has<Velocity>(entity));

    EXPECT_EQ(scene.get<Position>(entity), nullptr);
    EXPECT_EQ(scene.get<Velocity>(entity), nullptr);
}

// Tests that stale entity handles cannot access components after index reuse
TEST(SceneTests, RejectsStaleEntityComponents)
{
    omega::Scene scene;
    omega::Entity old_entity = scene.create_entity();

    scene.flush_commands();

    EXPECT_TRUE(scene.add<Position>(old_entity, {10.0f}));
    EXPECT_TRUE(scene.destroy_entity(old_entity));

    scene.flush_commands();

    omega::Entity new_entity = scene.create_entity();

    EXPECT_EQ(new_entity.index, old_entity.index);
    EXPECT_NE(new_entity.generation, old_entity.generation);

    scene.flush_commands();

    EXPECT_FALSE(scene.has<Position>(old_entity));
    EXPECT_EQ(scene.get<Position>(old_entity), nullptr);

    EXPECT_TRUE(scene.add<Position>(new_entity, {20.0f}));
    EXPECT_FLOAT_EQ(scene.get<Position>(new_entity)->x, 20.0f);
}

// Tests that duplicate components are rejected
TEST(SceneTests, RejectsDuplicateComponents)
{
    omega::Scene scene;
    omega::Entity entity = scene.create_entity();

    scene.flush_commands();

    EXPECT_TRUE(scene.add<Position>(entity, {10.0f}));
    EXPECT_FALSE(scene.add<Position>(entity, {20.0f}));

    ASSERT_NE(scene.get<Position>(entity), nullptr);
    EXPECT_FLOAT_EQ(scene.get<Position>(entity)->x, 10.0f);
}

TEST(SceneTests, ComponentEvents)
{
    omega::EventSystem events;
    omega::Scene scene(events);

    omega::Entity entity = scene.create_entity();
    scene.flush_commands();

    std::vector<omega::ComponentEvent> received;

    events.subscribe<omega::ComponentEvent>([&](const omega::ComponentEvent& event) { received.push_back(event); });

    ASSERT_TRUE(scene.add<Position>(entity, Position{10.0f}));

    // Events are queued until dispatch
    EXPECT_TRUE(received.empty());

    events.dispatch();

    ASSERT_EQ(received.size(), 1);
    EXPECT_EQ(received[0].entity, entity);
    EXPECT_EQ(received[0].component_type, std::type_index(typeid(Position)));
    EXPECT_EQ(received[0].type, omega::ComponentEventType::Added);

    ASSERT_TRUE(scene.remove<Position>(entity));

    events.dispatch();

    ASSERT_EQ(received.size(), 2);
    EXPECT_EQ(received[1].entity, entity);
    EXPECT_EQ(received[1].component_type, std::type_index(typeid(Position)));
    EXPECT_EQ(received[1].type, omega::ComponentEventType::Removed);
}

TEST(SceneTests, DestroyEntityPublishesComponentRemovedEvents)
{
    omega::EventSystem events;
    omega::Scene scene(events);

    omega::Entity entity = scene.create_entity();
    scene.flush_commands();

    ASSERT_TRUE(scene.add<Position>(entity, Position{10.0f}));
    ASSERT_TRUE(scene.add<Velocity>(entity, Velocity{2.0f}));

    events.dispatch();

    std::vector<omega::ComponentEvent> received;

    events.subscribe<omega::ComponentEvent>([&](const omega::ComponentEvent& event) { received.push_back(event); });

    scene.destroy_entity(entity);
    scene.flush_commands();

    // Destruction queues one removal event per component
    EXPECT_TRUE(received.empty());

    events.dispatch();

    ASSERT_EQ(received.size(), 2);

    for (const auto& event : received)
    {
        EXPECT_EQ(event.entity, entity);
        EXPECT_EQ(event.type, omega::ComponentEventType::Removed);
    }

    EXPECT_TRUE(scene.is_alive(entity) == false);
}

TEST(SceneTests, FailedComponentOperationsDoNotPublishEvents)
{
    omega::EventSystem events;
    omega::Scene scene(events);

    omega::Entity entity = scene.create_entity();
    scene.flush_commands();

    std::vector<omega::ComponentEvent> received;

    events.subscribe<omega::ComponentEvent>([&](const omega::ComponentEvent& event) { received.push_back(event); });

    // Removing a component that doesn't exist must fail
    EXPECT_FALSE(scene.remove<Position>(entity));

    // Adding a component to a destroyed entity must fail
    scene.destroy_entity(entity);
    scene.flush_commands();

    EXPECT_FALSE(scene.add<Position>(entity, Position{10.0f}));

    events.dispatch();

    EXPECT_TRUE(received.empty());
}