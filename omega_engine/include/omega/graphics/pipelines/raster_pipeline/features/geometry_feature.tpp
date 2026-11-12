#ifndef GEOMETRY_RENDER_FEATURE_TPP
#define GEOMETRY_RENDER_FEATURE_TPP

#include <omega/core/event/event_system.hpp>
#include <omega/core/component/component_system.hpp>

#include <omega/lighting/gpu_light_data.hpp>

#include <orendergraph/render_graph.hpp>

namespace omega
{

template <>
inline GeometryRenderFeature::LightFrameResources<DirectionalLight>& GeometryRenderFeature::FrameResources::get<DirectionalLight>()
{
    return directional;
}

template <>
inline GeometryRenderFeature::LightFrameResources<PointLight>& GeometryRenderFeature::FrameResources::get<PointLight>()
{
    return point;
}

template <>
inline GeometryRenderFeature::LightFrameResources<SpotLight>& GeometryRenderFeature::FrameResources::get<SpotLight>()
{
    return spot;
}

template <typename T> 
void GeometryRenderFeature::capture_light_events()
{
    const auto& added = m_desc.events.get_queue<ComponentAddedEvent<T>>().get_events();
    const auto& removed = m_desc.events.get_queue<ComponentRemovedEvent<T>>().get_events();
    const auto& modified = m_desc.events.get_queue<ComponentModifiedEvent<T>>().get_events();

    for (FrameResources& frame : m_frames)
    {
        frame.get<T>().pending_added.insert(frame.get<T>().pending_added.end(), added.begin(), added.end());
        frame.get<T>().pending_removed.insert(frame.get<T>().pending_removed.end(), removed.begin(), removed.end());
        frame.get<T>().pending_modified.insert(frame.get<T>().pending_modified.end(), modified.begin(), modified.end());
    }
}

template <typename T> 
void GeometryRenderFeature::process_light_events(uint32_t frame_index, ogfx::CommandBuffer& cmd, orendergraph::RenderGraph& rg)
{
    if (frame_index >= m_frames.size())
        return;

    process_light_events(m_frames[frame_index].get<T>(), cmd, rg);
}

template <typename T> 
void GeometryRenderFeature::process_light_events(LightFrameResources<T>& resources, ogfx::CommandBuffer& cmd, orendergraph::RenderGraph& rg)
{
    auto& storage = m_desc.components.get_storage<T>();

    const uint64_t required_size = std::max<uint64_t>(storage.size() * sizeof(GpuLightData<T>), sizeof(GpuLightData<T>));
    const uint64_t previous_capacity = resources.buffer.get_capacity();
    if (!resources.buffer.ensure_capacity(rg, required_size))
        return;

    // If created or grown, update the descriptor and upload everything.
    const bool buffer_changed = resources.buffer.get_capacity() != previous_capacity;
    if (buffer_changed)
    {
        resources.descriptor_update_pending = true;
        upload_all_lights<T>(storage, resources, rg);
    }
    else
    {
        // Otherwise, apply only pending events.
        process_added_lights<T>(storage, resources, rg);
        process_removed_lights<T>(storage, resources, cmd, rg);
        process_modified_lights<T>(storage, resources, rg);
    }

    resources.pending_added.clear();
    resources.pending_removed.clear();
    resources.pending_modified.clear();
}

template <typename T>
bool GeometryRenderFeature::create_light_buffers(LightFrameResources<T>& resources, uint64_t required_size, orendergraph::RenderGraph& rg)
{
    ogfx::BufferDesc buffer_desc{};
    buffer_desc.size = std::max<uint64_t>(required_size, sizeof(GpuLightData<T>));
    buffer_desc.usage = ogfx::BufferUsage::Storage | ogfx::BufferUsage::TransferSrc | ogfx::BufferUsage::TransferDst;
    buffer_desc.memory_properties = ogfx::MemoryProperty::HostVisible | ogfx::MemoryProperty::HostCoherent;

    return resources.buffer.create(rg, buffer_desc);
}

template <typename T>
void GeometryRenderFeature::upload_all_lights(ComponentStorage<T>& storage, LightFrameResources<T>& resources, orendergraph::RenderGraph& rg)
{
    const uint32_t count = static_cast<uint32_t>(storage.size());
    if (count == 0)
        return;

    std::vector<GpuLightData<T>> light_data;
    light_data.reserve(count);

    for (uint32_t i = 0; i < count; ++i)
    {
        const Entity entity = storage.entity_at(i);
        const T* light = storage.get(entity);

        light_data.push_back(gpu_light_data<T>(entity, *light, m_desc.transforms));
    }

    ogfx::Buffer* buffer = rg.get_resource(resources.buffer.get_handle());

    ogfx::BufferWriteInfo write_info{};
    write_info.data = light_data.data();
    write_info.size = light_data.size() * sizeof(GpuLightData<T>);

    buffer->write(write_info);
}

template <typename T>
void GeometryRenderFeature::process_added_lights(ComponentStorage<T>& storage, LightFrameResources<T>& resources, orendergraph::RenderGraph& rg)
{
    ogfx::Buffer* buffer = rg.get_resource(resources.buffer.get_handle());
    for (const auto& event : resources.pending_added)
    {
        const uint32_t index = storage.get_index(event.entity);
        const T* light = storage.get(event.entity);
        if (!light)
            continue;

        const GpuLightData<T> gpu_data = gpu_light_data(event.entity, *light, m_desc.transforms);

        ogfx::BufferWriteInfo write_info{};
        write_info.data = &gpu_data;
        write_info.size = sizeof(GpuLightData<T>);
        write_info.offset = index * sizeof(GpuLightData<T>);

        buffer->write(write_info);
    }
}

template <typename T>
void GeometryRenderFeature::process_removed_lights(ComponentStorage<T>& storage, LightFrameResources<T>& resources,
                                                   ogfx::CommandBuffer& cmd, orendergraph::RenderGraph& rg)
{
    ogfx::Buffer* buffer = rg.get_resource(resources.buffer.get_handle());
    constexpr uint64_t element_size = sizeof(GpuLightData<T>);

    for (const auto& event : resources.pending_removed)
    {
        const uint64_t removed_offset = event.swap_info.new_index * element_size;
        const uint64_t last_offset = event.swap_info.old_index * element_size;
        if (removed_offset == last_offset)
            continue;

        ogfx::BufferToBufferCopyInfo copy_info{};
        copy_info.src_buffer = buffer;
        copy_info.dst_buffer = buffer;
        copy_info.src_offset = last_offset;
        copy_info.dst_offset = removed_offset;
        copy_info.size = element_size;

        cmd.copy_buffer_to_buffer(copy_info);

        ogfx::BufferBarrierInfo barrier_info{};
        barrier_info.buffer = buffer;
        barrier_info.memory_barrier_info.src_access = ogfx::AccessFlags::TransferWrite;
        barrier_info.memory_barrier_info.dst_access = ogfx::AccessFlags::ShaderRead;
        barrier_info.memory_barrier_info.src_stages = ogfx::PipelineStage::Transfer;
        barrier_info.memory_barrier_info.dst_stages = ogfx::PipelineStage::FragmentShader;
        barrier_info.offset = removed_offset;
        barrier_info.size = element_size;
        cmd.buffer_barrier(barrier_info);
    }
}

template <typename T>
void GeometryRenderFeature::process_modified_lights(ComponentStorage<T>& storage, LightFrameResources<T>& resources, orendergraph::RenderGraph& rg)
{
    ogfx::Buffer* buffer = rg.get_resource(resources.buffer.get_handle());

    for (const auto& event : resources.pending_modified)
    {
        const uint32_t index = storage.get_index(event.entity);
        const T* light = storage.get(event.entity);
        if (!light)
            continue;

        const GpuLightData<T> gpu_data = gpu_light_data<T>(event.entity, *light, m_desc.transforms);

        ogfx::BufferWriteInfo write_info{};
        write_info.data = &gpu_data;
        write_info.size = sizeof(GpuLightData<T>);
        write_info.offset = index * sizeof(GpuLightData<T>);

        buffer->write(write_info);
    }
}

template <typename T>
void GeometryRenderFeature::update_light_resource_set(LightFrameResources<T>& resources, orendergraph::RenderGraph& rg, orendergraph::RenderGraphNode<orendergraph::RasterContext>& node)
{
    if (!resources.descriptor_update_pending)
        return;

    resources.buffer_write.write_info.buffers[0].range = resources.buffer.get_capacity();
    node.context().update_resource_set(rg, resources.buffer_write);

    resources.descriptor_update_pending = false;
}

}

#endif // GEOMETRY_RENDER_FEATURE_TPP