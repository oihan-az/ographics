#ifndef OMEGA_MESH_LAYOUT_GROUP_HPP
#define OMEGA_MESH_LAYOUT_GROUP_HPP

#include <omega/mesh/mesh_page.hpp>
#include <omega/mesh/mesh_layout_group_base.hpp>

#include <cstdint>
#include <vector>

namespace omega
{

class MeshLayoutGroup : public MeshLayoutGroupBase<MeshPage>
{
  public:
    MeshLayoutGroup(uint32_t default_vertex_capacity, uint32_t default_index_capacity,
                    std::vector<uint32_t> binding_strides);

  protected:
    MeshPage& create_page(uint32_t vertex_capacity, uint32_t index_capacity,
                          const std::vector<uint32_t>& binding_strides) override;
};

} // namespace omega

#endif // OMEGA_MESH_LAYOUT_GROUP_HPP