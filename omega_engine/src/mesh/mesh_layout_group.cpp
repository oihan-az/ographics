#include <omega/mesh/mesh_layout_group.hpp>

namespace omega
{

MeshLayoutGroup::MeshLayoutGroup(uint32_t default_vertex_capacity, uint32_t default_index_capacity,
                                 std::vector<uint32_t> binding_strides)
    : MeshLayoutGroupBase<MeshPage>(default_vertex_capacity, default_index_capacity, binding_strides)
{
}

MeshPage& MeshLayoutGroup::create_page(uint32_t vertex_capacity, uint32_t index_capacity,
                                       const std::vector<uint32_t>& binding_strides)
{
    return m_pages.emplace_back(vertex_capacity, index_capacity, binding_strides);
}

} // namespace omega