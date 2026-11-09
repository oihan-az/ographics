#ifndef OMEGA_MESH_SYSTEM_HPP
#define OMEGA_MESH_SYSTEM_HPP

#include <omega/mesh/mesh_asset.hpp>
#include <omega/mesh/mesh_data.hpp>
#include <omega/mesh/mesh_handle.hpp>
#include <omega/mesh/vertex_layout_registry.hpp>
#include <omega/mesh/mesh_layout_group.hpp>

namespace omega
{

struct MeshSystemDesc
{
    uint32_t page_vertex_capacity = 65536;
    uint32_t page_index_capacity = 196608;
};

class MeshSystem
{
  public:
    MeshSystem(const MeshSystemDesc& desc = {});

    bool create_mesh(const MeshAsset& asset, MeshHandle& out_handle);
    bool release_mesh(MeshHandle handle);

    MeshData* get(MeshHandle handle);
    const MeshData* get(MeshHandle handle) const;

    uint32_t mesh_count() const;

  private:
    struct MeshSlot
    {
        uint32_t generation = 0;
        bool occupied = false;
        MeshData data;
    };

    MeshSystemDesc m_desc;

    VertexLayoutRegistry m_layout_registry;
    std::vector<MeshLayoutGroup> m_layout_groups;

    std::vector<MeshSlot> m_mesh_slots;
    std::vector<uint32_t> m_free_mesh_indices;

    uint32_t m_mesh_count = 0;
};

} // namespace omega

#endif // OMEGA_MESH_SYSTEM_HPP