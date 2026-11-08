
#include <gtest/gtest.h>

#include <omega/core/entity/entity_system.hpp>
#include <omega/core/event/event_system.hpp>
#include <omega/core/hierarchy/hierarchy_event.hpp>
#include <omega/core/hierarchy/hierarchy_system.hpp>

#include <algorithm>
#include <vector>

using namespace omega;

class HierarchySystemTest : public ::testing::Test
{
  protected:
    EventSystem events;
    EntitySystem entities{events};
    HierarchySystem hierarchy{events, entities};

    void update()
    {
        events.swap_all_buffers();
        entities.update();
        hierarchy.update();
    }

    Entity create_alive_entity()
    {
        Entity entity = entities.create_entity();
        update();
        return entity;
    }

    void expect_parent(Entity child, Entity parent)
    {
        EXPECT_TRUE(hierarchy.has_parent(child));
        EXPECT_EQ(hierarchy.get_parent(child), parent);
    }

    void expect_no_parent(Entity child)
    {
        EXPECT_FALSE(hierarchy.has_parent(child));
        EXPECT_FALSE(hierarchy.get_parent(child).is_valid());
    }

    void expect_child(Entity parent, Entity child)
    {
        const auto& children = hierarchy.get_children(parent);

        EXPECT_EQ(std::count(children.begin(), children.end(), child), 1);
    }

    void expect_no_child(Entity parent, Entity child)
    {
        const auto& children = hierarchy.get_children(parent);

        EXPECT_EQ(std::count(children.begin(), children.end(), child), 0);
    }
};

// ============================================================
// PARENT ASSIGNMENT
// ============================================================

TEST_F(HierarchySystemTest, SetParent_AssignsParentAfterUpdate)
{
    Entity parent = create_alive_entity();
    Entity child = create_alive_entity();

    ASSERT_TRUE(hierarchy.set_parent(child, parent));

    update();

    expect_parent(child, parent);
}

TEST_F(HierarchySystemTest, SetParent_AddsChildToParent)
{
    Entity parent = create_alive_entity();
    Entity child = create_alive_entity();

    hierarchy.set_parent(child, parent);
    update();

    expect_child(parent, child);
}

TEST_F(HierarchySystemTest, SetParent_MultipleChildren)
{
    Entity parent = create_alive_entity();
    Entity child_a = create_alive_entity();
    Entity child_b = create_alive_entity();
    Entity child_c = create_alive_entity();

    ASSERT_TRUE(hierarchy.set_parent(child_a, parent));
    ASSERT_TRUE(hierarchy.set_parent(child_b, parent));
    ASSERT_TRUE(hierarchy.set_parent(child_c, parent));

    update();

    const auto& children = hierarchy.get_children(parent);

    ASSERT_EQ(children.size(), 3);

    expect_child(parent, child_a);
    expect_child(parent, child_b);
    expect_child(parent, child_c);
}

TEST_F(HierarchySystemTest, SetParent_ReassignsParent)
{
    Entity first_parent = create_alive_entity();
    Entity second_parent = create_alive_entity();
    Entity child = create_alive_entity();

    hierarchy.set_parent(child, first_parent);
    update();

    ASSERT_TRUE(hierarchy.set_parent(child, second_parent));
    update();

    expect_parent(child, second_parent);
    expect_no_child(first_parent, child);
    expect_child(second_parent, child);
}

TEST_F(HierarchySystemTest, SetParent_SameParentIsNoOp)
{
    Entity parent = create_alive_entity();
    Entity child = create_alive_entity();

    hierarchy.set_parent(child, parent);
    update();

    ASSERT_TRUE(hierarchy.set_parent(child, parent));
    update();

    expect_parent(child, parent);

    const auto& children = hierarchy.get_children(parent);
    EXPECT_EQ(children.size(), 1);
}

