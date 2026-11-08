#include <gtest/gtest.h>

#include <omega/core/component/component_event.hpp>
#include <omega/core/component/component_system.hpp>
#include <omega/core/entity/entity_event.hpp>
#include <omega/core/entity/entity_system.hpp>
#include <omega/core/event/event_system.hpp>

#include <algorithm>
#include <cstdint>
#include <random>
#include <unordered_set>
#include <vector>

using namespace omega;

// ============================================================
// TEST COMPONENTS
// ============================================================

struct Position
{
    float x = 0.0f;
    float y = 0.0f;

    bool operator==(const Position&) const = default;
};

struct Velocity
{
    float x = 0.0f;
    float y = 0.0f;

    bool operator==(const Velocity&) const = default;
};

struct Health
{
    int value = 100;

    bool operator==(const Health&) const = default;
};

// ============================================================
// FIXTURE
// ============================================================

class ComponentSystemTest : public ::testing::Test
{
  protected:
    EventSystem events;
    EntitySystem entities{events};
    ComponentSystem components{events, entities};

    void update()
    {
        events.swap_all_buffers();
        entities.update();
        components.update();
    }

    Entity create_alive_entity()
    {
        Entity entity = entities.create_entity();
        update();
        return entity;
    };

    template <typename T> void expect_component(Entity entity)
    {
        EXPECT_TRUE(components.has<T>(entity));
        EXPECT_NE(components.get<T>(entity), nullptr);
    }

    template <typename T> void expect_no_component(Entity entity)
    {
        EXPECT_FALSE(components.has<T>(entity));
        EXPECT_EQ(components.get<T>(entity), nullptr);
    }
};

// ============================================================
// COMPONENT ADDITION
// ============================================================

TEST_F(ComponentSystemTest, Add_AddsComponentImmediately)
{
    Entity entity = create_alive_entity();

    ASSERT_TRUE(components.add<Position>(entity, Position{10.0f, 20.0f}));

    expect_component<Position>(entity);

    const Position* position = components.get<Position>(entity);

    ASSERT_NE(position, nullptr);
    EXPECT_FLOAT_EQ(position->x, 10.0f);
    EXPECT_FLOAT_EQ(position->y, 20.0f);
}

TEST_F(ComponentSystemTest, Add_DoesNotRequireUpdate)
{
    Entity entity = create_alive_entity();

    ASSERT_TRUE(components.add<Position>(entity, Position{5.0f, 8.0f}));

    EXPECT_TRUE(components.has<Position>(entity));
}

TEST_F(ComponentSystemTest, Add_RejectsInvalidEntity)
{
    EXPECT_FALSE(components.add<Position>(Entity::invalid(), Position{}));

    EXPECT_FALSE(components.has_storage<Position>());
}

TEST_F(ComponentSystemTest, Add_RejectsDeadEntity)
{
    Entity entity = create_alive_entity();

    ASSERT_TRUE(entities.destroy_entity(entity));

    EXPECT_FALSE(components.add<Position>(entity, Position{}));
}

TEST_F(ComponentSystemTest, Add_RejectsDuplicateComponent)
{
    Entity entity = create_alive_entity();

    ASSERT_TRUE(components.add<Position>(entity, Position{1.0f, 2.0f}));

    EXPECT_FALSE(components.add<Position>(entity, Position{3.0f, 4.0f}));

    const Position* position = components.get<Position>(entity);

    ASSERT_NE(position, nullptr);
    EXPECT_FLOAT_EQ(position->x, 1.0f);
    EXPECT_FLOAT_EQ(position->y, 2.0f);
}

TEST_F(ComponentSystemTest, Add_AllowsDifferentComponentTypes)
{
    Entity entity = create_alive_entity();

    ASSERT_TRUE(components.add<Position>(entity, Position{1.0f, 2.0f}));
    ASSERT_TRUE(components.add<Velocity>(entity, Velocity{3.0f, 4.0f}));
    ASSERT_TRUE(components.add<Health>(entity, Health{75}));

    expect_component<Position>(entity);
    expect_component<Velocity>(entity);
    expect_component<Health>(entity);
}

