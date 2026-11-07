#include <gtest/gtest.h>

#include <omega/core/scene/scene.hpp>
#include <omega/core/component/component_event.hpp>

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

    ASSERT_TRUE(scene.add<Position>(entity, Position{10.0f}));

    EXPECT_TRUE(events.get_queue<omega::ComponentAddedEvent<Position>>().get_events().empty());

    events.swap_all_buffers();

    const auto& added_queue = events.get_queue<omega::ComponentAddedEvent<Position>>().get_events();
    ASSERT_EQ(added_queue.size(), 1);
    EXPECT_EQ(added_queue[0].entity, entity);

    ASSERT_TRUE(scene.remove<Position>(entity));

    events.swap_all_buffers();

    const auto& removed_queue = events.get_queue<omega::ComponentRemovedEvent<Position>>().get_events();
    ASSERT_EQ(removed_queue.size(), 1);
    EXPECT_EQ(removed_queue[0].entity, entity);

    EXPECT_FALSE(removed_queue[0].swap_info.exchanged);
}

TEST(SceneTests, DestroyEntityPublishesComponentRemovedEvents)
{
    omega::EventSystem events;
    omega::Scene scene(events);

    omega::Entity entity = scene.create_entity();
    scene.flush_commands();

    ASSERT_TRUE(scene.add<Position>(entity, Position{10.0f}));
    ASSERT_TRUE(scene.add<Velocity>(entity, Velocity{2.0f}));

    EXPECT_TRUE(events.get_queue<omega::ComponentAddedEvent<Position>>().get_events().empty());
    EXPECT_TRUE(events.get_queue<omega::ComponentAddedEvent<Velocity>>().get_events().empty());

    events.swap_all_buffers();

    scene.destroy_entity(entity);
    scene.flush_commands();

    events.swap_all_buffers();

    const auto& position_queue = events.get_queue<omega::ComponentRemovedEvent<Position>>().get_events();
    ASSERT_EQ(position_queue.size(), 1);
    EXPECT_EQ(position_queue[0].entity, entity);

    const auto& velocity_queue = events.get_queue<omega::ComponentRemovedEvent<Velocity>>().get_events();
    ASSERT_EQ(velocity_queue.size(), 1);
    EXPECT_EQ(velocity_queue[0].entity, entity);

    EXPECT_FALSE(scene.is_alive(entity));
}

TEST(SceneTests, FailedComponentOperationsDoNotPublishEvents)
{
    omega::EventSystem events;
    omega::Scene scene(events);

    omega::Entity entity = scene.create_entity();
    scene.flush_commands();

    // Removing a component that does not exist must return false and push no event entries
    EXPECT_FALSE(scene.remove<Position>(entity));

    // Adding a component to an already dead entity boundary must fail as well
    scene.destroy_entity(entity);
    scene.flush_commands();

    EXPECT_FALSE(scene.add<Position>(entity, Position{10.0f}));

    events.swap_all_buffers();

    // Both action queues must remain completely empty and untouched
    EXPECT_TRUE(events.get_queue<omega::ComponentAddedEvent<Position>>().get_events().empty());
    EXPECT_TRUE(events.get_queue<omega::ComponentRemovedEvent<Position>>().get_events().empty());
}

// Checks that a parent and its children can be queried
TEST(SceneTests, QueriesParentAndChildren)
{
    omega::Scene scene;

    auto parent = scene.create_entity();
    auto child = scene.create_entity();

    scene.flush_commands();

    ASSERT_TRUE(scene.set_parent(child, parent));

    EXPECT_TRUE(scene.has_parent(child));
    EXPECT_EQ(scene.get_parent(child), parent);
    ASSERT_EQ(scene.get_children(parent).size(), 1);
    EXPECT_EQ(scene.get_children(parent)[0], child);
}

// Checks that destroying a parent reparents its children to its parent
TEST(SceneTests, DestroyingParentReparentsChildren)
{
    omega::Scene scene;

    auto grandparent = scene.create_entity();
    auto parent = scene.create_entity();
    auto child = scene.create_entity();

    scene.flush_commands();

    ASSERT_TRUE(scene.set_parent(parent, grandparent));
    ASSERT_TRUE(scene.set_parent(child, parent));

    scene.destroy_entity(parent);
    scene.flush_commands();

    EXPECT_TRUE(scene.is_alive(child));
    EXPECT_EQ(scene.get_parent(child), grandparent);

    ASSERT_EQ(scene.get_children(grandparent).size(), 1);
    EXPECT_EQ(scene.get_children(grandparent)[0], child);
}