TEST_F(HierarchySystemTest, SetParent_DoesNotModifyCommittedStateImmediately)
{
    Entity parent = create_alive_entity();
    Entity child = create_alive_entity();

    ASSERT_TRUE(hierarchy.set_parent(child, parent));

    expect_no_parent(child);
    EXPECT_TRUE(hierarchy.get_children(parent).empty());

    update();

    expect_parent(child, parent);
    expect_child(parent, child);
}

// ============================================================
// PARENT CLEARING
// ============================================================

TEST_F(HierarchySystemTest, ClearParent_RemovesParentAfterUpdate)
{
    Entity parent = create_alive_entity();
    Entity child = create_alive_entity();

    hierarchy.set_parent(child, parent);
    update();

    ASSERT_TRUE(hierarchy.clear_parent(child));

    update();

    expect_no_parent(child);
    expect_no_child(parent, child);
}

TEST_F(HierarchySystemTest, ClearParent_RootEntityIsNoOp)
{
    Entity child = create_alive_entity();

    EXPECT_TRUE(hierarchy.clear_parent(child));

    update();

    expect_no_parent(child);
}

TEST_F(HierarchySystemTest, ClearParent_DoesNotModifyCommittedStateImmediately)
{
    Entity parent = create_alive_entity();
    Entity child = create_alive_entity();

    hierarchy.set_parent(child, parent);
    update();

    ASSERT_TRUE(hierarchy.clear_parent(child));

    expect_parent(child, parent);
    expect_child(parent, child);

    update();

    expect_no_parent(child);
    expect_no_child(parent, child);
}

TEST_F(HierarchySystemTest, ClearParent_DoesNotAffectOtherChildren)
{
    Entity parent = create_alive_entity();
    Entity child_a = create_alive_entity();
    Entity child_b = create_alive_entity();

    hierarchy.set_parent(child_a, parent);
    hierarchy.set_parent(child_b, parent);
    update();

    hierarchy.clear_parent(child_a);
    update();

    expect_no_parent(child_a);
    expect_parent(child_b, parent);

    expect_no_child(parent, child_a);
    expect_child(parent, child_b);
}

// ============================================================
// VALIDATION
// ============================================================

TEST_F(HierarchySystemTest, Validation_InvalidChild)
{
    Entity parent = create_alive_entity();
    Entity invalid = Entity::invalid();

    EXPECT_FALSE(hierarchy.set_parent(invalid, parent));
}

TEST_F(HierarchySystemTest, Validation_InvalidParent)
{
    Entity child = create_alive_entity();

    EXPECT_FALSE(hierarchy.set_parent(child, Entity::invalid()));
}

TEST_F(HierarchySystemTest, Validation_DeadChild)
{
    Entity child = create_alive_entity();
    Entity parent = create_alive_entity();

    entities.destroy_entity(child);

    EXPECT_FALSE(hierarchy.set_parent(child, parent));
}

TEST_F(HierarchySystemTest, Validation_DeadParent)
{
    Entity child = create_alive_entity();
    Entity parent = create_alive_entity();

    entities.destroy_entity(parent);

    EXPECT_FALSE(hierarchy.set_parent(child, parent));
}

TEST_F(HierarchySystemTest, Validation_StaleChild)
{
    Entity child = create_alive_entity();
    Entity parent = create_alive_entity();

    Entity stale{.index = child.index, .generation = child.generation + 1};

    EXPECT_FALSE(hierarchy.set_parent(stale, parent));
}

TEST_F(HierarchySystemTest, Validation_StaleParent)
{
    Entity child = create_alive_entity();
    Entity parent = create_alive_entity();

    Entity stale{.index = parent.index, .generation = parent.generation + 1};

    EXPECT_FALSE(hierarchy.set_parent(child, stale));
}

TEST_F(HierarchySystemTest, Validation_CannotParentEntityToItself)
{
    Entity entity = create_alive_entity();

    EXPECT_FALSE(hierarchy.set_parent(entity, entity));
}