TEST_F(ComponentSystemTest, Add_AllowsSameComponentTypeOnDifferentEntities)
{
    Entity first = create_alive_entity();
    Entity second = create_alive_entity();

    ASSERT_TRUE(components.add<Position>(first, Position{1.0f, 2.0f}));
    ASSERT_TRUE(components.add<Position>(second, Position{3.0f, 4.0f}));

    EXPECT_EQ(components.get_storage<Position>().size(), 2);
}

// ============================================================
// COMPONENT REMOVAL
// ============================================================

TEST_F(ComponentSystemTest, Remove_RemovesComponentImmediately)
{
    Entity entity = create_alive_entity();

    ASSERT_TRUE(components.add<Position>(entity, Position{}));

    ASSERT_TRUE(components.remove<Position>(entity));

    expect_no_component<Position>(entity);
}

TEST_F(ComponentSystemTest, Remove_DoesNotRequireUpdate)
{
    Entity entity = create_alive_entity();

    components.add<Position>(entity, Position{});

    ASSERT_TRUE(components.remove<Position>(entity));

    EXPECT_FALSE(components.has<Position>(entity));
}

TEST_F(ComponentSystemTest, Remove_ReturnsFalseWhenStorageDoesNotExist)
{
    Entity entity = create_alive_entity();

    EXPECT_FALSE(components.remove<Position>(entity));
}

TEST_F(ComponentSystemTest, Remove_ReturnsFalseWhenComponentDoesNotExist)
{
    Entity entity = create_alive_entity();

    components.get_storage<Position>();

    EXPECT_FALSE(components.remove<Position>(entity));
}

TEST_F(ComponentSystemTest, Remove_DoesNotAffectOtherComponentTypes)
{
    Entity entity = create_alive_entity();

    components.add<Position>(entity, Position{});
    components.add<Velocity>(entity, Velocity{});

    ASSERT_TRUE(components.remove<Position>(entity));

    expect_no_component<Position>(entity);
    expect_component<Velocity>(entity);
}

TEST_F(ComponentSystemTest, Remove_DoesNotAffectOtherEntities)
{
    Entity first = create_alive_entity();
    Entity second = create_alive_entity();

    components.add<Position>(first, Position{});
    components.add<Position>(second, Position{});

    ASSERT_TRUE(components.remove<Position>(first));

    expect_no_component<Position>(first);
    expect_component<Position>(second);
}

TEST_F(ComponentSystemTest, Remove_CanBeCalledOnEntityPendingDestruction)
{
    Entity entity = create_alive_entity();

    components.add<Position>(entity, Position{});

    ASSERT_TRUE(entities.destroy_entity(entity));

    EXPECT_TRUE(components.remove<Position>(entity));

    expect_no_component<Position>(entity);
}

// ============================================================
// COMPONENT ACCESS
// ============================================================

TEST_F(ComponentSystemTest, Accessors_GetReturnsComponent)
{
    Entity entity = create_alive_entity();

    components.add<Position>(entity, Position{12.0f, 24.0f});

    Position* position = components.get<Position>(entity);

    ASSERT_NE(position, nullptr);

    EXPECT_FLOAT_EQ(position->x, 12.0f);
    EXPECT_FLOAT_EQ(position->y, 24.0f);
}

TEST_F(ComponentSystemTest, Accessors_GetReturnsNullForMissingComponent)
{
    Entity entity = create_alive_entity();

    EXPECT_EQ(components.get<Position>(entity), nullptr);
}

TEST_F(ComponentSystemTest, Accessors_ConstGetReturnsComponent)
{
    Entity entity = create_alive_entity();

    components.add<Position>(entity, Position{12.0f, 24.0f});

    const ComponentSystem& const_components = components;

    const Position* position = const_components.get<Position>(entity);

    ASSERT_NE(position, nullptr);

    EXPECT_FLOAT_EQ(position->x, 12.0f);
    EXPECT_FLOAT_EQ(position->y, 24.0f);
}

TEST_F(ComponentSystemTest, Accessors_ConstGetReturnsNullForMissingComponent)
{
    Entity entity = create_alive_entity();

    const ComponentSystem& const_components = components;

    EXPECT_EQ(const_components.get<Position>(entity), nullptr);
}

TEST_F(ComponentSystemTest, Accessors_HasStorage)
{
    EXPECT_FALSE(components.has_storage<Position>());

    components.get_storage<Position>();

    EXPECT_TRUE(components.has_storage<Position>());
}

