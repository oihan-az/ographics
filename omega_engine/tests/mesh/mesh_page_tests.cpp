#include <gtest/gtest.h>

#include <omega/mesh/mesh_page.hpp>

#include <cstdint>
#include <vector>

namespace
{

omega::MeshPage make_page()
{
    return omega::MeshPage(100, 200, {12, 8});
}

} // namespace

TEST(MeshPageTests, AllocatesVertexAndIndexRanges)
{
    auto page = make_page();

    omega::MeshAllocation allocation;

    ASSERT_TRUE(page.allocate(10, 30, allocation));

    EXPECT_EQ(allocation.vertex_count, 10);
    EXPECT_EQ(allocation.index_count, 30);
    ASSERT_EQ(allocation.vertex_offsets.size(), 2);
    EXPECT_EQ(allocation.vertex_offsets[0], 0);
    EXPECT_EQ(allocation.vertex_offsets[1], 0);
    EXPECT_EQ(allocation.index_offset, 0);
}

TEST(MeshPageTests, AllocatesNonIndexedMesh)
{
    auto page = make_page();

    omega::MeshAllocation allocation;

    ASSERT_TRUE(page.allocate(10, 0, allocation));

    EXPECT_EQ(allocation.vertex_count, 10);
    EXPECT_EQ(allocation.index_count, 0);
    EXPECT_EQ(allocation.index_offset, 0);
}

TEST(MeshPageTests, AllocatesMultipleMeshes)
{
    auto page = make_page();

    omega::MeshAllocation first;
    omega::MeshAllocation second;

    ASSERT_TRUE(page.allocate(10, 20, first));
    ASSERT_TRUE(page.allocate(15, 30, second));

    EXPECT_EQ(first.vertex_offsets[0], 0);
    EXPECT_EQ(second.vertex_offsets[0], 10);

    EXPECT_EQ(first.index_offset, 0);
    EXPECT_EQ(second.index_offset, 20);
}

TEST(MeshPageTests, ReleasesAllocation)
{
    auto page = make_page();

    omega::MeshAllocation allocation;

    ASSERT_TRUE(page.allocate(20, 40, allocation));
    ASSERT_TRUE(page.release(allocation));

    omega::MeshAllocation reused;

    ASSERT_TRUE(page.allocate(20, 40, reused));

    EXPECT_EQ(reused.vertex_offsets[0], 0);
    EXPECT_EQ(reused.index_offset, 0);
}

TEST(MeshPageTests, RejectsAllocationExceedingVertexCapacity)
{
    auto page = make_page();

    omega::MeshAllocation allocation;

    EXPECT_FALSE(page.allocate(101, 10, allocation));
}

TEST(MeshPageTests, RejectsAllocationExceedingIndexCapacity)
{
    auto page = make_page();

    omega::MeshAllocation allocation;

    EXPECT_FALSE(page.allocate(10, 201, allocation));
}

TEST(MeshPageTests, RollsBackWhenBindingAllocationFails)
{
    omega::MeshPage page(100, 100, {12, 8});

    omega::MeshAllocation first;

    ASSERT_TRUE(page.allocate(60, 0, first));

    // This allocation cannot fit in the first binding's remaining space
    omega::MeshAllocation second;

    EXPECT_FALSE(page.allocate(50, 0, second));

    // The second binding must not have leaked its partially allocated range
    omega::MeshAllocation third;

    ASSERT_TRUE(page.allocate(40, 0, third));
    EXPECT_EQ(third.vertex_offsets[1], 60);
}

TEST(MeshPageTests, RejectsInvalidRelease)
{
    auto page = make_page();

    omega::MeshAllocation invalid;
    invalid.vertex_count = 10;
    invalid.vertex_offsets = {0};

    EXPECT_FALSE(page.release(invalid));
}

TEST(MeshPageTests, ReportsCapacities)
{
    auto page = make_page();

    EXPECT_EQ(page.vertex_capacity(), 100);
    EXPECT_EQ(page.index_capacity(), 200);
}

TEST(MeshPageTests, RejectsDoubleRelease)
{
    auto page = make_page();

    omega::MeshAllocation allocation;
    ASSERT_TRUE(page.allocate(10, 20, allocation));

    EXPECT_TRUE(page.release(allocation));
    EXPECT_FALSE(page.release(allocation));
}

TEST(MeshPageTests, RejectsModifiedAllocation)
{
    auto page = make_page();

    omega::MeshAllocation allocation;
    ASSERT_TRUE(page.allocate(10, 20, allocation));

    omega::MeshAllocation modified = allocation;
    modified.vertex_count = 5;

    EXPECT_FALSE(page.release(modified));
    EXPECT_TRUE(page.release(allocation));
}

TEST(MeshPageTests, RejectsStaleAllocationAfterReuse)
{
    auto page = make_page();

    omega::MeshAllocation old_allocation;
    ASSERT_TRUE(page.allocate(10, 20, old_allocation));
    ASSERT_TRUE(page.release(old_allocation));

    omega::MeshAllocation new_allocation;
    ASSERT_TRUE(page.allocate(10, 20, new_allocation));

    EXPECT_EQ(old_allocation.vertex_offsets, new_allocation.vertex_offsets);
    EXPECT_NE(old_allocation.allocation_id, new_allocation.allocation_id);

    EXPECT_FALSE(page.release(old_allocation));
    EXPECT_TRUE(page.release(new_allocation));
}