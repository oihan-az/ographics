#ifndef OMEGA_GPU_MESH_PAGE_HPP
#define OMEGA_GPU_MESH_PAGE_HPP

#include <omega/mesh/mesh_page_base.hpp>

#include <orendergraph/resource_handle.hpp>
#include <ogfx/buffer.hpp>

#include <cstdint>
#include <vector>
#include <cstddef>
#include <span>

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

class GpuMeshPage : public MeshPageBase
{
  public:
    GpuMeshPage(uint32_t vertex_capacity, uint32_t index_capacity, const std::vector<uint32_t>& binding_strides,
                const ogfx::PhysicalDevice& physical_device, const ogfx::Device& device,
                orendergraph::ResourcePoolRegistry& resources);

    ~GpuMeshPage();

    GpuMeshPage(const GpuMeshPage&) = delete;
    GpuMeshPage& operator=(const GpuMeshPage&) = delete;

    GpuMeshPage(GpuMeshPage&& other);
    GpuMeshPage& operator=(GpuMeshPage&& other) = delete;

    bool write(const MeshAllocation& allocation, 
               const std::vector<std::span<const std::byte>>& vertex_data,
               const std::span<const uint32_t> indices);

    const ogfx::Buffer* vertex_buffer(uint32_t binding_index) const;
    const ogfx::Buffer* index_buffer() const;

  private:
    std::vector<orendergraph::ResourceHandle<ogfx::Buffer>> m_vertex_buffers;
    orendergraph::ResourceHandle<ogfx::Buffer> m_index_buffer;

    const ogfx::PhysicalDevice& m_physical_device;
    const ogfx::Device& m_device;
    orendergraph::ResourcePoolRegistry& m_resources;
};

} // namespace omega

#endif // OMEGA_MESH_PAGE_HPP