TEST_F(HierarchySystemTest, Validation_CannotCreateDirectCycle)
{
    Entity a = create_alive_entity();
    Entity b = create_alive_entity();

    ASSERT_TRUE(hierarchy.set_parent(a, b));
    update();

    EXPECT_FALSE(hierarchy.set_parent(b, a));
}

TEST_F(HierarchySystemTest, Validation_CannotCreateIndirectCycle)
{
    Entity a = create_alive_entity();
    Entity b = create_alive_entity();
    Entity c = create_alive_entity();

    ASSERT_TRUE(hierarchy.set_parent(b, a));
    ASSERT_TRUE(hierarchy.set_parent(c, b));
    update();

    EXPECT_FALSE(hierarchy.set_parent(a, c));
}

TEST_F(HierarchySystemTest, Validation_CannotCreateCycleUsingPendingChanges)
{
    Entity a = create_alive_entity();
    Entity b = create_alive_entity();

    ASSERT_TRUE(hierarchy.set_parent(a, b));

    // The first change has not been committed yet
    EXPECT_FALSE(hierarchy.set_parent(b, a));
}

// ============================================================
// PENDING CHANGES
// ============================================================

TEST_F(HierarchySystemTest, PendingChanges_LastParentAssignmentWins)
{
    Entity first_parent = create_alive_entity();
    Entity second_parent = create_alive_entity();
    Entity child = create_alive_entity();

    ASSERT_TRUE(hierarchy.set_parent(child, first_parent));
    ASSERT_TRUE(hierarchy.set_parent(child, second_parent));

    update();

    expect_parent(child, second_parent);
    expect_no_child(first_parent, child);
    expect_child(second_parent, child);
}

TEST_F(HierarchySystemTest, PendingChanges_ClearAfterSetWins)
{
    Entity parent = create_alive_entity();
    Entity child = create_alive_entity();

    ASSERT_TRUE(hierarchy.set_parent(child, parent));
    ASSERT_TRUE(hierarchy.clear_parent(child));

    update();

    expect_no_parent(child);
    expect_no_child(parent, child);
}

TEST_F(HierarchySystemTest, PendingChanges_SetAfterClearWins)
{
    Entity first_parent = create_alive_entity();
    Entity second_parent = create_alive_entity();
    Entity child = create_alive_entity();

    hierarchy.set_parent(child, first_parent);
    update();

    ASSERT_TRUE(hierarchy.clear_parent(child));
    ASSERT_TRUE(hierarchy.set_parent(child, second_parent));

    update();

    expect_parent(child, second_parent);
    expect_no_child(first_parent, child);
    expect_child(second_parent, child);
}

TEST_F(HierarchySystemTest, PendingChanges_ClearTwiceIsNoOp)
{
    Entity parent = create_alive_entity();
    Entity child = create_alive_entity();

    hierarchy.set_parent(child, parent);
    update();

    ASSERT_TRUE(hierarchy.clear_parent(child));
    ASSERT_TRUE(hierarchy.clear_parent(child));

    update();

    expect_no_parent(child);
    expect_no_child(parent, child);
}

TEST_F(HierarchySystemTest, PendingChanges_DestroyedChildDiscardsPendingAssignment)
{
    Entity parent = create_alive_entity();
    Entity child = create_alive_entity();

    ASSERT_TRUE(hierarchy.set_parent(child, parent));
    ASSERT_TRUE(entities.destroy_entity(child));

    update();

    expect_no_parent(child);
    expect_no_child(parent, child);
}

TEST_F(HierarchySystemTest, PendingChanges_DestroyedParentDiscardsPendingAssignment)
{
    Entity parent = create_alive_entity();
    Entity child = create_alive_entity();

    ASSERT_TRUE(hierarchy.set_parent(child, parent));
    ASSERT_TRUE(entities.destroy_entity(parent));

    update();

    expect_no_parent(child);
    EXPECT_TRUE(hierarchy.get_children(parent).empty());
}

