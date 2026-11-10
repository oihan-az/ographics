#ifndef OMEGA_MESH_LAYOUT_GROUP_BASE_HPP
#define OMEGA_MESH_LAYOUT_GROUP_BASE_HPP

#include <omega/mesh/mesh_page_allocation.hpp>

#include <cstdint>
#include <vector>
#include <span>

namespace omega
{

template <typename PageType>
class MeshLayoutGroupBase
{
  public:
    MeshLayoutGroupBase(uint32_t default_vertex_capacity, uint32_t default_index_capacity,
                        std::vector<uint32_t> binding_strides);

    bool allocate(uint32_t vertex_count, uint32_t index_count, MeshPageAllocation& out_allocation);
    bool release(const MeshPageAllocation& allocation);

    bool write(const MeshPageAllocation& allocation, 
               const std::vector<std::span<const std::byte>>& vertex_data,
               const std::span<const uint32_t> indices);

    uint32_t page_count() const;
    const PageType* page(uint32_t page_index) const;

protected:
    virtual PageType& create_page(uint32_t vertex_capacity, uint32_t index_capacity, const std::vector<uint32_t>& binding_strides) = 0;

    std::vector<PageType> m_pages;

  private:
    uint32_t m_default_vertex_capacity = 0;
    uint32_t m_default_index_capacity = 0;

    std::vector<uint32_t> m_binding_strides;
};

} // namespace omega

#include <omega/mesh/mesh_layout_group_base.tpp>

#endif // OMEGA_MESH_LAYOUT_GROUP_BASE_HPP