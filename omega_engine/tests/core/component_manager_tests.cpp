#include <omega/core/component_manager.hpp>

#include <gtest/gtest.h>

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

// Tests that storage is created lazily
TEST(ComponentManagerTests, CreatesStorageOnDemand)
{
    omega::ComponentManager manager;

    EXPECT_FALSE(manager.has_storage<Position>());

    auto& storage = manager.get_storage<Position>();

    EXPECT_TRUE(manager.has_storage<Position>());
    EXPECT_EQ(storage.size(), 0);
}

// Tests that requesting existing storage returns the same instance
TEST(ComponentManagerTests, ReusesExistingStorage)
{
    omega::ComponentManager manager;

    auto& first = manager.get_storage<Position>();
    auto& second = manager.get_storage<Position>();

    EXPECT_EQ(&first, &second);
}

// Tests that different component types have independent storages
TEST(ComponentManagerTests, MaintainsSeparateStorages)
{
    omega::ComponentManager manager;

    auto& positions = manager.get_storage<Position>();
    auto& velocities = manager.get_storage<Velocity>();

    EXPECT_NE(static_cast<void*>(&positions), static_cast<void*>(&velocities));

    EXPECT_TRUE(manager.has_storage<Position>());
    EXPECT_TRUE(manager.has_storage<Velocity>());
}

// Tests that removing storage unregisters and destroys it
TEST(ComponentManagerTests, RemovesStorage)
{
    omega::ComponentManager manager;

    manager.get_storage<Position>();

    EXPECT_TRUE(manager.has_storage<Position>());
    EXPECT_TRUE(manager.remove_storage<Position>());
    EXPECT_FALSE(manager.has_storage<Position>());

    // Removing an already removed storage should fail
    EXPECT_FALSE(manager.remove_storage<Position>());
}

// Tests that clearing removes component data but preserves storage
TEST(ComponentManagerTests, ClearsAllStorages)
{
    omega::ComponentManager manager;

    omega::Entity entity{0, 0};

    auto& positions = manager.get_storage<Position>();
    auto& velocities = manager.get_storage<Velocity>();

    EXPECT_TRUE(positions.add(entity, {10.0f}));
    EXPECT_TRUE(velocities.add(entity, {20.0f}));

    manager.clear();

    EXPECT_TRUE(manager.has_storage<Position>());
    EXPECT_TRUE(manager.has_storage<Velocity>());

    EXPECT_EQ(positions.size(), 0);
    EXPECT_EQ(velocities.size(), 0);
}

// Tests that storage can be recreated after removal
TEST(ComponentManagerTests, RecreatesRemovedStorage)
{
    omega::ComponentManager manager;

    auto& original = manager.get_storage<Position>();

    EXPECT_TRUE(manager.remove_storage<Position>());
    EXPECT_FALSE(manager.has_storage<Position>());

    auto& recreated = manager.get_storage<Position>();

    EXPECT_TRUE(manager.has_storage<Position>());
    EXPECT_EQ(recreated.size(), 0);
}


// Tests adding and retrieving components through the manager
TEST(ComponentManagerTests, AddAndGetComponent)
{
    omega::ComponentManager manager;
    omega::Entity entity{0, 0};

    EXPECT_TRUE(manager.add<Position>(entity, {10.0f}));
    EXPECT_TRUE(manager.has<Position>(entity));

    Position* position = manager.get<Position>(entity);

    ASSERT_NE(position, nullptr);
    EXPECT_FLOAT_EQ(position->x, 10.0f);

    // Verify that the component can be modified through the returned pointer
    position->x = 25.0f;
    EXPECT_FLOAT_EQ(manager.get<Position>(entity)->x, 25.0f);
}

// Tests that duplicate components cannot be added to the same entity
TEST(ComponentManagerTests, RejectsDuplicateComponent)
{
    omega::ComponentManager manager;
    omega::Entity entity{0, 0};

    EXPECT_TRUE(manager.add<Position>(entity, {10.0f}));
    EXPECT_FALSE(manager.add<Position>(entity, {20.0f}));

    ASSERT_NE(manager.get<Position>(entity), nullptr);
    EXPECT_FLOAT_EQ(manager.get<Position>(entity)->x, 10.0f);
}

// Tests that different component types can belong to the same entity
TEST(ComponentManagerTests, SupportsMultipleComponentTypes)
{
    omega::ComponentManager manager;
    omega::Entity entity{0, 0};

    EXPECT_TRUE(manager.add<Position>(entity, {10.0f}));
    EXPECT_TRUE(manager.add<Velocity>(entity, {20.0f}));

    EXPECT_TRUE(manager.has<Position>(entity));
    EXPECT_TRUE(manager.has<Velocity>(entity));

    EXPECT_FLOAT_EQ(manager.get<Position>(entity)->x, 10.0f);
    EXPECT_FLOAT_EQ(manager.get<Velocity>(entity)->x, 20.0f);
}

// Tests removing a component without affecting other component types
TEST(ComponentManagerTests, RemovesComponent)
{
    omega::ComponentManager manager;
    omega::Entity entity{0, 0};

    EXPECT_TRUE(manager.add<Position>(entity, {10.0f}));
    EXPECT_TRUE(manager.add<Velocity>(entity, {20.0f}));

    EXPECT_TRUE(manager.remove<Position>(entity));

    EXPECT_FALSE(manager.has<Position>(entity));
    EXPECT_TRUE(manager.has<Velocity>(entity));

    EXPECT_EQ(manager.get<Position>(entity), nullptr);
    EXPECT_FLOAT_EQ(manager.get<Velocity>(entity)->x, 20.0f);

    // Removing the same component again should fail
    EXPECT_FALSE(manager.remove<Position>(entity));
}

// Tests that missing storages and components are handled safely
TEST(ComponentManagerTests, HandlesMissingComponents)
{
    omega::ComponentManager manager;
    omega::Entity entity{0, 0};

    EXPECT_FALSE(manager.has<Position>(entity));
    EXPECT_EQ(manager.get<Position>(entity), nullptr);
    EXPECT_FALSE(manager.remove<Position>(entity));

    // Read-only operations must not create storage.
    EXPECT_FALSE(manager.has_storage<Position>());
}

// Tests that stale entity generations cannot access components
TEST(ComponentManagerTests, RejectsStaleEntityHandles)
{
    omega::ComponentManager manager;

    omega::Entity entity{0, 1};
    omega::Entity stale_entity{0, 0};

    EXPECT_TRUE(manager.add<Position>(entity, {10.0f}));

    EXPECT_FALSE(manager.has<Position>(stale_entity));
    EXPECT_EQ(manager.get<Position>(stale_entity), nullptr);
    EXPECT_FALSE(manager.remove<Position>(stale_entity));

    EXPECT_TRUE(manager.has<Position>(entity));
}


} // namespace