// ============================================================
// ENTITY DESTRUCTION
// ============================================================

TEST_F(HierarchySystemTest, Destruction_DestroyLeaf)
{
    Entity parent = create_alive_entity();
    Entity child = create_alive_entity();

    hierarchy.set_parent(child, parent);
    update();

    ASSERT_TRUE(entities.destroy_entity(child));
    update();

    EXPECT_TRUE(hierarchy.get_children(parent).empty());
    expect_no_parent(child);
}

TEST_F(HierarchySystemTest, Destruction_DestroyParentReparentsChildrenToGrandparent)
{
    Entity grandparent = create_alive_entity();
    Entity parent = create_alive_entity();
    Entity child = create_alive_entity();

    hierarchy.set_parent(parent, grandparent);
    hierarchy.set_parent(child, parent);
    update();

    ASSERT_TRUE(entities.destroy_entity(parent));
    update();

    expect_parent(child, grandparent);
    expect_no_child(parent, child);
    expect_child(grandparent, child);
}

TEST_F(HierarchySystemTest, Destruction_DestroyRootMakesChildrenRoots)
{
    Entity root = create_alive_entity();
    Entity child_a = create_alive_entity();
    Entity child_b = create_alive_entity();

    hierarchy.set_parent(child_a, root);
    hierarchy.set_parent(child_b, root);
    update();

    ASSERT_TRUE(entities.destroy_entity(root));
    update();

    expect_no_parent(child_a);
    expect_no_parent(child_b);
    EXPECT_TRUE(hierarchy.get_children(root).empty());
}

TEST_F(HierarchySystemTest, Destruction_DestroyEntityWithGrandchildren)
{
    Entity root = create_alive_entity();
    Entity parent = create_alive_entity();
    Entity child = create_alive_entity();
    Entity grandchild = create_alive_entity();

    hierarchy.set_parent(parent, root);
    hierarchy.set_parent(child, parent);
    hierarchy.set_parent(grandchild, child);
    update();

    ASSERT_TRUE(entities.destroy_entity(parent));
    update();

    expect_parent(child, root);
    expect_parent(grandchild, child);
    expect_child(root, child);
    expect_child(child, grandchild);
}

TEST_F(HierarchySystemTest, Destruction_DestroyMultipleAncestors)
{
    Entity a = create_alive_entity();
    Entity b = create_alive_entity();
    Entity c = create_alive_entity();
    Entity d = create_alive_entity();

    hierarchy.set_parent(b, a);
    hierarchy.set_parent(c, b);
    hierarchy.set_parent(d, c);
    update();

    ASSERT_TRUE(entities.destroy_entity(b));
    ASSERT_TRUE(entities.destroy_entity(c));

    update();

    // D must be reparented to the nearest surviving ancestor: A
    expect_parent(d, a);
    expect_child(a, d);

    EXPECT_TRUE(hierarchy.get_children(b).empty());
    EXPECT_TRUE(hierarchy.get_children(c).empty());
}

TEST_F(HierarchySystemTest, Destruction_DestroyMultipleAncestors_2)
{
    Entity a = create_alive_entity();
    Entity b = create_alive_entity();
    Entity c = create_alive_entity();
    Entity d = create_alive_entity();

    hierarchy.set_parent(b, a);
    hierarchy.set_parent(c, b);
    hierarchy.set_parent(d, c);
    update();

    ASSERT_TRUE(entities.destroy_entity(c));
    ASSERT_TRUE(entities.destroy_entity(b));

    update();

    // D must be reparented to the nearest surviving ancestor: A
    expect_parent(d, a);
    expect_child(a, d);

    EXPECT_TRUE(hierarchy.get_children(b).empty());
    EXPECT_TRUE(hierarchy.get_children(c).empty());
}

