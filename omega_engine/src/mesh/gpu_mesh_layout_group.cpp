#include <omega/mesh/gpu_mesh_layout_group.hpp>

namespace omega
{

GpuMeshLayoutGroup::GpuMeshLayoutGroup(const ogfx::PhysicalDevice& physical_device, const ogfx::Device& device,
                                       orendergraph::ResourcePoolRegistry& resource_registry,
                                       uint32_t default_vertex_capacity, uint32_t default_index_capacity,
                                       std::vector<uint32_t> binding_strides)
    : MeshLayoutGroupBase<GpuMeshPage>(default_vertex_capacity, default_index_capacity, binding_strides),
      m_physical_device(physical_device), m_device(device), m_resources(resource_registry)
{
}

GpuMeshPage& GpuMeshLayoutGroup::create_page(uint32_t vertex_capacity, uint32_t index_capacity,
                                            const std::vector<uint32_t>& binding_strides)
{
    return m_pages.emplace_back(vertex_capacity, index_capacity, binding_strides, 
                                m_physical_device, m_device, m_resources);
}

} // namespace omega