#include <gtest/gtest.h>

#include <omega/core/entity/entity_event.hpp>
#include <omega/core/entity/entity_system.hpp>
#include <omega/core/event/event_system.hpp>

#include <algorithm>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <unordered_set>
#include <vector>

using namespace omega;

class EntitySystemTest : public ::testing::Test
{
  protected:
    EventSystem events;
    EntitySystem entities{events};

    void update()
    {
        events.swap_all_buffers();
        entities.update();
    }

    Entity create_alive_entity()
    {
        Entity entity = entities.create_entity();
        update();
        return entity;
    }

    std::vector<Entity> collect_entities() const
    {
        std::vector<Entity> result;

        for (auto it = entities.begin(); it != entities.end(); ++it)
            result.push_back(*it);

        return result;
    }
};

// ============================================================
// CREATION
// ============================================================

TEST_F(EntitySystemTest, CreateEntity_ReturnsValidHandle)
{
    Entity entity = entities.create_entity();

    EXPECT_EQ(entity.index, 0);
    EXPECT_EQ(entity.generation, 0);
}

TEST_F(EntitySystemTest, CreateEntity_FirstEntityUsesIndexZero)
{
    Entity entity = entities.create_entity();

    EXPECT_EQ(entity.index, 0);
}

TEST_F(EntitySystemTest, CreateEntity_AllocatesUniqueIndices)
{
    std::unordered_set<uint32_t> indices;

    for (int i = 0; i < 100; ++i)
    {
        Entity entity = entities.create_entity();
        EXPECT_TRUE(indices.insert(entity.index).second);
    }
}

TEST_F(EntitySystemTest, CreateEntity_IsNotImmediatelyAlive)
{
    Entity entity = entities.create_entity();

    EXPECT_FALSE(entities.is_alive(entity));
}

TEST_F(EntitySystemTest, CreateEntity_IsDestructible)
{
    Entity entity = entities.create_entity();

    EXPECT_TRUE(entities.is_destructible(entity));
}

TEST_F(EntitySystemTest, CreateEntity_DoesNotImmediatelyIncreaseCount)
{
    entities.create_entity();

    EXPECT_EQ(entities.entity_count(), 0);
}

TEST_F(EntitySystemTest, CreateMultipleEntities_ProducesUniqueHandles)
{
    Entity a = entities.create_entity();
    Entity b = entities.create_entity();
    Entity c = entities.create_entity();

    EXPECT_NE(a.index, b.index);
    EXPECT_NE(a.index, c.index);
    EXPECT_NE(b.index, c.index);
}

TEST_F(EntitySystemTest, CreateMultipleEntities_AllBecomeAliveAfterUpdate)
{
    Entity a = entities.create_entity();
    Entity b = entities.create_entity();
    Entity c = entities.create_entity();

    update();

    EXPECT_TRUE(entities.is_alive(a));
    EXPECT_TRUE(entities.is_alive(b));
    EXPECT_TRUE(entities.is_alive(c));
    EXPECT_EQ(entities.entity_count(), 3);
}

// ============================================================
// DESTRUCTION
// ============================================================

TEST_F(EntitySystemTest, DestroyEntity_AliveEntityCanBeDestroyed)
{
    Entity entity = create_alive_entity();

    EXPECT_TRUE(entities.destroy_entity(entity));
}

TEST_F(EntitySystemTest, DestroyEntity_DoesNotImmediatelyDecreaseCount)
{
    Entity entity = create_alive_entity();

    entities.destroy_entity(entity);

    EXPECT_EQ(entities.entity_count(), 1);
}

TEST_F(EntitySystemTest, DestroyEntity_ImmediatelyMakesEntityNotAlive)
{
    Entity entity = create_alive_entity();

    entities.destroy_entity(entity);

    EXPECT_FALSE(entities.is_alive(entity));
}

TEST_F(EntitySystemTest, DestroyEntity_ImmediatelyMakesEntityNotDestructible)
{
    Entity entity = create_alive_entity();

    entities.destroy_entity(entity);

    EXPECT_FALSE(entities.is_destructible(entity));
}

TEST_F(EntitySystemTest, DestroyEntity_CannotBeDestroyedTwice)
{
    Entity entity = create_alive_entity();

    EXPECT_TRUE(entities.destroy_entity(entity));
    EXPECT_FALSE(entities.destroy_entity(entity));
}

