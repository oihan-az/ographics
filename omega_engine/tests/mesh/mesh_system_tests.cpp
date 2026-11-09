#include <gtest/gtest.h>

#include <cstring>
#include <vector>

#include <omega/mesh/mesh_system.hpp>

namespace
{

omega::MeshAsset make_mesh_asset(uint32_t vertex_count = 3)
{
    omega::MeshAsset asset;

    asset.vertex_input.bindings.push_back({0, sizeof(float) * 3, ogfx::VertexInputRate::Vertex});

    asset.vertex_input.attributes.push_back({0, 0, ogfx::Format::RGB32_FLOAT, 0});

    asset.vertex_count = vertex_count;

    std::vector<float> vertices(vertex_count * 3, 1.0f);

    std::vector<std::byte> vertex_data(vertices.size() * sizeof(float));

    std::memcpy(vertex_data.data(), vertices.data(), vertex_data.size());

    asset.vertex_data.push_back(std::move(vertex_data));

    if (vertex_count >= 3)
    {
        asset.indices = {0, 1, 2};
    }

    return asset;
}

TEST(MeshSystemTests, CreatesMesh)
{
    omega::MeshSystem system;

    omega::MeshHandle handle;

    ASSERT_TRUE(system.create_mesh(make_mesh_asset(), handle));

    EXPECT_TRUE(handle.is_valid());
    EXPECT_EQ(system.mesh_count(), 1);

    const omega::MeshData* data = system.get(handle);

    ASSERT_NE(data, nullptr);
    EXPECT_EQ(data->vertex_count, 3);
    EXPECT_EQ(data->index_count, 3);
    EXPECT_TRUE(data->is_indexed());
}

TEST(MeshSystemTests, RejectsInvalidAsset)
{
    omega::MeshSystem system;

    omega::MeshAsset asset;

    omega::MeshHandle handle;

    EXPECT_FALSE(system.create_mesh(asset, handle));
    EXPECT_EQ(system.mesh_count(), 0);
}

TEST(MeshSystemTests, CreatesMultipleMeshesWithSameLayout)
{
    omega::MeshSystem system;

    omega::MeshHandle first;
    omega::MeshHandle second;

    ASSERT_TRUE(system.create_mesh(make_mesh_asset(), first));
    ASSERT_TRUE(system.create_mesh(make_mesh_asset(6), second));

    const omega::MeshData* first_data = system.get(first);
    const omega::MeshData* second_data = system.get(second);

    ASSERT_NE(first_data, nullptr);
    ASSERT_NE(second_data, nullptr);

    EXPECT_EQ(first_data->layout, second_data->layout);
    EXPECT_EQ(system.mesh_count(), 2);
}

TEST(MeshSystemTests, ReusesMeshSlotWithNewGeneration)
{
    omega::MeshSystem system;

    omega::MeshHandle first;
    ASSERT_TRUE(system.create_mesh(make_mesh_asset(), first));

    ASSERT_TRUE(system.release_mesh(first));
    EXPECT_EQ(system.get(first), nullptr);

    omega::MeshHandle second;
    ASSERT_TRUE(system.create_mesh(make_mesh_asset(), second));

    EXPECT_EQ(first.index, second.index);
    EXPECT_NE(first.generation, second.generation);
    EXPECT_NE(system.get(second), nullptr);
}

TEST(MeshSystemTests, RejectsDoubleRelease)
{
    omega::MeshSystem system;

    omega::MeshHandle handle;
    ASSERT_TRUE(system.create_mesh(make_mesh_asset(), handle));

    EXPECT_TRUE(system.release_mesh(handle));
    EXPECT_FALSE(system.release_mesh(handle));
    EXPECT_EQ(system.mesh_count(), 0);
}

TEST(MeshSystemTests, RejectsStaleHandle)
{
    omega::MeshSystem system;

    omega::MeshHandle first;
    ASSERT_TRUE(system.create_mesh(make_mesh_asset(), first));

    ASSERT_TRUE(system.release_mesh(first));

    omega::MeshHandle second;
    ASSERT_TRUE(system.create_mesh(make_mesh_asset(), second));

    EXPECT_EQ(system.get(first), nullptr);
    EXPECT_NE(system.get(second), nullptr);
}

TEST(MeshSystemTests, SupportsNonIndexedMesh)
{
    omega::MeshSystem system;

    omega::MeshAsset asset = make_mesh_asset();
    asset.indices.clear();

    omega::MeshHandle handle;

    ASSERT_TRUE(system.create_mesh(asset, handle));

    const omega::MeshData* data = system.get(handle);

    ASSERT_NE(data, nullptr);
    EXPECT_EQ(data->index_count, 0);
    EXPECT_FALSE(data->is_indexed());
}

TEST(MeshSystemTests, RejectsInvalidHandle)
{
    omega::MeshSystem system;

    EXPECT_EQ(system.get({}), nullptr);
    EXPECT_FALSE(system.release_mesh({}));
}

} // namespace