// Checks that destroying a root parent makes its children roots
TEST(SceneTests, DestroyingRootParentMakesChildrenRoots)
{
    omega::Scene scene;

    auto parent = scene.create_entity();
    auto child = scene.create_entity();

    scene.flush_commands();

    ASSERT_TRUE(scene.set_parent(child, parent));

    scene.destroy_entity(parent);
    scene.flush_commands();

    EXPECT_TRUE(scene.is_alive(child));
    EXPECT_FALSE(scene.has_parent(child));
    EXPECT_TRUE(scene.get_children(parent).empty());
}

// Checks that reparenting removes the child from its previous parent
TEST(SceneTests, ReparentsChild)
{
    omega::Scene scene;

    auto first_parent = scene.create_entity();
    auto second_parent = scene.create_entity();
    auto child = scene.create_entity();

    scene.flush_commands();

    ASSERT_TRUE(scene.set_parent(child, first_parent));
    ASSERT_TRUE(scene.set_parent(child, second_parent));

    EXPECT_EQ(scene.get_parent(child), second_parent);
    EXPECT_TRUE(scene.get_children(first_parent).empty());

    ASSERT_EQ(scene.get_children(second_parent).size(), 1);
    EXPECT_EQ(scene.get_children(second_parent)[0], child);
}

// Checks that clearing a parent makes the child a root entity
TEST(SceneTests, ClearsParent)
{
    omega::Scene scene;

    auto parent = scene.create_entity();
    auto child = scene.create_entity();

    scene.flush_commands();

    ASSERT_TRUE(scene.set_parent(child, parent));
    ASSERT_TRUE(scene.clear_parent(child));

    EXPECT_FALSE(scene.has_parent(child));
    EXPECT_FALSE(scene.get_parent(child).is_valid());
    EXPECT_TRUE(scene.get_children(parent).empty());
}

// Checks that an entity cannot be its own parent
TEST(SceneTests, CannotSetEntityAsItsOwnParent)
{
    omega::Scene scene;

    auto entity = scene.create_entity();
    scene.flush_commands();

    EXPECT_FALSE(scene.set_parent(entity, entity));
    EXPECT_FALSE(scene.has_parent(entity));
    EXPECT_TRUE(scene.get_children(entity).empty());
}

// Checks that creating a circular hierarchy is rejected
TEST(SceneTests, CannotCreateCircularHierarchy)
{
    omega::Scene scene;

    auto first = scene.create_entity();
    auto second = scene.create_entity();
    auto third = scene.create_entity();

    scene.flush_commands();

    ASSERT_TRUE(scene.set_parent(second, first));
    ASSERT_TRUE(scene.set_parent(third, second));

    EXPECT_FALSE(scene.set_parent(first, third));

    EXPECT_FALSE(scene.has_parent(first));
    EXPECT_EQ(scene.get_parent(second), first);
    EXPECT_EQ(scene.get_parent(third), second);
}

// Checks that parenting fails when either entity is invalid or no longer alive
TEST(SceneTests, CannotSetParentWithInvalidEntities)
{
    omega::Scene scene;

    auto parent = scene.create_entity();
    auto child = scene.create_entity();

    scene.flush_commands();

    EXPECT_FALSE(scene.set_parent(omega::Entity::invalid(), parent));
    EXPECT_FALSE(scene.set_parent(child, omega::Entity::invalid()));

    scene.destroy_entity(parent);
    scene.flush_commands();

    EXPECT_FALSE(scene.set_parent(child, parent));
    EXPECT_FALSE(scene.has_parent(child));
}

// Checks that clearing the parent of a root entity is a successful no-op
TEST(SceneTests, ClearingParentOfRootEntitySucceeds)
{
    omega::Scene scene;

    auto entity = scene.create_entity();
    scene.flush_commands();

    EXPECT_TRUE(scene.clear_parent(entity));
    EXPECT_FALSE(scene.has_parent(entity));
    EXPECT_TRUE(scene.get_children(entity).empty());
}

// Checks that a parent can have multiple children
TEST(SceneTests, SupportsMultipleChildren)
{
    omega::Scene scene;

    auto parent = scene.create_entity();
    auto first_child = scene.create_entity();
    auto second_child = scene.create_entity();

    scene.flush_commands();

    ASSERT_TRUE(scene.set_parent(first_child, parent));
    ASSERT_TRUE(scene.set_parent(second_child, parent));

    const auto& children = scene.get_children(parent);

    ASSERT_EQ(children.size(), 2);
    EXPECT_NE(std::find(children.begin(), children.end(), first_child), children.end());
    EXPECT_NE(std::find(children.begin(), children.end(), second_child), children.end());
}