TEST_F(EntitySystemTest, DestroyEntity_PublishesOnlyOneRemovalEvent)
{
    Entity entity = create_alive_entity();

    entities.destroy_entity(entity);
    entities.destroy_entity(entity);
    entities.destroy_entity(entity);

    events.swap_all_buffers();
    const auto& queue = events.get_queue<EntityRemovedEvent>().get_events();

    EXPECT_EQ(queue.size(), 1);
}

TEST_F(EntitySystemTest, DestroyEntity_PendingCreationCanBeDestroyed)
{
    Entity entity = entities.create_entity();

    EXPECT_TRUE(entities.destroy_entity(entity));
    EXPECT_FALSE(entities.destroy_entity(entity));
}

TEST_F(EntitySystemTest, DestroyEntity_InvalidIndexReturnsFalse)
{
    Entity entity{.index = 100, .generation = 0};

    EXPECT_FALSE(entities.destroy_entity(entity));
}

TEST_F(EntitySystemTest, DestroyEntity_StaleGenerationReturnsFalse)
{
    Entity entity = create_alive_entity();

    Entity stale{.index = entity.index, .generation = entity.generation + 1};

    EXPECT_FALSE(entities.destroy_entity(stale));
}

TEST_F(EntitySystemTest, DestroyEntity_DoesNotAffectOtherEntities)
{
    Entity a = create_alive_entity();
    Entity b = entities.create_entity();
    update();

    entities.destroy_entity(a);
    update();

    EXPECT_FALSE(entities.is_alive(a));
    EXPECT_TRUE(entities.is_alive(b));
    EXPECT_EQ(entities.entity_count(), 1);
}

// ============================================================
// LIFECYCLE
// ============================================================

TEST_F(EntitySystemTest, Update_CreatedEntityBecomesAlive)
{
    Entity entity = entities.create_entity();

    update();

    EXPECT_TRUE(entities.is_alive(entity));
}

TEST_F(EntitySystemTest, Update_CreatedEntityIncreasesCount)
{
    entities.create_entity();

    update();

    EXPECT_EQ(entities.entity_count(), 1);
}

TEST_F(EntitySystemTest, Update_DestroyedEntityBecomesDead)
{
    Entity entity = create_alive_entity();

    entities.destroy_entity(entity);
    update();

    EXPECT_FALSE(entities.is_alive(entity));
}

TEST_F(EntitySystemTest, Update_DestroyedEntityDecreasesCount)
{
    Entity entity = create_alive_entity();

    entities.destroy_entity(entity);
    update();

    EXPECT_EQ(entities.entity_count(), 0);
}

TEST_F(EntitySystemTest, Update_MultipleUpdatesWithoutEventsDoNotChangeCount)
{
    Entity entity = create_alive_entity();

    for (int i = 0; i < 10; ++i)
        update();

    EXPECT_TRUE(entities.is_alive(entity));
    EXPECT_EQ(entities.entity_count(), 1);
}

TEST_F(EntitySystemTest, Lifecycle_CreateAndDestroyBeforeUpdate)
{
    Entity entity = entities.create_entity();

    EXPECT_TRUE(entities.destroy_entity(entity));

    update();

    EXPECT_FALSE(entities.is_alive(entity));
    EXPECT_EQ(entities.entity_count(), 0);
}

TEST_F(EntitySystemTest, Lifecycle_DestroyAndCreateReplacement)
{
    Entity first = create_alive_entity();

    entities.destroy_entity(first);
    update();

    Entity second = entities.create_entity();
    update();

    EXPECT_TRUE(entities.is_alive(second));
    EXPECT_FALSE(entities.is_alive(first));
    EXPECT_EQ(entities.entity_count(), 1);
}

TEST_F(EntitySystemTest, Lifecycle_MultipleBufferSwaps)
{
    Entity first = entities.create_entity();

    update();

    ASSERT_TRUE(entities.is_alive(first));

    Entity second = entities.create_entity();

    update();

    ASSERT_TRUE(entities.is_alive(second));
    EXPECT_EQ(entities.entity_count(), 2);

    entities.destroy_entity(first);

    update();

    EXPECT_FALSE(entities.is_alive(first));
    EXPECT_TRUE(entities.is_alive(second));
    EXPECT_EQ(entities.entity_count(), 1);
}

// ============================================================
// GENERATION AND SLOT REUSE
// ============================================================

TEST_F(EntitySystemTest, SlotReuse_ReusesFreedIndex)
{
    Entity first = create_alive_entity();

    entities.destroy_entity(first);
    update();

    Entity second = entities.create_entity();

    EXPECT_EQ(first.index, second.index);
}

