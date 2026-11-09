#include <gtest/gtest.h>

#include <omega/mesh/mesh_asset.hpp>

namespace
{

omega::MeshAsset make_valid_mesh_asset()
{
    omega::MeshAsset asset;

    asset.vertex_input.bindings.push_back({0, 12, ogfx::VertexInputRate::Vertex});

    asset.vertex_input.attributes.push_back({0, 0, ogfx::Format::RGB32_FLOAT, 0});

    asset.vertex_count = 3;
    asset.vertex_data.emplace_back(36, std::byte{0});

    return asset;
}

} // namespace

TEST(MeshAssetTests, ValidIndexedMesh)
{
    auto asset = make_valid_mesh_asset();
    asset.indices = {0, 1, 2};

    EXPECT_TRUE(omega::validate_mesh_asset(asset));
}

TEST(MeshAssetTests, ValidNonIndexedMesh)
{
    auto asset = make_valid_mesh_asset();

    EXPECT_TRUE(asset.indices.empty());
    EXPECT_TRUE(omega::validate_mesh_asset(asset));
}

TEST(MeshAssetTests, RejectsZeroVertexCount)
{
    auto asset = make_valid_mesh_asset();
    asset.vertex_count = 0;
    asset.vertex_data[0].clear();

    EXPECT_FALSE(omega::validate_mesh_asset(asset));
}

TEST(MeshAssetTests, RejectsMismatchedVertexStreamCount)
{
    auto asset = make_valid_mesh_asset();
    asset.vertex_data.clear();

    EXPECT_FALSE(omega::validate_mesh_asset(asset));
}

TEST(MeshAssetTests, RejectsIncorrectVertexStreamSize)
{
    auto asset = make_valid_mesh_asset();
    asset.vertex_data[0].resize(24);

    EXPECT_FALSE(omega::validate_mesh_asset(asset));
}

TEST(MeshAssetTests, RejectsZeroBindingStride)
{
    auto asset = make_valid_mesh_asset();
    asset.vertex_input.bindings[0].stride = 0;

    EXPECT_FALSE(omega::validate_mesh_asset(asset));
}

TEST(MeshAssetTests, RejectsInstanceRateBinding)
{
    auto asset = make_valid_mesh_asset();
    asset.vertex_input.bindings[0].input_rate = ogfx::VertexInputRate::Instance;

    EXPECT_FALSE(omega::validate_mesh_asset(asset));
}

TEST(MeshAssetTests, RejectsUndefinedAttributeFormat)
{
    auto asset = make_valid_mesh_asset();
    asset.vertex_input.attributes[0].format = ogfx::Format::Undefined;

    EXPECT_FALSE(omega::validate_mesh_asset(asset));
}

TEST(MeshAssetTests, RejectsDepthAttributeFormat)
{
    auto asset = make_valid_mesh_asset();
    asset.vertex_input.attributes[0].format = ogfx::Format::D32_FLOAT;

    EXPECT_FALSE(omega::validate_mesh_asset(asset));
}

TEST(MeshAssetTests, RejectsAttributeWithUnknownBinding)
{
    auto asset = make_valid_mesh_asset();
    asset.vertex_input.attributes[0].binding = 1;

    EXPECT_FALSE(omega::validate_mesh_asset(asset));
}

TEST(MeshAssetTests, RejectsAttributeOutsideBindingStride)
{
    auto asset = make_valid_mesh_asset();
    asset.vertex_input.attributes[0].offset = 4;

    EXPECT_FALSE(omega::validate_mesh_asset(asset));
}

TEST(MeshAssetTests, RejectsAttributeOffsetBeyondBindingStride)
{
    auto asset = make_valid_mesh_asset();
    asset.vertex_input.attributes[0].offset = 13;

    EXPECT_FALSE(omega::validate_mesh_asset(asset));
}

TEST(MeshAssetTests, RejectsOutOfRangeIndex)
{
    auto asset = make_valid_mesh_asset();
    asset.indices = {0, 1, 3};

    EXPECT_FALSE(omega::validate_mesh_asset(asset));
}

TEST(MeshAssetTests, RejectsDuplicateBindingIndices)
{
    auto asset = make_valid_mesh_asset();

    asset.vertex_input.bindings.push_back({0, 12, ogfx::VertexInputRate::Vertex});

    asset.vertex_data.emplace_back(36, std::byte{0});

    EXPECT_FALSE(omega::validate_mesh_asset(asset));
}