TEST_F(HierarchySystemTest, Destruction_DestroyAllAncestorsMakesSurvivorRoot)
{
    Entity a = create_alive_entity();
    Entity b = create_alive_entity();
    Entity c = create_alive_entity();
    Entity d = create_alive_entity();

    hierarchy.set_parent(b, a);
    hierarchy.set_parent(c, b);
    hierarchy.set_parent(d, c);
    update();

    ASSERT_TRUE(entities.destroy_entity(a));
    ASSERT_TRUE(entities.destroy_entity(b));
    ASSERT_TRUE(entities.destroy_entity(c));

    update();

    expect_no_parent(d);
}

TEST_F(HierarchySystemTest, Destruction_DestroyParentDoesNotAffectOtherBranches)
{
    Entity root = create_alive_entity();
    Entity parent = create_alive_entity();
    Entity sibling = create_alive_entity();
    Entity child = create_alive_entity();

    hierarchy.set_parent(parent, root);
    hierarchy.set_parent(sibling, root);
    hierarchy.set_parent(child, parent);
    update();

    ASSERT_TRUE(entities.destroy_entity(parent));
    update();

    expect_parent(child, root);
    expect_parent(sibling, root);

    expect_child(root, child);
    expect_child(root, sibling);
}

TEST_F(HierarchySystemTest, Destruction_DestroyAndReuseIndexDoesNotInheritHierarchy)
{
    Entity parent = create_alive_entity();
    Entity child = create_alive_entity();

    hierarchy.set_parent(child, parent);
    update();

    ASSERT_TRUE(entities.destroy_entity(parent));
    update();

    Entity replacement = create_alive_entity();

    EXPECT_EQ(replacement.index, parent.index);
    EXPECT_NE(replacement.generation, parent.generation);

    expect_no_parent(child);
    EXPECT_TRUE(hierarchy.get_children(replacement).empty());
}

// ============================================================
// ACCESSORS
// ============================================================

TEST_F(HierarchySystemTest, Accessors_RootHasNoParent)
{
    Entity root = create_alive_entity();

    EXPECT_FALSE(hierarchy.has_parent(root));
    EXPECT_FALSE(hierarchy.get_parent(root).is_valid());
}

TEST_F(HierarchySystemTest, Accessors_EntityWithoutChildrenReturnsEmptyList)
{
    Entity entity = create_alive_entity();

    EXPECT_TRUE(hierarchy.get_children(entity).empty());
}

TEST_F(HierarchySystemTest, Accessors_ParentAndChildrenRemainConsistent)
{
    Entity a = create_alive_entity();
    Entity b = create_alive_entity();
    Entity c = create_alive_entity();

    hierarchy.set_parent(b, a);
    hierarchy.set_parent(c, b);
    update();

    expect_parent(b, a);
    expect_parent(c, b);

    expect_child(a, b);
    expect_child(b, c);

    EXPECT_TRUE(hierarchy.get_children(c).empty());
}

// ============================================================
// EVENT PROCESSING
// ============================================================

TEST_F(HierarchySystemTest, EventProcessing_MultipleParentChanges)
{
    Entity a = create_alive_entity();
    Entity b = create_alive_entity();
    Entity c = create_alive_entity();
    Entity d = create_alive_entity();

    hierarchy.set_parent(b, a);
    hierarchy.set_parent(c, a);
    hierarchy.set_parent(d, a);

    update();

    ASSERT_EQ(hierarchy.get_children(a).size(), 3);

    hierarchy.clear_parent(b);
    hierarchy.clear_parent(c);
    update();

    ASSERT_EQ(hierarchy.get_children(a).size(), 1);
    expect_child(a, d);
}

TEST_F(HierarchySystemTest, EventProcessing_NoEventsDoesNotChangeHierarchy)
{
    Entity parent = create_alive_entity();
    Entity child = create_alive_entity();

    hierarchy.set_parent(child, parent);
    update();

    for (int i = 0; i < 10; ++i)
        update();

    expect_parent(child, parent);
    expect_child(parent, child);
}