TEST_F(ComponentSystemTest, Accessors_HasChecksComponentPresence)
{
    Entity entity = create_alive_entity();

    EXPECT_FALSE(components.has<Position>(entity));

    components.add<Position>(entity, Position{});

    EXPECT_TRUE(components.has<Position>(entity));
}

TEST_F(ComponentSystemTest, Accessors_GetStorageReturnsSameStorage)
{
    ComponentStorage<Position>& first = components.get_storage<Position>();

    ComponentStorage<Position>& second = components.get_storage<Position>();

    EXPECT_EQ(&first, &second);
}

// ============================================================
// STORAGE MANAGEMENT
// ============================================================

TEST_F(ComponentSystemTest, Storage_RemoveStorage)
{
    Entity entity = create_alive_entity();

    components.add<Position>(entity, Position{});

    ASSERT_TRUE(components.remove_storage<Position>());

    EXPECT_FALSE(components.has_storage<Position>());
    EXPECT_FALSE(components.remove_storage<Position>());
}

TEST_F(ComponentSystemTest, Storage_RemoveStorageDoesNotAffectOtherStorages)
{
    Entity entity = create_alive_entity();

    components.add<Position>(entity, Position{});
    components.add<Velocity>(entity, Velocity{});

    ASSERT_TRUE(components.remove_storage<Position>());

    EXPECT_FALSE(components.has<Position>(entity));
    EXPECT_TRUE(components.has<Velocity>(entity));
}

TEST_F(ComponentSystemTest, Storage_SizeTracksComponents)
{
    Entity first = create_alive_entity();
    Entity second = create_alive_entity();
    Entity third = create_alive_entity();

    auto& storage = components.get_storage<Position>();

    EXPECT_EQ(storage.size(), 0);

    components.add<Position>(first, Position{});
    EXPECT_EQ(storage.size(), 1);

    components.add<Position>(second, Position{});
    EXPECT_EQ(storage.size(), 2);

    components.add<Position>(third, Position{});
    EXPECT_EQ(storage.size(), 3);

    components.remove<Position>(second);
    EXPECT_EQ(storage.size(), 2);
}

TEST_F(ComponentSystemTest, Storage_EntityAtReturnsStoredEntities)
{
    Entity first = create_alive_entity();
    Entity second = create_alive_entity();
    Entity third = create_alive_entity();

    components.add<Position>(first, Position{});
    components.add<Position>(second, Position{});
    components.add<Position>(third, Position{});

    const auto& storage = components.get_storage<Position>();

    std::unordered_set<uint64_t> stored_entities;

    for (uint32_t i = 0; i < storage.size(); ++i)
    {
        Entity entity = storage.entity_at(i);

        uint64_t key = (static_cast<uint64_t>(entity.generation) << 32) | entity.index;

        stored_entities.insert(key);
    }

    EXPECT_EQ(stored_entities.size(), 3);

    const auto entity_key = [](Entity entity)
    { return (static_cast<uint64_t>(entity.generation) << 32) | entity.index; };

    EXPECT_TRUE(stored_entities.contains(entity_key(first)));
    EXPECT_TRUE(stored_entities.contains(entity_key(second)));
    EXPECT_TRUE(stored_entities.contains(entity_key(third)));
}

// ============================================================
// MODIFICATION
// ============================================================

TEST_F(ComponentSystemTest, MarkDirty_PublishesModifiedEvent)
{
    Entity entity = create_alive_entity();

    components.add<Position>(entity, Position{});

    ASSERT_TRUE(components.mark_dirty<Position>(entity));

    events.swap_all_buffers();

    const auto& queue = events.get_queue<ComponentModifiedEvent<Position>>().get_events();

    ASSERT_EQ(queue.size(), 1);
    EXPECT_EQ(queue.front().entity, entity);
}

TEST_F(ComponentSystemTest, MarkDirty_ReturnsFalseForMissingComponent)
{
    Entity entity = create_alive_entity();

    EXPECT_FALSE(components.mark_dirty<Position>(entity));
}

TEST_F(ComponentSystemTest, MarkDirty_ReturnsFalseForDeadEntity)
{
    Entity entity = create_alive_entity();

    components.add<Position>(entity, Position{});

    ASSERT_TRUE(entities.destroy_entity(entity));

    EXPECT_FALSE(components.mark_dirty<Position>(entity));
}

