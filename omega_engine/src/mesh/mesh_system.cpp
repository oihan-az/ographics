#include <omega/mesh/mesh_system.hpp>
#include <omega/mesh/mesh_event.hpp>

#include <omega/core/event/event_system.hpp>

#include <utility>

namespace omega
{

MeshSystem::MeshSystem(EventSystem& events, const MeshSystemDesc& desc) : m_events(events), m_desc(desc) {}

bool MeshSystem::create_mesh(const MeshAsset& asset, MeshHandle& out_handle)
{
    if (!validate_mesh_asset(asset))
    {
        return false;
    }

    // Register or retrieve the vertex layout
    const VertexLayout layout{asset.vertex_input};
    const VertexLayoutId layout_id = m_layout_registry.register_layout(layout);
    if (!layout_id.is_valid())
    {
        return false;
    }

    // Create the corresponding layout group if necessary
    while (m_layout_groups.size() <= layout_id.value)
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

        m_layout_groups.emplace_back(m_desc.page_vertex_capacity, m_desc.page_index_capacity,
                                     std::move(binding_strides));
    }

    MeshLayoutGroup& group = m_layout_groups[layout_id.value];

    // Allocate space in one of the group's pages
    MeshPageAllocation allocation;
    if (!group.allocate(asset.vertex_count, static_cast<uint32_t>(asset.indices.size()), allocation))
    {
        return false;
    }

    // Create non-owning views of the asset's streams
    std::vector<std::span<const std::byte>> vertex_data;
    vertex_data.reserve(asset.vertex_data.size());

    for (const auto& stream : asset.vertex_data)
    {
        vertex_data.emplace_back(stream);
    }

    const std::span<const uint32_t> indices{asset.indices};

    // Copy directly from the asset into the allocated page.
    if (!group.write(allocation, vertex_data, indices))
    {
        group.release(allocation);
        return false;
    }

    // Obtain a mesh slot
    uint32_t mesh_index;
    if (!m_free_mesh_indices.empty())
    {
        mesh_index = m_free_mesh_indices.back();
        m_free_mesh_indices.pop_back();
    }
    else
    {
        mesh_index = static_cast<uint32_t>(m_mesh_slots.size());
        m_mesh_slots.emplace_back();
    }

    MeshSlot& slot = m_mesh_slots[mesh_index];
    slot.occupied = true;

    const MeshHandle handle{mesh_index, slot.generation};
    slot.data = MeshData{handle, layout_id, std::move(allocation), 
                         asset.vertex_count, static_cast<uint32_t>(asset.indices.size())};

    ++m_mesh_count;
    out_handle = handle;

    m_events.publish(MeshCreatedEvent{handle});

    return true;
}

bool MeshSystem::release_mesh(MeshHandle handle)
{
    if (!handle.is_valid() || handle.index >= m_mesh_slots.size())
    {
        return false;
    }

    MeshSlot& slot = m_mesh_slots[handle.index];
    if (!slot.occupied || slot.generation != handle.generation)
    {
        return false;
    }

    const VertexLayoutId layout_id = slot.data.layout;
    if (!layout_id.is_valid() || layout_id.value >= m_layout_groups.size())
    {
        return false;
    }

    if (!m_layout_groups[layout_id.value].release(slot.data.allocation))
    {
        return false;
    }

    slot.occupied = false;
    slot.data = {};
    ++slot.generation;

    m_free_mesh_indices.push_back(handle.index);
    --m_mesh_count;

    m_events.publish(MeshReleasedEvent{handle});

    return true;
}

MeshData* MeshSystem::get(MeshHandle handle)
{
    if (!handle.is_valid() || handle.index >= m_mesh_slots.size())
    {
        return nullptr;
    }

    MeshSlot& slot = m_mesh_slots[handle.index];
    if (!slot.occupied || slot.generation != handle.generation)
    {
        return nullptr;
    }

    return &slot.data;
}

const MeshData* MeshSystem::get(MeshHandle handle) const
{
    if (!handle.is_valid() || handle.index >= m_mesh_slots.size())
    {
        return nullptr;
    }

    const MeshSlot& slot = m_mesh_slots[handle.index];
    if (!slot.occupied || slot.generation != handle.generation)
    {
        return nullptr;
    }

    return &slot.data;
}

uint32_t MeshSystem::mesh_count() const
{
    return m_mesh_count;
}

uint32_t MeshSystem::layout_group_count() const
{
    return static_cast<uint32_t>(m_layout_groups.size());
}

const MeshLayoutGroup* MeshSystem::layout_group(VertexLayoutId layout) const
{
    if (!layout.is_valid() || layout.value >= m_layout_groups.size())
        return nullptr;

    return &m_layout_groups[layout.value];
}

const VertexLayout* MeshSystem::vertex_layout(VertexLayoutId layout) const
{
    return m_layout_registry.get(layout);
}

} // namespace omega