TEST_F(EntitySystemTest, SlotReuse_IncrementsGeneration)
{
    Entity first = create_alive_entity();

    entities.destroy_entity(first);
    update();

    Entity second = entities.create_entity();

    EXPECT_EQ(second.generation, first.generation + 1);
}

TEST_F(EntitySystemTest, SlotReuse_StaleHandleIsNotAlive)
{
    Entity first = create_alive_entity();

    entities.destroy_entity(first);
    update();

    Entity second = entities.create_entity();
    update();

    EXPECT_FALSE(entities.is_alive(first));
    EXPECT_TRUE(entities.is_alive(second));
}

TEST_F(EntitySystemTest, SlotReuse_StaleHandleCannotDestroyReplacement)
{
    Entity first = create_alive_entity();

    entities.destroy_entity(first);
    update();

    Entity second = create_alive_entity();

    EXPECT_FALSE(entities.destroy_entity(first));
    EXPECT_TRUE(entities.is_alive(second));
}

TEST_F(EntitySystemTest, SlotReuse_MultipleSlotsAreReused)
{
    std::vector<Entity> original;

    for (int i = 0; i < 10; ++i)
        original.push_back(create_alive_entity());

    for (Entity entity : original)
        EXPECT_TRUE(entities.destroy_entity(entity));

    update();

    std::vector<Entity> replacements;

    for (int i = 0; i < 10; ++i)
        replacements.push_back(entities.create_entity());

    std::unordered_set<uint32_t> original_indices;

    for (Entity entity : original)
        original_indices.insert(entity.index);

    for (Entity entity : replacements)
        EXPECT_TRUE(original_indices.contains(entity.index));
}

TEST_F(EntitySystemTest, SlotReuse_GenerationIncrementsOnlyOnReuse)
{
    Entity first = create_alive_entity();

    Entity second = create_alive_entity();

    EXPECT_EQ(first.generation, 0);
    EXPECT_EQ(second.generation, 0);
}

// ============================================================
// ENTITY COUNT
// ============================================================

TEST_F(EntitySystemTest, EntityCount_InitiallyZero)
{
    EXPECT_EQ(entities.entity_count(), 0);
}

TEST_F(EntitySystemTest, EntityCount_MultipleCreations)
{
    for (int i = 0; i < 50; ++i)
        entities.create_entity();

    EXPECT_EQ(entities.entity_count(), 0);

    update();

    EXPECT_EQ(entities.entity_count(), 50);
}

TEST_F(EntitySystemTest, EntityCount_MultipleDestructions)
{
    std::vector<Entity> created;

    for (int i = 0; i < 50; ++i)
        created.push_back(create_alive_entity());

    for (Entity entity : created)
        entities.destroy_entity(entity);

    EXPECT_EQ(entities.entity_count(), 50);

    update();

    EXPECT_EQ(entities.entity_count(), 0);
}

TEST_F(EntitySystemTest, EntityCount_PartialDestruction)
{
    std::vector<Entity> created;

    for (int i = 0; i < 10; ++i)
        created.push_back(create_alive_entity());

    for (int i = 0; i < 5; ++i)
        entities.destroy_entity(created[i]);

    update();

    EXPECT_EQ(entities.entity_count(), 5);
}

TEST_F(EntitySystemTest, EntityCount_CreateAfterDestruction)
{
    Entity first = create_alive_entity();

    entities.destroy_entity(first);
    update();

    Entity second = create_alive_entity();

    EXPECT_EQ(entities.entity_count(), 1);
    EXPECT_TRUE(entities.is_alive(second));
}

// ============================================================
// ITERATION
// ============================================================

TEST_F(EntitySystemTest, Iteration_EmptySystemReturnsNoEntities)
{
    EXPECT_TRUE(collect_entities().empty());
}

TEST_F(EntitySystemTest, Iteration_ReturnsAllAliveEntities)
{
    for (int i = 0; i < 10; ++i)
        entities.create_entity();

    update();

    auto result = collect_entities();

    EXPECT_EQ(result.size(), 10);
}

TEST_F(EntitySystemTest, Iteration_DoesNotReturnPendingCreation)
{
    entities.create_entity();

    EXPECT_TRUE(collect_entities().empty());
}

