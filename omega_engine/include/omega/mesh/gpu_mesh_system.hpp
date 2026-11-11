#ifndef OMEGA_GPU_MESH_SYSTEM_HPP
#define OMEGA_GPU_MESH_SYSTEM_HPP

#include <omega/mesh/mesh_data.hpp>
#include <omega/mesh/mesh_handle.hpp>
#include <omega/mesh/gpu_mesh_page.hpp>
#include <omega/mesh/gpu_mesh_layout_group.hpp>
#include <omega/mesh/vertex_layout_registry.hpp>
#include <omega/mesh/mesh_system_desc.hpp>

#include <ogfx/index_type.hpp>

namespace ogfx
{

class PhysicalDevice;
class Device;

} // namespace ogfx

namespace orendergraph
{

class ResourcePoolRegistry;

} // namespace ogfx

namespace omega
{

class MeshSystem;
class EventSystem;

struct GpuMeshVertexBinding
{
    const ogfx::Buffer* buffer = nullptr;
    uint32_t binding = 0;
    uint64_t offset = 0;
};

struct GpuMeshDrawData
{
    std::vector<GpuMeshVertexBinding> vertex_bindings;

    const ogfx::Buffer* index_buffer = nullptr;
    uint64_t index_offset = 0;
    ogfx::IndexType index_type = ogfx::IndexType::UInt32;

    uint32_t vertex_count = 0;
    uint32_t index_count = 0;

    bool is_indexed() const { return index_count > 0; }
    bool is_valid() const { return !vertex_bindings.empty() && (!is_indexed() || index_buffer != nullptr); }
};

class GpuMeshSystem
{
  public:
    GpuMeshSystem(const ogfx::PhysicalDevice& physical_device, const ogfx::Device& device,
                  orendergraph::ResourcePoolRegistry& resource_registry, 
                  const MeshSystem& mesh_system, EventSystem& events,
                  const MeshSystemDesc& desc = {});

    ~GpuMeshSystem();

    bool make_resident(MeshHandle handle);
    bool make_non_resident(MeshHandle handle);

    void synchronize();

    bool is_resident(MeshHandle handle) const;
    bool is_pending(MeshHandle handle) const;

    const MeshData* get(MeshHandle handle) const;

    uint32_t resident_mesh_count() const;
    uint32_t layout_group_count() const;

    GpuMeshDrawData get_draw_data(MeshHandle mesh) const;

  private:
    struct MeshSlot
    {
        uint32_t generation = 0;
        bool occupied = false;
        bool requested_resident = false;
        bool pending = false;
        MeshData data{};
    };

    const GpuMeshLayoutGroup* layout_group(VertexLayoutId layout) const;
    const VertexLayout* vertex_layout(VertexLayoutId layout) const;

    bool upload_mesh(MeshHandle handle);
    bool release_gpu_mesh(MeshHandle handle);

    bool is_valid_slot(MeshHandle handle) const;

    void process_pending_requests();

    void enqueue_pending(MeshHandle handle);

    const MeshSystem& m_mesh_system;
    EventSystem& m_events;

    const ogfx::PhysicalDevice& m_physical_device;
    const ogfx::Device& m_device;
    orendergraph::ResourcePoolRegistry& m_resources;

    MeshSystemDesc m_desc;

    VertexLayoutRegistry m_layout_registry;
    std::vector<GpuMeshLayoutGroup> m_layout_groups;

    std::vector<MeshSlot> m_mesh_slots;

    std::vector<MeshHandle> m_pending_meshes;

    uint32_t m_resident_mesh_count = 0;
};

} // namespace omega

#endif // OMEGA_GPU_MESH_SYSTEM_HPP