TEST_F(ComponentSystemTest, MarkDirty_DoesNotModifyComponent)
{
    Entity entity = create_alive_entity();

    components.add<Position>(entity, Position{10.0f, 20.0f});

    ASSERT_TRUE(components.mark_dirty<Position>(entity));

    const Position* position = components.get<Position>(entity);

    ASSERT_NE(position, nullptr);
    EXPECT_FLOAT_EQ(position->x, 10.0f);
    EXPECT_FLOAT_EQ(position->y, 20.0f);
}

// ============================================================
// EVENT PUBLICATION
// ============================================================

TEST_F(ComponentSystemTest, Events_AddPublishesAddedEvent)
{
    Entity entity = create_alive_entity();

    ASSERT_TRUE(components.add<Position>(entity, Position{}));

    events.swap_all_buffers();

    const auto& queue = events.get_queue<ComponentAddedEvent<Position>>().get_events();

    ASSERT_EQ(queue.size(), 1);
    EXPECT_EQ(queue.front().entity, entity);
}

TEST_F(ComponentSystemTest, Events_RemovePublishesRemovedEvent)
{
    Entity entity = create_alive_entity();

    components.add<Position>(entity, Position{});

    // Move the added notification out of the write buffer.
    events.swap_all_buffers();

    ASSERT_TRUE(components.remove<Position>(entity));

    events.swap_all_buffers();

    const auto& queue = events.get_queue<ComponentRemovedEvent<Position>>().get_events();

    ASSERT_EQ(queue.size(), 1);
    EXPECT_EQ(queue.front().entity, entity);
}

TEST_F(ComponentSystemTest, Events_FailedAddDoesNotPublishEvent)
{
    Entity entity = create_alive_entity();

    ASSERT_TRUE(components.add<Position>(entity, Position{}));

    events.swap_all_buffers();

    EXPECT_FALSE(components.add<Position>(entity, Position{}));

    events.swap_all_buffers();

    const auto& queue = events.get_queue<ComponentAddedEvent<Position>>().get_events();

    EXPECT_TRUE(queue.empty());
}

TEST_F(ComponentSystemTest, Events_FailedRemoveDoesNotPublishEvent)
{
    Entity entity = create_alive_entity();

    components.get_storage<Position>();

    EXPECT_FALSE(components.remove<Position>(entity));

    events.swap_all_buffers();

    const auto& queue = events.get_queue<ComponentRemovedEvent<Position>>().get_events();

    EXPECT_TRUE(queue.empty());
}

TEST_F(ComponentSystemTest, Events_MultipleOperationsPublishInOrder)
{
    Entity entity = create_alive_entity();

    components.add<Position>(entity, Position{});
    components.remove<Position>(entity);
    components.add<Position>(entity, Position{});

    events.swap_all_buffers();

    const auto& added_queue = events.get_queue<ComponentAddedEvent<Position>>().get_events();

    const auto& removed_queue = events.get_queue<ComponentRemovedEvent<Position>>().get_events();

    EXPECT_EQ(added_queue.size(), 2);
    EXPECT_EQ(removed_queue.size(), 1);
}

// ============================================================
// ENTITY DESTRUCTION
// ============================================================

TEST_F(ComponentSystemTest, Destruction_RemovesAllComponents)
{
    Entity entity = create_alive_entity();

    components.add<Position>(entity, Position{});
    components.add<Velocity>(entity, Velocity{});
    components.add<Health>(entity, Health{});

    ASSERT_TRUE(entities.destroy_entity(entity));

    update();

    expect_no_component<Position>(entity);
    expect_no_component<Velocity>(entity);
    expect_no_component<Health>(entity);
}

TEST_F(ComponentSystemTest, Destruction_RemovalIsDeferredUntilUpdate)
{
    Entity entity = create_alive_entity();

    components.add<Position>(entity, Position{});

    ASSERT_TRUE(entities.destroy_entity(entity));

    // Storage is not modified until ComponentSystem::update().
    EXPECT_TRUE(components.has<Position>(entity));

    update();

    EXPECT_FALSE(components.has<Position>(entity));
}

