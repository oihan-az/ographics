#ifndef OMEGA_IMPORTED_MODEL_HPP
#define OMEGA_IMPORTED_MODEL_HPP

#include <omega/mesh/mesh_asset.hpp>
#include <omega/texture/texture_asset.hpp>
#include <omega/transform/transform.hpp>

#include <cstdint>
#include <limits>
#include <string>
#include <vector>

namespace omega
{

inline constexpr uint32_t invalid_imported_index = std::numeric_limits<uint32_t>::max();

struct ImportedPrimitive
{
    MeshAsset mesh;
    uint32_t material = invalid_imported_index;
};

struct ImportedMesh
{
    std::string name;

    std::vector<ImportedPrimitive> primitives;
};

struct ImportedTexture
{
    TextureAsset texture;
};

struct ImportedPbrOpaqueMaterial
{
    Vector4 base_color_factor{1.0f};
    float metallic_factor = 1.0f;
    float roughness_factor = 1.0f;

    Vector3 emissive_factor{0.0f};
    float normal_scale = 1.0f;
    float occlusion_strength = 1.0f;

    uint32_t base_color_texture = invalid_imported_index;
    uint32_t metallic_roughness_texture = invalid_imported_index;
    uint32_t normal_texture = invalid_imported_index;
    uint32_t occlusion_texture = invalid_imported_index;
    uint32_t emissive_texture = invalid_imported_index;
};

struct ImportedNode
{
    std::string name;

    Transform local_transform;

    std::vector<uint32_t> meshes;

    std::vector<uint32_t> children;
};

struct ImportedModel
{
    std::string name;

    std::vector<ImportedMesh> meshes;
    std::vector<ImportedTexture> textures;
    std::vector<ImportedPbrOpaqueMaterial> materials;
    std::vector<ImportedNode> nodes;

    std::vector<uint32_t> root_nodes;
};

} // namespace omega

#endif // OMEGA_IMPORTED_MODEL_HPP