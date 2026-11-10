#ifndef OMEGA_MESH_GPU_LAYOUT_GROUP_HPP
#define OMEGA_MESH_GPU_LAYOUT_GROUP_HPP

#include <omega/mesh/gpu_mesh_page.hpp>
#include <omega/mesh/mesh_layout_group_base.hpp>

#include <cstdint>
#include <vector>

namespace ogfx
{
class PhysicalDevice;
class Device;
} // namespace ogfx

namespace orendergraph
{
class ResourcePoolRegistry;
} // namespace orendergraph

namespace omega
{

class GpuMeshLayoutGroup : public MeshLayoutGroupBase<GpuMeshPage>
{
  public:
    GpuMeshLayoutGroup(const ogfx::PhysicalDevice& physical_device, const ogfx::Device& device,
                       orendergraph::ResourcePoolRegistry& resource_registry,
                       uint32_t default_vertex_capacity, uint32_t default_index_capacity,
                       std::vector<uint32_t> binding_strides);

  protected:
    GpuMeshPage& create_page(uint32_t vertex_capacity, uint32_t index_capacity,
                             const std::vector<uint32_t>& binding_strides) override;

  private:
    orendergraph::ResourcePoolRegistry& m_resources;
    const ogfx::PhysicalDevice& m_physical_device;
    const ogfx::Device& m_device;
};

} // namespace omega

#endif // OMEGA_MESH_GPU_LAYOUT_GROUP_HPP