TEST_F(HierarchySystemTest, EventProcessing_ChangesArePublishedForNextBuffer)
{
    Entity parent = create_alive_entity();
    Entity child = create_alive_entity();

    ASSERT_TRUE(hierarchy.set_parent(child, parent));

    update();

    events.swap_all_buffers();

    const auto& queue = events.get_queue<HierarchyModifiedEvent>().get_events();

    EXPECT_FALSE(queue.empty());
}


#include <cstdint>
#include <random>
#include <unordered_set>

// ============================================================
// STRESS TESTS
// ============================================================

TEST_F(HierarchySystemTest, Stress_RandomizedHierarchyOperations)
{
    constexpr uint32_t initial_count = 1000;
    constexpr uint32_t operations = 20000;
    constexpr uint32_t validation_interval = 25;

    std::mt19937 rng(0xC0FFEE);
    std::vector<Entity> active;
    active.reserve(initial_count + operations / 2);

    // Create the initial entity population.
    for (uint32_t i = 0; i < initial_count; ++i)
        active.push_back(entities.create_entity());

    update();

    ASSERT_EQ(active.size(), initial_count);

    const auto entity_key = [](Entity entity) -> uint64_t
    { return (static_cast<uint64_t>(entity.generation) << 32) | entity.index; };

    const auto validate_hierarchy = [&]()
    {
        std::unordered_set<uint64_t> alive_entities;

        for (Entity entity : active)
        {
            ASSERT_TRUE(entities.is_alive(entity));
            ASSERT_TRUE(alive_entities.insert(entity_key(entity)).second);
        }

        // Validate parent-child consistency.
        for (Entity parent : active)
        {
            const auto& children = hierarchy.get_children(parent);

            std::unordered_set<uint64_t> unique_children;

            for (Entity child : children)
            {
                EXPECT_TRUE(alive_entities.contains(entity_key(child)));

                // No duplicate child entries.
                EXPECT_TRUE(unique_children.insert(entity_key(child)).second);

                // Every child must point back to this parent.
                EXPECT_TRUE(hierarchy.has_parent(child));
                EXPECT_EQ(hierarchy.get_parent(child), parent);
            }
        }

        // Validate every parent relationship and check for cycles.
        for (Entity child : active)
        {
            if (!hierarchy.has_parent(child))
                continue;

            Entity parent = hierarchy.get_parent(child);

            ASSERT_TRUE(parent.is_valid());
            EXPECT_TRUE(alive_entities.contains(entity_key(parent)));
            EXPECT_NE(child, parent);

            const auto& siblings = hierarchy.get_children(parent);

            EXPECT_EQ(std::count(siblings.begin(), siblings.end(), child), 1);

            // Walk the ancestor chain to detect cycles.
            std::unordered_set<uint64_t> visited;
            Entity ancestor = child;

            while (ancestor.is_valid())
            {
                EXPECT_TRUE(visited.insert(entity_key(ancestor)).second);

                if (!visited.contains(entity_key(ancestor)))
                    break;

                ancestor = hierarchy.get_parent(ancestor);
            }
        }
    };

    for (uint32_t i = 0; i < operations; ++i)
    {
        // Always create if there are no available entities.
        const uint32_t operation = active.empty() ? 0 : rng() % 100;

        if (operation < 20)
        {
            // Create entity.
            active.push_back(entities.create_entity());
        }
        else if (operation < 60)
        {
            // Assign or change parent.
            const size_t child_index = rng() % active.size();
            const size_t parent_index = rng() % active.size();

            Entity child = active[child_index];
            Entity parent = active[parent_index];

            hierarchy.set_parent(child, parent);
        }
        else if (operation < 80)
        {
            // Clear parent.
            const size_t index = rng() % active.size();

            hierarchy.clear_parent(active[index]);
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
            validate_hierarchy();
    }

    // Final validation.
    validate_hierarchy();
}
