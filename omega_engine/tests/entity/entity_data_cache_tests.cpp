#include <gtest/gtest.h>

#include <omega/core/entity/entity_data_cache.hpp>

TEST(EntityDataCacheTests, NewEntryDoesNotExist)
{
    omega::EntityDataCache<int> cache;
    omega::Entity entity{1, 0};

    EXPECT_FALSE(cache.contains(entity));
    EXPECT_TRUE(cache.is_dirty(entity));
    EXPECT_EQ(cache.get(entity), nullptr);
}

TEST(EntityDataCacheTests, SetCreatesValidEntry)
{
    omega::EntityDataCache<int> cache;
    omega::Entity entity{1, 0};

    cache.set(entity, 42);

    ASSERT_TRUE(cache.contains(entity));
    EXPECT_FALSE(cache.is_dirty(entity));
    ASSERT_NE(cache.get(entity), nullptr);
    EXPECT_EQ(*cache.get(entity), 42);
}

TEST(EntityDataCacheTests, InvalidateMarksExistingEntryDirty)
{
    omega::EntityDataCache<int> cache;
    omega::Entity entity{1, 0};

    cache.set(entity, 42);
    cache.invalidate(entity);

    EXPECT_TRUE(cache.is_dirty(entity));
    EXPECT_EQ(*cache.get(entity), 42);
}

TEST(EntityDataCacheTests, InvalidateDoesNotCreateEntry)
{
    omega::EntityDataCache<int> cache;
    omega::Entity entity{1, 0};

    cache.invalidate(entity);

    EXPECT_FALSE(cache.contains(entity));
}

TEST(EntityDataCacheTests, SetRefreshesExistingEntry)
{
    omega::EntityDataCache<int> cache;
    omega::Entity entity{1, 0};

    cache.set(entity, 42);
    cache.invalidate(entity);
    cache.set(entity, 100);

    EXPECT_FALSE(cache.is_dirty(entity));
    EXPECT_EQ(*cache.get(entity), 100);
}

TEST(EntityDataCacheTests, EraseRemovesEntry)
{
    omega::EntityDataCache<int> cache;
    omega::Entity entity{1, 0};

    cache.set(entity, 42);
    cache.erase(entity);

    EXPECT_FALSE(cache.contains(entity));
    EXPECT_EQ(cache.get(entity), nullptr);
}

TEST(EntityDataCacheTests, ClearRemovesAllEntries)
{
    omega::EntityDataCache<int> cache;

    omega::Entity first{1, 0};
    omega::Entity second{2, 0};

    cache.set(first, 10);
    cache.set(second, 20);

    cache.clear();

    EXPECT_FALSE(cache.contains(first));
    EXPECT_FALSE(cache.contains(second));
}