// Checks that destroying one child preserves its siblings
TEST(SceneTests, DestroyingChildPreservesSiblings)
{
    omega::Scene scene;

    auto parent = scene.create_entity();
    auto first_child = scene.create_entity();
    auto second_child = scene.create_entity();

    scene.flush_commands();

    ASSERT_TRUE(scene.set_parent(first_child, parent));
    ASSERT_TRUE(scene.set_parent(second_child, parent));

    scene.destroy_entity(first_child);
    scene.flush_commands();

    EXPECT_FALSE(scene.is_alive(first_child));
    EXPECT_TRUE(scene.is_alive(second_child));
    EXPECT_FALSE(scene.has_parent(first_child));

    const auto& children = scene.get_children(parent);

    ASSERT_EQ(children.size(), 1);
    EXPECT_EQ(children[0], second_child);
}

// Checks that a child can be assigned a new parent after clearing its previous one
TEST(SceneTests, CanSetParentAfterClearingPreviousParent)
{
    omega::Scene scene;

    auto first_parent = scene.create_entity();
    auto second_parent = scene.create_entity();
    auto child = scene.create_entity();

    scene.flush_commands();

    ASSERT_TRUE(scene.set_parent(child, first_parent));
    ASSERT_TRUE(scene.clear_parent(child));
    ASSERT_TRUE(scene.set_parent(child, second_parent));

    EXPECT_EQ(scene.get_parent(child), second_parent);
    EXPECT_TRUE(scene.get_children(first_parent).empty());

    ASSERT_EQ(scene.get_children(second_parent).size(), 1);
    EXPECT_EQ(scene.get_children(second_parent)[0], child);
}

// Checks that destroying a parent reparents all its children
TEST(SceneTests, DestroyingParentReparentsAllChildren)
{
    omega::Scene scene;

    auto grandparent = scene.create_entity();
    auto parent = scene.create_entity();
    auto first_child = scene.create_entity();
    auto second_child = scene.create_entity();

    scene.flush_commands();

    ASSERT_TRUE(scene.set_parent(parent, grandparent));
    ASSERT_TRUE(scene.set_parent(first_child, parent));
    ASSERT_TRUE(scene.set_parent(second_child, parent));

    scene.destroy_entity(parent);
    scene.flush_commands();

    EXPECT_EQ(scene.get_parent(first_child), grandparent);
    EXPECT_EQ(scene.get_parent(second_child), grandparent);

    const auto& children = scene.get_children(grandparent);

    ASSERT_EQ(children.size(), 2);
    EXPECT_NE(std::find(children.begin(), children.end(), first_child), children.end());
    EXPECT_NE(std::find(children.begin(), children.end(), second_child), children.end());
}

// Checks that marking an existing component dirty publishes a Modified event
TEST(SceneTests, MarkDirtyPublishesChangedEvent)
{
    omega::EventSystem events;
    omega::Scene scene(events);

    auto entity = scene.create_entity();
    scene.flush_commands();

    ASSERT_TRUE(scene.add(entity, Position{}));

    // Drain initial addition event from the main processing view
    events.swap_all_buffers();

    ASSERT_TRUE(scene.mark_dirty<Position>(entity));

    // Swap to expose mutation state change events inside the processed view
    events.swap_all_buffers();

    // Read directly from the discrete, dedicated modification event queue
    const auto& modified_queue = events.get_queue<omega::ComponentModifiedEvent<Position>>().get_events();
    ASSERT_EQ(modified_queue.size(), 1);
    EXPECT_EQ(modified_queue[0].entity, entity);
}

// Checks that marking a missing component dirty fails without publishing an event
TEST(SceneTests, MarkDirtyFailsForMissingComponent)
{
    omega::EventSystem events;
    omega::Scene scene(events);

    auto entity = scene.create_entity();
    scene.flush_commands();

    // Mark dirty fails when component storage metadata doesn't track this entity
    EXPECT_FALSE(scene.mark_dirty<Position>(entity));

    events.swap_all_buffers();

    // No structural or modification events must be logged in the isolated channels
    EXPECT_TRUE(events.get_queue<omega::ComponentModifiedEvent<Position>>().get_events().empty());
}

// Checks that marking a component on a destroyed entity fails
TEST(SceneTests, MarkDirtyFailsForDestroyedEntity)
{
    omega::EventSystem events;
    omega::Scene scene(events);

    auto entity = scene.create_entity();
    scene.flush_commands();

    ASSERT_TRUE(scene.add(entity, Position{}));

    scene.destroy_entity(entity);
    scene.flush_commands();

    EXPECT_FALSE(scene.mark_dirty<Position>(entity));
}

// Checks that marking a component dirty succeeds when no EventSystem is attached
TEST(SceneTests, MarkDirtyWorksWithoutEventSystem)
{
    omega::Scene scene;

    auto entity = scene.create_entity();
    scene.flush_commands();

    ASSERT_TRUE(scene.add(entity, Position{}));

    EXPECT_TRUE(scene.mark_dirty<Position>(entity));
}