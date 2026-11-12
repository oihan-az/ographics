#include <omega/mesh/gpu_mesh_page.hpp>

#include <orendergraph/resource_pool_registry.hpp>
#include <orendergraph/resource_traits/buffer.hpp>

#include <utility>
#include <algorithm>
#include <stdexcept>

namespace omega
{

GpuMeshPage::GpuMeshPage(uint32_t vertex_capacity, uint32_t index_capacity, const std::vector<uint32_t>& binding_strides,
                      const ogfx::PhysicalDevice& physical_device, const ogfx::Device& device,
                      orendergraph::ResourcePoolRegistry& resources)
    : MeshPageBase(vertex_capacity, index_capacity, binding_strides),
      m_physical_device(physical_device), m_device(device), m_resources(resources)
{
    m_vertex_buffers.reserve(binding_strides.size());

    for (uint32_t stride : binding_strides)
    {
        ogfx::BufferDesc buffer_desc;
        buffer_desc.size = static_cast<uint64_t>(vertex_capacity) * stride;
        buffer_desc.usage = ogfx::BufferUsage::Vertex | ogfx::BufferUsage::Storage;
        buffer_desc.memory_properties = ogfx::MemoryProperty::HostVisible | ogfx::MemoryProperty::HostCoherent;
        auto handle = m_resources.create_resource<ogfx::Buffer>(physical_device, device, buffer_desc);

        if (!handle.is_valid())
            throw std::runtime_error("Failed to create GPU vertex buffer");

        m_vertex_buffers.push_back(handle);
    }

    ogfx::BufferDesc index_desc;
    index_desc.size = static_cast<uint64_t>(index_capacity) * sizeof(uint32_t);
    index_desc.usage = ogfx::BufferUsage::Index | ogfx::BufferUsage::Storage;
    index_desc.memory_properties = ogfx::MemoryProperty::HostVisible | ogfx::MemoryProperty::HostCoherent;
    m_index_buffer = m_resources.create_resource<ogfx::Buffer>(physical_device, device, index_desc);

    if (!m_index_buffer.is_valid())
        throw std::runtime_error("Failed to create GPU index buffer");
}

GpuMeshPage::GpuMeshPage(GpuMeshPage&& other)
    : MeshPageBase(std::move(other)), m_vertex_buffers(std::move(other.m_vertex_buffers)),
      m_index_buffer(std::exchange(other.m_index_buffer, orendergraph::ResourceHandle<ogfx::Buffer>{})),
      m_physical_device(other.m_physical_device), m_device(other.m_device), m_resources(other.m_resources)
{
    other.m_vertex_buffers.clear();
}

GpuMeshPage::~GpuMeshPage()
{
    for (auto handle : m_vertex_buffers)
        m_resources.release_resource(handle);

    if (m_index_buffer.is_valid())
        m_resources.release_resource(m_index_buffer);
}

bool GpuMeshPage::write(const MeshAllocation& allocation, 
                        const std::vector<std::span<const std::byte>>& vertex_data,
                        const std::span<const uint32_t> indices)
{
    if (!is_active(allocation))
        return false;

    uint32_t binding_count = this->binding_count();

    if (vertex_data.size() != binding_count || allocation.vertex_offsets.size() != binding_count ||
        indices.size() != allocation.index_count)
    {
        return false;
    }

    for (size_t i = 0; i < vertex_data.size(); ++i)
    {
        const uint64_t expected_size = static_cast<uint64_t>(allocation.vertex_count) * binding_stride(i);

        if (vertex_data[i].size() != expected_size)
            return false;

        auto* buffer = m_resources.get_resource(m_vertex_buffers[i]);
        if (!buffer)
            return false;

        ogfx::BufferWriteInfo write_info;
        write_info.data = vertex_data[i].data();
        write_info.size = expected_size;
        write_info.offset = static_cast<uint64_t>(allocation.vertex_offsets[i]) * binding_stride(i);

        buffer->write(write_info);
    }

    if (!indices.empty())
    {
        auto* buffer = m_resources.get_resource(m_index_buffer);
        if (!buffer)
            return false;

        ogfx::BufferWriteInfo write_info;
        write_info.data = indices.data();
        write_info.size = static_cast<uint64_t>(indices.size()) * sizeof(uint32_t);
        write_info.offset = static_cast<uint64_t>(allocation.index_offset) * sizeof(uint32_t);
        buffer->write(write_info);
    }

    return true;
}

const ogfx::Buffer* GpuMeshPage::vertex_buffer(uint32_t binding_index) const
{
    if (binding_index >= m_vertex_buffers.size())
        return nullptr;

    return m_resources.get_resource(m_vertex_buffers[binding_index]);
}

const ogfx::Buffer* GpuMeshPage::index_buffer() const
{
    return m_resources.get_resource(m_index_buffer);
}

} // namespace omega