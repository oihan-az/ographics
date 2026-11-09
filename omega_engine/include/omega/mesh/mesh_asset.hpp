#ifndef OMEGA_MESH_ASSET_HPP
#define OMEGA_MESH_ASSET_HPP

#include <ogfx/vertex_input.hpp>

#include <cstdint>
#include <vector>
#include <cstddef>

namespace omega
{

struct MeshAsset
{
    ogfx::VertexInputStateDesc vertex_input;

    std::vector<std::vector<std::byte>> vertex_data;

    std::vector<uint32_t> indices;

    uint32_t vertex_count = 0;
};

bool validate_mesh_asset(const MeshAsset& asset);

} // namespace omega

#endif // OMEGA_MESH_ASSET_HPP