TEST_F(ComponentSystemTest, Destruction_DoesNotRemoveOtherEntitiesComponents)
{
    Entity destroyed = create_alive_entity();
    Entity survivor = create_alive_entity();

    components.add<Position>(destroyed, Position{1.0f, 2.0f});
    components.add<Position>(survivor, Position{3.0f, 4.0f});

    ASSERT_TRUE(entities.destroy_entity(destroyed));

    update();

    expect_no_component<Position>(destroyed);
    expect_component<Position>(survivor);

    const Position* position = components.get<Position>(survivor);

    ASSERT_NE(position, nullptr);
    EXPECT_FLOAT_EQ(position->x, 3.0f);
    EXPECT_FLOAT_EQ(position->y, 4.0f);
}

TEST_F(ComponentSystemTest, Destruction_EntityWithoutComponents)
{
    Entity entity = create_alive_entity();

    ASSERT_TRUE(entities.destroy_entity(entity));

    EXPECT_NO_THROW(update());
}

TEST_F(ComponentSystemTest, Destruction_MultipleEntities)
{
    Entity first = create_alive_entity();
    Entity second = create_alive_entity();
    Entity third = create_alive_entity();

    components.add<Position>(first, Position{});
    components.add<Position>(second, Position{});
    components.add<Position>(third, Position{});

    ASSERT_TRUE(entities.destroy_entity(first));
    ASSERT_TRUE(entities.destroy_entity(second));

    update();

    expect_no_component<Position>(first);
    expect_no_component<Position>(second);
    expect_component<Position>(third);

    EXPECT_EQ(components.get_storage<Position>().size(), 1);
}

TEST_F(ComponentSystemTest, Destruction_MultipleComponentTypes)
{
    Entity entity = create_alive_entity();

    components.add<Position>(entity, Position{});
    components.add<Position>(entity, Position{});
    components.add<Velocity>(entity, Velocity{});
    components.add<Health>(entity, Health{});

    ASSERT_TRUE(entities.destroy_entity(entity));

    update();

    EXPECT_EQ(components.get_storage<Position>().size(), 0);
    EXPECT_EQ(components.get_storage<Velocity>().size(), 0);
    EXPECT_EQ(components.get_storage<Health>().size(), 0);
}

TEST_F(ComponentSystemTest, Destruction_PublishesRemovedEvents)
{
    Entity entity = create_alive_entity();

    components.add<Position>(entity, Position{});
    components.add<Velocity>(entity, Velocity{});

    ASSERT_TRUE(entities.destroy_entity(entity));

    update();

    // Component removal notifications were published into the write buffers.
    events.swap_all_buffers();

    const auto& position_queue = events.get_queue<ComponentRemovedEvent<Position>>().get_events();

    const auto& velocity_queue = events.get_queue<ComponentRemovedEvent<Velocity>>().get_events();

    ASSERT_EQ(position_queue.size(), 1);
    ASSERT_EQ(velocity_queue.size(), 1);

    EXPECT_EQ(position_queue.front().entity, entity);
    EXPECT_EQ(velocity_queue.front().entity, entity);
}

TEST_F(ComponentSystemTest, Destruction_DoesNotRemoveReplacementEntityComponents)
{
    Entity original = create_alive_entity();

    components.add<Position>(original, Position{1.0f, 2.0f});

    ASSERT_TRUE(entities.destroy_entity(original));

    update();

    Entity replacement = create_alive_entity();

    EXPECT_EQ(replacement.index, original.index);
    EXPECT_NE(replacement.generation, original.generation);

    ASSERT_TRUE(components.add<Position>(replacement, Position{3.0f, 4.0f}));

    expect_no_component<Position>(original);
    expect_component<Position>(replacement);
}

// ============================================================
// EVENT PROCESSING
// ============================================================

TEST_F(ComponentSystemTest, EventProcessing_NoEventsDoesNotChangeStorage)
{
    Entity entity = create_alive_entity();

    components.add<Position>(entity, Position{});

    for (int i = 0; i < 10; ++i)
        update();

    expect_component<Position>(entity);
}

TEST_F(ComponentSystemTest, EventProcessing_EntityRemovalRequiresBufferSwap)
{
    Entity entity = create_alive_entity();

    components.add<Position>(entity, Position{});

    ASSERT_TRUE(entities.destroy_entity(entity));

    // Without swapping, the removal event is not visible to ComponentSystem.
    components.update();

    EXPECT_TRUE(components.has<Position>(entity));

    update();

    EXPECT_FALSE(components.has<Position>(entity));
}

