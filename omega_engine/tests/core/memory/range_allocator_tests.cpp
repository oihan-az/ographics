#include <gtest/gtest.h>

#include <omega/core/memory/range_allocator.hpp>

TEST(RangeAllocatorTests, InitiallyEntireCapacityIsAvailable)
{
    omega::RangeAllocator allocator(100);

    EXPECT_EQ(allocator.capacity(), 100);
    EXPECT_EQ(allocator.available(), 100);
}

TEST(RangeAllocatorTests, AllocatesRange)
{
    omega::RangeAllocator allocator(100);

    uint32_t offset = 999;

    ASSERT_TRUE(allocator.allocate(20, offset));

    EXPECT_EQ(offset, 0);
    EXPECT_EQ(allocator.available(), 80);
}

TEST(RangeAllocatorTests, AllocatesContiguousRanges)
{
    omega::RangeAllocator allocator(100);

    uint32_t first = 0;
    uint32_t second = 0;

    ASSERT_TRUE(allocator.allocate(20, first));
    ASSERT_TRUE(allocator.allocate(30, second));

    EXPECT_EQ(first, 0);
    EXPECT_EQ(second, 20);
}

TEST(RangeAllocatorTests, RejectsAllocationWhenInsufficientSpace)
{
    omega::RangeAllocator allocator(10);

    uint32_t offset = 0;

    EXPECT_FALSE(allocator.allocate(11, offset));
    EXPECT_EQ(allocator.available(), 10);
}

TEST(RangeAllocatorTests, ReusesReleasedRange)
{
    omega::RangeAllocator allocator(100);

    uint32_t first = 0;
    uint32_t second = 0;

    ASSERT_TRUE(allocator.allocate(20, first));
    ASSERT_TRUE(allocator.allocate(20, second));
    ASSERT_TRUE(allocator.release(first, 20));

    uint32_t reused = 0;

    ASSERT_TRUE(allocator.allocate(15, reused));

    EXPECT_EQ(reused, first);
}

TEST(RangeAllocatorTests, MergesAdjacentReleasedRanges)
{
    omega::RangeAllocator allocator(100);

    uint32_t first = 0;
    uint32_t second = 0;
    uint32_t third = 0;

    ASSERT_TRUE(allocator.allocate(20, first));
    ASSERT_TRUE(allocator.allocate(20, second));
    ASSERT_TRUE(allocator.allocate(20, third));

    ASSERT_TRUE(allocator.release(first, 20));
    ASSERT_TRUE(allocator.release(second, 20));

    uint32_t merged = 0;

    ASSERT_TRUE(allocator.allocate(40, merged));
    EXPECT_EQ(merged, 0);
}

TEST(RangeAllocatorTests, RejectsDoubleRelease)
{
    omega::RangeAllocator allocator(100);

    uint32_t offset = 0;

    ASSERT_TRUE(allocator.allocate(20, offset));
    ASSERT_TRUE(allocator.release(offset, 20));

    EXPECT_FALSE(allocator.release(offset, 20));
    EXPECT_EQ(allocator.available(), 100);
}

TEST(RangeAllocatorTests, RejectsOverlappingRelease)
{
    omega::RangeAllocator allocator(100);

    uint32_t offset = 0;

    ASSERT_TRUE(allocator.allocate(40, offset));
    ASSERT_TRUE(allocator.release(offset, 20));

    EXPECT_FALSE(allocator.release(10, 20));
}

TEST(RangeAllocatorTests, RejectsOutOfBoundsRelease)
{
    omega::RangeAllocator allocator(100);

    EXPECT_FALSE(allocator.release(90, 20));
    EXPECT_FALSE(allocator.release(100, 1));
}

TEST(RangeAllocatorTests, RejectsZeroSizeAllocationAndRelease)
{
    omega::RangeAllocator allocator(100);

    uint32_t offset = 0;

    EXPECT_FALSE(allocator.allocate(0, offset));
    EXPECT_FALSE(allocator.release(0, 0));
}

TEST(RangeAllocatorTests, ResetRestoresEntireCapacity)
{
    omega::RangeAllocator allocator(100);

    uint32_t offset = 0;
    ASSERT_TRUE(allocator.allocate(50, offset));

    allocator.reset(200);

    EXPECT_EQ(allocator.capacity(), 200);
    EXPECT_EQ(allocator.available(), 200);
}