TEST_F(EntitySystemTest, Iteration_DoesNotReturnPendingDestruction)
{
    Entity entity = create_alive_entity();

    entities.destroy_entity(entity);

    EXPECT_TRUE(collect_entities().empty());
}

TEST_F(EntitySystemTest, Iteration_DoesNotReturnDestroyedEntities)
{
    Entity a = create_alive_entity();
    Entity b = create_alive_entity();

    entities.destroy_entity(a);
    update();

    auto result = collect_entities();

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].index, b.index);
}

TEST_F(EntitySystemTest, Iteration_DoesNotReturnFreeSlots)
{
    Entity entity = create_alive_entity();

    entities.destroy_entity(entity);
    update();

    EXPECT_TRUE(collect_entities().empty());
}

TEST_F(EntitySystemTest, Iteration_ReturnedEntitiesAreAlive)
{
    for (int i = 0; i < 100; ++i)
        create_alive_entity();

    for (Entity entity : collect_entities())
        EXPECT_TRUE(entities.is_alive(entity));
}

TEST_F(EntitySystemTest, Iteration_ReturnsUniqueEntities)
{
    for (int i = 0; i < 100; ++i)
        create_alive_entity();

    std::unordered_set<uint32_t> indices;

    for (Entity entity : collect_entities())
        EXPECT_TRUE(indices.insert(entity.index).second);
}

// ============================================================
// EVENT BATCHING
// ============================================================

TEST_F(EntitySystemTest, EventBatching_MultipleAdditions)
{
    std::vector<Entity> created;

    for (int i = 0; i < 100; ++i)
        created.push_back(entities.create_entity());

    update();

    EXPECT_EQ(entities.entity_count(), 100);

    for (Entity entity : created)
        EXPECT_TRUE(entities.is_alive(entity));
}

TEST_F(EntitySystemTest, EventBatching_MultipleRemovals)
{
    std::vector<Entity> created;

    for (int i = 0; i < 100; ++i)
        created.push_back(create_alive_entity());

    for (Entity entity : created)
        EXPECT_TRUE(entities.destroy_entity(entity));

    update();

    EXPECT_EQ(entities.entity_count(), 0);
}

TEST_F(EntitySystemTest, EventBatching_AdditionsAndRemovals)
{
    std::vector<Entity> created;

    for (int i = 0; i < 100; ++i)
        created.push_back(create_alive_entity());

    for (int i = 0; i < 50; ++i)
        entities.destroy_entity(created[i]);

    for (int i = 0; i < 50; ++i)
        entities.create_entity();

    update();

    EXPECT_EQ(entities.entity_count(), 100);
}

TEST_F(EntitySystemTest, EventBatching_InvalidEventDoesNotBlockValidEvents)
{
    Entity a = entities.create_entity();
    Entity b = entities.create_entity();

    // An event with an invalid index must not prevent
    // valid events from being processed.
    events.publish(EntityAddedEvent{Entity{.index = 1000, .generation = 0}});

    update();

    EXPECT_TRUE(entities.is_alive(a));
    EXPECT_TRUE(entities.is_alive(b));
    EXPECT_EQ(entities.entity_count(), 2);
}

// ============================================================
// INVALID HANDLES
// ============================================================

TEST_F(EntitySystemTest, InvalidHandle_IsAliveReturnsFalse)
{
    Entity invalid{.index = 1000, .generation = 0};

    EXPECT_FALSE(entities.is_alive(invalid));
}

TEST_F(EntitySystemTest, InvalidHandle_IsDestructibleReturnsFalse)
{
    Entity invalid{.index = 1000, .generation = 0};

    EXPECT_FALSE(entities.is_destructible(invalid));
}

TEST_F(EntitySystemTest, InvalidHandle_DestroyReturnsFalse)
{
    Entity invalid{.index = 1000, .generation = 0};

    EXPECT_FALSE(entities.destroy_entity(invalid));
}

TEST_F(EntitySystemTest, InvalidHandle_MaxIndexReturnsFalse)
{
    Entity invalid{.index = std::numeric_limits<uint32_t>::max(), .generation = 0};

    EXPECT_FALSE(entities.is_alive(invalid));
    EXPECT_FALSE(entities.is_destructible(invalid));
    EXPECT_FALSE(entities.destroy_entity(invalid));
}

TEST_F(EntitySystemTest, InvalidHandle_WrongGenerationReturnsFalse)
{
    Entity entity = create_alive_entity();

    Entity invalid{.index = entity.index, .generation = entity.generation + 1};

    EXPECT_FALSE(entities.is_alive(invalid));
    EXPECT_FALSE(entities.is_destructible(invalid));
    EXPECT_FALSE(entities.destroy_entity(invalid));
}