// ============================================================
// STRESS TESTS
// ============================================================

TEST_F(ComponentSystemTest, Stress_RandomizedComponentOperations)
{
    constexpr uint32_t initial_count = 1000;
    constexpr uint32_t operations = 20000;
    constexpr uint32_t validation_interval = 25;

    std::mt19937 rng(0xC0FFEE);

    std::vector<Entity> active;
    active.reserve(initial_count + operations / 2);

    for (uint32_t i = 0; i < initial_count; ++i)
        active.push_back(entities.create_entity());

    update();

    ASSERT_EQ(active.size(), initial_count);

    const auto entity_key = [](Entity entity) -> uint64_t
    { return (static_cast<uint64_t>(entity.generation) << 32) | entity.index; };

    const auto validate_storage = [&]()
    {
        std::unordered_set<uint64_t> alive_entities;

        for (Entity entity : active)
        {
            ASSERT_TRUE(entities.is_alive(entity));

            ASSERT_TRUE(alive_entities.insert(entity_key(entity)).second);
        }

        const auto validate_component_storage = [&](const auto& storage)
        {
            std::unordered_set<uint64_t> stored_entities;

            for (uint32_t index = 0; index < storage.size(); ++index)
            {
                Entity entity = storage.entity_at(index);

                EXPECT_TRUE(entities.is_alive(entity));
                EXPECT_TRUE(alive_entities.contains(entity_key(entity)));

                EXPECT_TRUE(stored_entities.insert(entity_key(entity)).second);

                EXPECT_NE(storage.get(entity), nullptr);
            }

            EXPECT_EQ(stored_entities.size(), storage.size());
        };

        if (components.has_storage<Position>())
            validate_component_storage(components.get_storage<Position>());

        if (components.has_storage<Velocity>())
            validate_component_storage(components.get_storage<Velocity>());

        if (components.has_storage<Health>())
            validate_component_storage(components.get_storage<Health>());

        // Verify every active entity against each storage.
        for (Entity entity : active)
        {
            if (components.has<Position>(entity))
                EXPECT_NE(components.get<Position>(entity), nullptr);

            if (components.has<Velocity>(entity))
                EXPECT_NE(components.get<Velocity>(entity), nullptr);

            if (components.has<Health>(entity))
                EXPECT_NE(components.get<Health>(entity), nullptr);
        }
    };

    for (uint32_t i = 0; i < operations; ++i)
    {
        const uint32_t operation = active.empty() ? 0 : rng() % 100;

        if (operation < 15)
        {
            // Create entity.
            active.push_back(entities.create_entity());
        }
        else if (operation < 35)
        {
            // Add Position.
            Entity entity = active[rng() % active.size()];

            components.add<Position>(entity,
                                     Position{static_cast<float>(rng() % 1000), static_cast<float>(rng() % 1000)});
        }
        else if (operation < 50)
        {
            // Add Velocity.
            Entity entity = active[rng() % active.size()];

            components.add<Velocity>(entity,
                                     Velocity{static_cast<float>(rng() % 1000), static_cast<float>(rng() % 1000)});
        }
        else if (operation < 60)
        {
            // Add Health.
            Entity entity = active[rng() % active.size()];

            components.add<Health>(entity, Health{static_cast<int>(rng() % 100)});
        }
        else if (operation < 75)
        {
            // Remove a random component type.
            Entity entity = active[rng() % active.size()];

            switch (rng() % 3)
            {
            case 0:
                components.remove<Position>(entity);
                break;

            case 1:
                components.remove<Velocity>(entity);
                break;

            case 2:
                components.remove<Health>(entity);
                break;
            }
        }
        else if (operation < 85)
        {
            // Mark a random component dirty.
            Entity entity = active[rng() % active.size()];

            switch (rng() % 3)
            {
            case 0:
                components.mark_dirty<Position>(entity);
                break;

            case 1:
                components.mark_dirty<Velocity>(entity);
                break;

            case 2:
                components.mark_dirty<Health>(entity);
                break;
            }
        }
        else
        {
            // Destroy entity.
            const size_t index = rng() % active.size();

            Entity entity = active[index];

            ASSERT_TRUE(entities.destroy_entity(entity));

            active.erase(active.begin() + index);
        }

        update();

        if ((i + 1) % validation_interval == 0)
            validate_storage();
    }

    validate_storage();
}
