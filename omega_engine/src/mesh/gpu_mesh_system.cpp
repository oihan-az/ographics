#include <omega/mesh/gpu_mesh_system.hpp>
#include <omega/mesh/mesh_system.hpp>
#include <omega/mesh/mesh_event.hpp>

#include <omega/core/event/event_system.hpp>

#include <span>

namespace omega
{
GpuMeshSystem::GpuMeshSystem(const ogfx::PhysicalDevice& physical_device, const ogfx::Device& device,
                             orendergraph::ResourcePoolRegistry& resource_registry, const MeshSystem& mesh_system,
                             EventSystem& events, const MeshSystemDesc& desc)
    : m_mesh_system(mesh_system), m_events(events), m_physical_device(physical_device), m_device(device),
      m_resources(resource_registry), m_desc(desc)
{
}

GpuMeshSystem::~GpuMeshSystem()
{
    // Release all allocations while the resource registry is still alive.
    for (uint32_t i = 0; i < m_mesh_slots.size(); ++i)
    {
        auto& slot = m_mesh_slots[i];

        if (!slot.occupied || !slot.data.handle.is_valid())
            continue;

        release_gpu_mesh(slot.data.handle);
    }

    m_layout_groups.clear();
}

bool GpuMeshSystem::make_resident(MeshHandle handle)
{
    if (m_mesh_system.get(handle) == nullptr)
        return false;

    if (handle.index >= m_mesh_slots.size())
        m_mesh_slots.resize(static_cast<size_t>(handle.index) + 1);

    MeshSlot& slot = m_mesh_slots[handle.index];

    // The CPU slot may have been recycled before we synchronized.
    // Release the old generation's GPU allocation before using this slot.
    if (slot.occupied && slot.generation != handle.generation)
    {
        if (slot.data.handle.is_valid())
            release_gpu_mesh(slot.data.handle);

        slot = {};
    }

    if (!slot.occupied)
    {
        slot.generation = handle.generation;
        slot.occupied = true;
    }

    if (slot.data.handle == handle && slot.data.layout.is_valid())
    {
        slot.requested_resident = true;
        return true;
    }

    slot.requested_resident = true;
    enqueue_pending(handle);

    return true;
}

bool GpuMeshSystem::make_non_resident(MeshHandle handle)
{
    if (!is_valid_slot(handle))
        return false;

    MeshSlot& slot = m_mesh_slots[handle.index];

    slot.requested_resident = false;
    enqueue_pending(handle);

    return true;
}

void GpuMeshSystem::synchronize()
{
    process_pending_requests();
}

bool GpuMeshSystem::is_resident(MeshHandle handle) const
{
    if (!is_valid_slot(handle))
        return false;

    const MeshData& data = m_mesh_slots[handle.index].data;
    return data.handle == handle && data.layout.is_valid();
}

bool GpuMeshSystem::is_pending(MeshHandle handle) const
{
    if (!is_valid_slot(handle))
        return false;

    return m_mesh_slots[handle.index].pending;
}

const MeshData* GpuMeshSystem::get(MeshHandle handle) const
{
    if (!is_resident(handle))
        return nullptr;

    return &m_mesh_slots[handle.index].data;
}

uint32_t GpuMeshSystem::resident_mesh_count() const
{
    return m_resident_mesh_count;
}

uint32_t GpuMeshSystem::layout_group_count() const
{
    return static_cast<uint32_t>(m_layout_groups.size());
}

const GpuMeshLayoutGroup* GpuMeshSystem::layout_group(VertexLayoutId layout) const
{
    if (!layout.is_valid() || layout.value >= m_layout_groups.size())
        return nullptr;

    return &m_layout_groups[layout.value];
}

const VertexLayout* GpuMeshSystem::vertex_layout(VertexLayoutId layout) const
{
    return m_layout_registry.get(layout);
}

GpuMeshDrawData GpuMeshSystem::get_draw_data(MeshHandle handle) const
{
    GpuMeshDrawData result{};

    if (!is_resident(handle))
        return result;

    const MeshData* mesh = get(handle);
    if (!mesh)
        return result;

    const GpuMeshLayoutGroup* group = layout_group(mesh->layout);
    if (!group)
        return result;

    const GpuMeshPage* page = group->page(mesh->allocation.page_index);
    if (!page)
        return result;

    const VertexLayout* layout = vertex_layout(mesh->layout);
    if (!layout)
        return result;

    const auto& layout_bindings = layout->desc.bindings;
    const auto& vertex_offsets = mesh->allocation.allocation.vertex_offsets;

    if (layout_bindings.size() != vertex_offsets.size())
        return result;

    result.vertex_bindings.reserve(layout_bindings.size());

    for (uint32_t i = 0; i < layout_bindings.size(); ++i)
    {
        const ogfx::Buffer* buffer = page->vertex_buffer(i);
        if (!buffer)
            return {};

        GpuMeshVertexBinding binding{};
        binding.buffer = buffer;
        binding.binding = layout_bindings[i].binding;
        binding.offset = static_cast<uint64_t>(vertex_offsets[i]) * layout_bindings[i].stride;

        result.vertex_bindings.push_back(binding);
    }

    result.vertex_count = mesh->vertex_count;
    result.index_count = mesh->index_count;

    if (mesh->is_indexed())
    {
        result.index_buffer = page->index_buffer();
        if (!result.index_buffer)
            return {};

        result.index_offset = static_cast<uint64_t>(mesh->allocation.allocation.index_offset) * sizeof(uint32_t);
    }

    return result;
}

bool GpuMeshSystem::upload_mesh(MeshHandle handle)
{
    const MeshData* cpu_mesh = m_mesh_system.get(handle);
    if (cpu_mesh == nullptr)
        return false;

    const VertexLayout* layout = m_mesh_system.vertex_layout(cpu_mesh->layout);
    const MeshLayoutGroup* cpu_group = m_mesh_system.layout_group(cpu_mesh->layout);
    if (layout == nullptr || cpu_group == nullptr)
        return false;

    const MeshPage* cpu_page = cpu_group->page(cpu_mesh->allocation.page_index);
    if (cpu_page == nullptr)
        return false;

    const MeshAllocation& cpu_allocation = cpu_mesh->allocation.allocation;
    const size_t binding_count = layout->desc.bindings.size();
    if (cpu_allocation.vertex_offsets.size() != binding_count)
        return false;

    // Reference  only this mesh's vertex ranges from the CPU page
    std::vector<std::span<const std::byte>> vertex_data;
    vertex_data.reserve(binding_count);
    for (size_t i = 0; i < binding_count; ++i)
    {
        const uint32_t stride = layout->desc.bindings[i].stride;

        if (stride == 0)
            return false;

        const uint64_t source_offset = static_cast<uint64_t>(cpu_allocation.vertex_offsets[i]) * stride;
        const uint64_t source_size = static_cast<uint64_t>(cpu_mesh->vertex_count) * stride;
        const auto& source = cpu_page->vertex_data(static_cast<uint32_t>(i));
        if (source_offset > source.size() || source_size > source.size() - source_offset)
        {
            return false;
        }

        vertex_data.emplace_back(source.data() + source_offset, static_cast<size_t>(source_size));
    }

    // Reference this mesh's index range
    std::span<const uint32_t> indices;
    if (cpu_mesh->index_count > 0)
    {
        const auto& source = cpu_page->indices();

        const uint64_t source_offset = cpu_allocation.index_offset;
        const uint64_t source_size = cpu_mesh->index_count;
        if (source_offset > source.size() || source_size > source.size() - source_offset)
        {
            return false;
        }

        indices = std::span<const uint32_t>(source).subspan(static_cast<size_t>(source_offset),
                                                            static_cast<size_t>(source_size));
    }

   // Register or retrieve the vertex layout
    const VertexLayoutId gpu_layout_id = m_layout_registry.register_layout(*layout);
    if (!gpu_layout_id.is_valid())
    {
        return false;
    }

    // Create the corresponding layout group if necessary
    while (m_layout_groups.size() <= gpu_layout_id.value)
    {
        const uint32_t group_index = static_cast<uint32_t>(m_layout_groups.size());

        const VertexLayoutId id{group_index};
        const VertexLayout* registered_layout = m_layout_registry.get(id);
        if (registered_layout == nullptr)
        {
            return false;
        }

        std::vector<uint32_t> binding_strides;
        binding_strides.reserve(registered_layout->desc.bindings.size());
        for (const auto& binding : registered_layout->desc.bindings)
        {
            binding_strides.push_back(binding.stride);
        }

        m_layout_groups.emplace_back(m_physical_device, m_device, m_resources,
                                     m_desc.page_vertex_capacity, m_desc.page_index_capacity,
                                     std::move(binding_strides));
    }

    GpuMeshLayoutGroup& gpu_group = m_layout_groups[gpu_layout_id.value];

    MeshPageAllocation gpu_allocation;
    if (!gpu_group.allocate(cpu_mesh->vertex_count, cpu_mesh->index_count, gpu_allocation))
    {
        return false;
    }

    if (!gpu_group.write(gpu_allocation, vertex_data, indices))
    {
        gpu_group.release(gpu_allocation);
        return false;
    }

    // Store GPU-side allocation information independently from the CPU one
    MeshData gpu_data = *cpu_mesh;
    gpu_data.layout = gpu_layout_id;
    gpu_data.allocation = std::move(gpu_allocation);

    MeshSlot& slot = m_mesh_slots[handle.index];

    const bool was_resident = slot.data.handle == handle && slot.data.layout.is_valid();
    if (was_resident)
    {
        if (!release_gpu_mesh(handle))
        {
            gpu_group.release(gpu_allocation);
            return false;
        }
    }

    slot.data = std::move(gpu_data);

    if (!was_resident)
        ++m_resident_mesh_count;

    return true;
}

bool GpuMeshSystem::release_gpu_mesh(MeshHandle handle)
{
    if (handle.index >= m_mesh_slots.size())
        return false;

    MeshSlot& slot = m_mesh_slots[handle.index];

    // Always compare the complete handle. A delayed event for an older
    // generation must never release a newer mesh's allocation.
    if (!slot.occupied || slot.generation != handle.generation)
        return false;

    if (slot.data.handle != handle || !slot.data.layout.is_valid())
        return false;

    const VertexLayoutId layout_id = slot.data.layout;

    if (layout_id.value >= m_layout_groups.size())
    {
        return false;
    }

    const bool released = m_layout_groups[layout_id.value].release(slot.data.allocation);

    if (!released)
        return false;

    slot.data = {};

    if (m_resident_mesh_count > 0)
        --m_resident_mesh_count;

    return true;
}

bool GpuMeshSystem::is_valid_slot(MeshHandle handle) const
{
    if (!handle.is_valid() || handle.index >= m_mesh_slots.size())
        return false;

    const MeshSlot& slot = m_mesh_slots[handle.index];

    return slot.occupied && slot.generation == handle.generation;
}

void GpuMeshSystem::process_pending_requests()
{
    std::vector<MeshHandle> pending;
    pending.swap(m_pending_meshes);

    for (const MeshHandle handle : pending)
    {
        if (!is_valid_slot(handle))
            continue;

        MeshSlot& slot = m_mesh_slots[handle.index];

        if (!slot.pending)
            continue;

        slot.pending = false;

        if (slot.requested_resident)
        {
            // Keep the request pending if upload fails
            if (!upload_mesh(handle))
                enqueue_pending(handle);
        }
        else
        {
            if (slot.data.handle == handle)
                release_gpu_mesh(handle);

            slot.pending = false;
            slot.occupied = false;
        }
    }
}

void GpuMeshSystem::enqueue_pending(MeshHandle handle)
{
    if (!is_valid_slot(handle))
        return;

    MeshSlot& slot = m_mesh_slots[handle.index];

    if (slot.pending)
        return;

    slot.pending = true;
    m_pending_meshes.push_back(handle);
}

} // namespace omega