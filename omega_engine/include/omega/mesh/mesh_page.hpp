#ifndef OMEGA_MESH_PAGE_HPP
#define OMEGA_MESH_PAGE_HPP

#include <omega/mesh/mesh_page_base.hpp>

#include <cstdint>
#include <vector>
#include <cstddef>
#include <span>

namespace omega
{

class MeshPage : public MeshPageBase
{
  public:
    MeshPage(uint32_t vertex_capacity, uint32_t index_capacity, const std::vector<uint32_t>& binding_strides);

    bool write(const MeshAllocation& allocation, 
               const std::vector<std::span<const std::byte>>& vertex_data,
               const std::span<const uint32_t> indices);

    const std::vector<std::byte>& vertex_data(uint32_t binding) const;
    const std::vector<uint32_t>& indices() const;

  private:
    std::vector<std::vector<std::byte>> m_vertex_data;
    std::vector<uint32_t> m_indices;
};

} // namespace omega

#endif // OMEGA_MESH_PAGE_HPP