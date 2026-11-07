#include <gtest/gtest.h>

#include <omega/core/component/component_storage.hpp>
#include <omega/core/entity/entity_indexer.hpp>

#include <stdexcept>

namespace
{

struct TestComponent
{
    int value = 0;
};

// Tests insertion, lookup, and removal of components
TEST(ComponentStorageTests, AddGetAndRemove)
{
    omega::ComponentStorage<TestComponent> storage;

    omega::Entity entity_a{0, 0};
    omega::Entity entity_b{1, 0};

    // Add two components
    EXPECT_TRUE(storage.add(entity_a, {10}));
    EXPECT_TRUE(storage.add(entity_b, {20}));

    EXPECT_EQ(storage.size(), 2);

    // Verify component lookup
    EXPECT_TRUE(storage.contains(entity_a));
    EXPECT_TRUE(storage.contains(entity_b));

    ASSERT_NE(storage.get(entity_a), nullptr);
    ASSERT_NE(storage.get(entity_b), nullptr);

    EXPECT_EQ(storage.get(entity_a)->value, 10);
    EXPECT_EQ(storage.get(entity_b)->value, 20);

    // Remove one component and verify the other remains accessible
    EXPECT_TRUE(storage.remove(entity_a));

    EXPECT_EQ(storage.size(), 1);
    EXPECT_FALSE(storage.contains(entity_a));
    EXPECT_TRUE(storage.contains(entity_b));

    ASSERT_NE(storage.get(entity_b), nullptr);
    EXPECT_EQ(storage.get(entity_b)->value, 20);
}

// Tests that invalid entity handles cannot be added
TEST(ComponentStorageTests, RejectsInvalidEntity)
{
    omega::ComponentStorage<TestComponent> storage;

    EXPECT_FALSE(storage.add(omega::Entity::invalid(), {10}));
    EXPECT_EQ(storage.size(), 0);
}

// Tests that duplicate entities and conflicting generations are rejected
TEST(ComponentStorageTests, RejectsDuplicateEntities)
{
    omega::ComponentStorage<TestComponent> storage;

    omega::Entity entity{0, 0};
    omega::Entity newer_generation{0, 1};

    EXPECT_TRUE(storage.add(entity, {10}));

    // Reject adding the same entity twice
    EXPECT_FALSE(storage.add(entity, {20}));

    // Reject another generation while the index is still occupied
    EXPECT_FALSE(storage.add(newer_generation, {30}));

    EXPECT_EQ(storage.size(), 1);
    ASSERT_NE(storage.get(entity), nullptr);
    EXPECT_EQ(storage.get(entity)->value, 10);
}

// Tests that stale or missing entity handles cannot remove components
TEST(ComponentStorageTests, RejectsInvalidRemoval)
{
    omega::ComponentStorage<TestComponent> storage;

    omega::Entity entity{0, 1};
    omega::Entity stale_entity{0, 0};
    omega::Entity missing_entity{1, 0};

    EXPECT_TRUE(storage.add(entity, {10}));

    EXPECT_FALSE(storage.remove(stale_entity));
    EXPECT_FALSE(storage.remove(missing_entity));
    EXPECT_FALSE(storage.remove(omega::Entity::invalid()));

    EXPECT_EQ(storage.size(), 1);
    EXPECT_TRUE(storage.contains(entity));
}

// Tests that removing the first element moves the last element into its place
TEST(ComponentStorageTests, RemoveFirstElement)
{
    omega::ComponentStorage<TestComponent> storage;

    omega::Entity entity_a{0, 0};
    omega::Entity entity_b{1, 0};
    omega::Entity entity_c{2, 0};

    EXPECT_TRUE(storage.add(entity_a, {10}));
    EXPECT_TRUE(storage.add(entity_b, {20}));
    EXPECT_TRUE(storage.add(entity_c, {30}));

    EXPECT_TRUE(storage.remove(entity_a));

    EXPECT_EQ(storage.size(), 2);

    // The last element should now occupy the removed element's position
    EXPECT_EQ(storage.entity_at(0), entity_c);
    EXPECT_EQ(storage.entity_at(1), entity_b);

    ASSERT_NE(storage.get(entity_c), nullptr);
    EXPECT_EQ(storage.get(entity_c)->value, 30);

    EXPECT_FALSE(storage.contains(entity_a));
    EXPECT_TRUE(storage.contains(entity_b));
    EXPECT_TRUE(storage.contains(entity_c));
}

// Tests that removing a middle element preserves the remaining mappings
TEST(ComponentStorageTests, RemoveMiddleElement)
{
    omega::ComponentStorage<TestComponent> storage;

    omega::Entity entity_a{0, 0};
    omega::Entity entity_b{1, 0};
    omega::Entity entity_c{2, 0};

    EXPECT_TRUE(storage.add(entity_a, {10}));
    EXPECT_TRUE(storage.add(entity_b, {20}));
    EXPECT_TRUE(storage.add(entity_c, {30}));

    EXPECT_TRUE(storage.remove(entity_b));

    EXPECT_EQ(storage.size(), 2);
    EXPECT_EQ(storage.entity_at(1), entity_c);

    EXPECT_FALSE(storage.contains(entity_b));
    EXPECT_TRUE(storage.contains(entity_a));
    EXPECT_TRUE(storage.contains(entity_c));
}

// Tests removal of the last element, which requires no element movement
TEST(ComponentStorageTests, RemoveLastElement)
{
    omega::ComponentStorage<TestComponent> storage;

    omega::Entity entity_a{0, 0};
    omega::Entity entity_b{1, 0};

    EXPECT_TRUE(storage.add(entity_a, {10}));
    EXPECT_TRUE(storage.add(entity_b, {20}));

    EXPECT_TRUE(storage.remove(entity_b));

    EXPECT_EQ(storage.size(), 1);
    EXPECT_EQ(storage.entity_at(0), entity_a);
    EXPECT_TRUE(storage.contains(entity_a));
    EXPECT_FALSE(storage.contains(entity_b));
}

// Tests sparse-array expansion when entity indices are not consecutive
TEST(ComponentStorageTests, HandlesSparseEntityIndices)
{
    omega::ComponentStorage<TestComponent> storage;

    omega::Entity entity_a{2, 0};
    omega::Entity entity_b{10, 0};

    EXPECT_TRUE(storage.add(entity_a, {10}));
    EXPECT_TRUE(storage.add(entity_b, {20}));

    EXPECT_TRUE(storage.contains(entity_a));
    EXPECT_TRUE(storage.contains(entity_b));

    EXPECT_EQ(storage.get(entity_a)->value, 10);
    EXPECT_EQ(storage.get(entity_b)->value, 20);
}

// Tests that an index can be reused with a new generation after removal
TEST(ComponentStorageTests, SupportsEntityIndexReuse)
{
    omega::ComponentStorage<TestComponent> storage;

    omega::Entity old_entity{0, 0};
    omega::Entity new_entity{0, 1};

    EXPECT_TRUE(storage.add(old_entity, {10}));
    EXPECT_TRUE(storage.remove(old_entity));

    EXPECT_FALSE(storage.contains(old_entity));

    EXPECT_TRUE(storage.add(new_entity, {20}));

    EXPECT_FALSE(storage.contains(old_entity));
    EXPECT_TRUE(storage.contains(new_entity));
    EXPECT_EQ(storage.get(new_entity)->value, 20);
}

// Tests that clear removes all components and resets sparse lookup
TEST(ComponentStorageTests, Clear)
{
    omega::ComponentStorage<TestComponent> storage;

    omega::Entity entity_a{0, 0};
    omega::Entity entity_b{5, 0};

    EXPECT_TRUE(storage.add(entity_a, {10}));
    EXPECT_TRUE(storage.add(entity_b, {20}));

    storage.clear();

    EXPECT_EQ(storage.size(), 0);
    EXPECT_FALSE(storage.contains(entity_a));
    EXPECT_FALSE(storage.contains(entity_b));
    EXPECT_EQ(storage.get(entity_a), nullptr);

    // Verify that the storage can be reused after clearing
    EXPECT_TRUE(storage.add(entity_a, {30}));
    EXPECT_EQ(storage.get(entity_a)->value, 30);
}

} // namespace
