#ifndef OMEGA_MESH_LAYOUT_GROUP_HPP
#define OMEGA_MESH_LAYOUT_GROUP_HPP

#include <omega/mesh/mesh_page.hpp>
#include <omega/mesh/mesh_page_allocation.hpp>

#include <cstdint>

namespace omega
{

class MeshLayoutGroup
{
  public:
    MeshLayoutGroup(uint32_t default_vertex_capacity, uint32_t default_index_capacity,
                    std::vector<uint32_t> binding_strides);

    bool allocate(uint32_t vertex_count, uint32_t index_count, MeshPageAllocation& out_allocation);
    bool release(const MeshPageAllocation& allocation);

    bool write(const MeshPageAllocation& allocation, 
               const std::vector<std::vector<std::byte>>& vertex_data,
               const std::vector<uint32_t>& indices);

    uint32_t page_count() const;

  private:
    uint32_t m_default_vertex_capacity = 0;
    uint32_t m_default_index_capacity = 0;

    std::vector<uint32_t> m_binding_strides;
    std::vector<MeshPage> m_pages;
};

} // namespace omega

#endif // OMEGA_MESH_LAYOUT_GROUP_HPP