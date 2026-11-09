#include <gtest/gtest.h>

#include <omega/mesh/mesh_layout_group.hpp>

namespace
{

omega::MeshLayoutGroup make_group()
{
    return omega::MeshLayoutGroup(100, 200, {12, 8});
}

TEST(MeshLayoutGroupTests, CreatesPageForFirstAllocation)
{
    auto group = make_group();

    omega::MeshPageAllocation allocation;

    ASSERT_TRUE(group.allocate(10, 20, allocation));

    EXPECT_EQ(group.page_count(), 1);
    EXPECT_EQ(allocation.page_index, 0);
    EXPECT_EQ(allocation.allocation.vertex_count, 10);
    EXPECT_EQ(allocation.allocation.index_count, 20);
}

TEST(MeshLayoutGroupTests, ReusesExistingPage)
{
    auto group = make_group();

    omega::MeshPageAllocation first;
    omega::MeshPageAllocation second;

    ASSERT_TRUE(group.allocate(30, 40, first));
    ASSERT_TRUE(group.allocate(20, 30, second));

    EXPECT_EQ(group.page_count(), 1);
    EXPECT_EQ(first.page_index, second.page_index);
}

TEST(MeshLayoutGroupTests, CreatesNewPageWhenExistingPageIsFull)
{
    auto group = make_group();

    omega::MeshPageAllocation first;
    omega::MeshPageAllocation second;

    ASSERT_TRUE(group.allocate(60, 100, first));
    ASSERT_TRUE(group.allocate(50, 100, second));

    EXPECT_EQ(group.page_count(), 2);
    EXPECT_NE(first.page_index, second.page_index);
}

TEST(MeshLayoutGroupTests, CreatesLargerPageForOversizedMesh)
{
    auto group = make_group();

    omega::MeshPageAllocation allocation;

    ASSERT_TRUE(group.allocate(150, 250, allocation));

    EXPECT_EQ(group.page_count(), 1);
    EXPECT_EQ(allocation.page_index, 0);
    EXPECT_EQ(allocation.allocation.vertex_count, 150);
    EXPECT_EQ(allocation.allocation.index_count, 250);
}

TEST(MeshLayoutGroupTests, SupportsNonIndexedAllocation)
{
    auto group = make_group();

    omega::MeshPageAllocation allocation;

    ASSERT_TRUE(group.allocate(50, 0, allocation));

    EXPECT_EQ(group.page_count(), 1);
    EXPECT_EQ(allocation.allocation.index_count, 0);
}

TEST(MeshLayoutGroupTests, ReleasesAllocationToCorrectPage)
{
    auto group = make_group();

    omega::MeshPageAllocation first;
    omega::MeshPageAllocation second;

    ASSERT_TRUE(group.allocate(60, 100, first));
    ASSERT_TRUE(group.allocate(50, 100, second));

    ASSERT_EQ(group.page_count(), 2);

    EXPECT_TRUE(group.release(second));

    omega::MeshPageAllocation third;
    ASSERT_TRUE(group.allocate(50, 100, third));

    EXPECT_EQ(third.page_index, second.page_index);
    EXPECT_EQ(group.page_count(), 2);
}

TEST(MeshLayoutGroupTests, RejectsInvalidPageIndex)
{
    auto group = make_group();

    omega::MeshPageAllocation allocation;
    allocation.page_index = 42;

    EXPECT_FALSE(group.release(allocation));
}

TEST(MeshLayoutGroupTests, RejectsZeroVertexCount)
{
    auto group = make_group();

    omega::MeshPageAllocation allocation;

    EXPECT_FALSE(group.allocate(0, 10, allocation));
    EXPECT_EQ(group.page_count(), 0);
}

} // namespace