// ============================================================
// STRESS TESTS
// ============================================================

TEST_F(EntitySystemTest, Stress_Create10000Entities)
{
    constexpr uint32_t count = 10000;

    std::vector<Entity> created;
    created.reserve(count);

    for (uint32_t i = 0; i < count; ++i)
        created.push_back(entities.create_entity());

    update();

    EXPECT_EQ(entities.entity_count(), count);

    for (Entity entity : created)
        EXPECT_TRUE(entities.is_alive(entity));

    EXPECT_EQ(collect_entities().size(), count);
}

TEST_F(EntitySystemTest, Stress_Destroy10000Entities)
{
    constexpr uint32_t count = 10000;

    std::vector<Entity> created;

    for (uint32_t i = 0; i < count; ++i)
        created.push_back(create_alive_entity());

    for (Entity entity : created)
        EXPECT_TRUE(entities.destroy_entity(entity));

    update();

    EXPECT_EQ(entities.entity_count(), 0);
    EXPECT_TRUE(collect_entities().empty());
}

TEST_F(EntitySystemTest, Stress_CreateDestroyAndReuse10000Entities)
{
    constexpr uint32_t count = 10000;

    std::vector<Entity> original;

    for (uint32_t i = 0; i < count; ++i)
        original.push_back(create_alive_entity());

    for (Entity entity : original)
        entities.destroy_entity(entity);

    update();

    std::vector<Entity> replacements;

    for (uint32_t i = 0; i < count; ++i)
        replacements.push_back(entities.create_entity());

    update();

    EXPECT_EQ(entities.entity_count(), count);

    for (Entity entity : replacements)
        EXPECT_TRUE(entities.is_alive(entity));

    for (Entity entity : original)
        EXPECT_FALSE(entities.is_alive(entity));
}

TEST_F(EntitySystemTest, Stress_RepeatedCreateAndDestroy)
{
    constexpr uint32_t iterations = 1000;

    for (uint32_t i = 0; i < iterations; ++i)
    {
        Entity entity = entities.create_entity();
        update();

        ASSERT_TRUE(entities.is_alive(entity));

        ASSERT_TRUE(entities.destroy_entity(entity));
        update();

        ASSERT_FALSE(entities.is_alive(entity));
        ASSERT_EQ(entities.entity_count(), 0);
    }
}

TEST_F(EntitySystemTest, Stress_MixedCreationAndDestruction)
{
    constexpr uint32_t count = 5000;

    std::vector<Entity> created;

    for (uint32_t i = 0; i < count; ++i)
        created.push_back(create_alive_entity());

    for (uint32_t i = 0; i < count; i += 2)
        entities.destroy_entity(created[i]);

    update();

    EXPECT_EQ(entities.entity_count(), count / 2);

    for (uint32_t i = 0; i < count; ++i)
    {
        if (i % 2 == 0)
            EXPECT_FALSE(entities.is_alive(created[i]));
        else
            EXPECT_TRUE(entities.is_alive(created[i]));
    }

    EXPECT_EQ(collect_entities().size(), count / 2);
}

// ============================================================
// DETERMINISTIC RANDOMIZED STRESS
// ============================================================

TEST_F(EntitySystemTest, Stress_RandomizedLifecycleOperations)
{
    constexpr uint32_t operations = 50000;

    std::mt19937 rng(0xC0FFEE);
    std::vector<Entity> active;

    for (uint32_t i = 0; i < operations; ++i)
    {
        const bool should_create = active.empty() || (rng() % 100 < 55);

        if (should_create)
        {
            Entity entity = entities.create_entity();
            active.push_back(entity);
        }
        else
        {
            const size_t index = rng() % active.size();

            EXPECT_TRUE(entities.destroy_entity(active[index]));

            active.erase(active.begin() + index);
        }

        // Process all accumulated events.
        update();

        // All remaining tracked handles must be alive.
        for (Entity entity : active)
            EXPECT_TRUE(entities.is_alive(entity));

        EXPECT_EQ(entities.entity_count(), active.size());
    }

    // Final cleanup.
    for (Entity entity : active)
        EXPECT_TRUE(entities.destroy_entity(entity));

    update();

    EXPECT_EQ(entities.entity_count(), 0);
    EXPECT_TRUE(collect_entities().empty());
}