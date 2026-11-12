#ifndef OMEGA_IMPORTED_MODEL_HPP
#define OMEGA_IMPORTED_MODEL_HPP

#include <omega/mesh/mesh_asset.hpp>
#include <omega/transform/transform.hpp>

#include <cstdint>
#include <limits>
#include <string>
#include <vector>

namespace omega
{

struct ImportedPrimitive
{
    MeshAsset mesh;
};

struct ImportedMesh
{
    std::string name;

    std::vector<ImportedPrimitive> primitives;
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
    std::vector<ImportedNode> nodes;

    std::vector<uint32_t> root_nodes;
};

} // namespace omega

#endif // OMEGA_IMPORTED_MODEL_HPP