#ifndef COMPONENT_GPU_BUFFER_TPP
#define COMPONENT_GPU_BUFFER_TPP

#include <omega/core/component/component_system.hpp>
#include <omega/core/event/event_system.hpp>
#include <omega/graphics/gpu_component/gpu_component_data.hpp>

#include <orendergraph/render_graph.hpp>

#include <algorithm>
#include <cstring>

namespace omega
{

template <typename T>
ComponentGpuBuffer<T>::ComponentGpuBuffer(ComponentSystem& components, EventSystem& events, uint32_t frames_in_flight)
    : m_components(components), m_events(events), m_frames(frames_in_flight)
{
}

template <typename T> 
bool ComponentGpuBuffer<T>::create(orendergraph::RenderGraph& rg)
{
    const auto& storage = m_components.get_storage<T>();

    const uint64_t required_size = std::max<uint64_t>(storage.size() * sizeof(GpuComponentData<T>), sizeof(GpuComponentData<T>));

    for (FrameResources& frame : m_frames)
    {
        ogfx::BufferDesc desc{};
        desc.size = required_size;
        desc.usage = ogfx::BufferUsage::Storage | ogfx::BufferUsage::TransferSrc | ogfx::BufferUsage::TransferDst;
        desc.memory_properties = ogfx::MemoryProperty::HostVisible | ogfx::MemoryProperty::HostCoherent;

        if (!frame.buffer.create(rg, desc))
            return false;
    }

    return true;
}

template <typename T> 
void ComponentGpuBuffer<T>::capture_events()
{
    const auto& added = m_events.get_queue<ComponentAddedEvent<T>>().get_events();
    const auto& removed = m_events.get_queue<ComponentRemovedEvent<T>>().get_events();
    const auto& modified = m_events.get_queue<ComponentModifiedEvent<T>>().get_events();

    for (FrameResources& frame : m_frames)
    {
        frame.pending_added.insert(frame.pending_added.end(), added.begin(), added.end());
        frame.pending_removed.insert(frame.pending_removed.end(), removed.begin(), removed.end());
        frame.pending_modified.insert(frame.pending_modified.end(), modified.begin(), modified.end());
    }
}

template <typename T>
template <typename... TArgs>
bool ComponentGpuBuffer<T>::process_frame(uint32_t frame_index, ogfx::CommandBuffer& cmd, orendergraph::RenderGraph& rg,
                                          TArgs&&... args)
{
    if (frame_index >= m_frames.size())
        return false;

    FrameResources& frame = m_frames[frame_index];
    auto& storage = m_components.get_storage<T>();

    const uint64_t required_size = std::max<uint64_t>(storage.size() * sizeof(GpuComponentData<T>), sizeof(GpuComponentData<T>));
    const uint64_t previous_capacity = frame.buffer.get_capacity();

    if (!frame.buffer.ensure_capacity(rg, required_size))
        return false;

    const bool buffer_changed = frame.buffer.get_capacity() != previous_capacity;

    if (buffer_changed)
    {
        upload_all(storage, frame, rg, args...);
    }
    else
    {
        process_added(storage, frame, rg, args...);
        process_removed(frame, cmd, rg);
        process_modified(storage, frame, rg, args...);
    }

    frame.pending_added.clear();
    frame.pending_removed.clear();
    frame.pending_modified.clear();

    return buffer_changed;
}

template <typename T>
template <typename... TArgs>
void ComponentGpuBuffer<T>::upload_all(ComponentStorage<T>& storage, FrameResources& frame,
                                       orendergraph::RenderGraph& rg, TArgs&&... args)
{
    const uint32_t count = static_cast<uint32_t>(storage.size());

    if (count == 0)
        return;

    std::vector<GpuComponentData<T>> data;
    data.reserve(count);

    for (uint32_t i = 0; i < count; ++i)
    {
        const Entity entity = storage.entity_at(i);
        const T* component = storage.get(entity);

        if (!component)
            continue;

        data.push_back(gpu_component_data(*component, entity, args...));
    }

    ogfx::Buffer* buffer = rg.get_resource(frame.buffer.get_handle());

    ogfx::BufferWriteInfo write_info{};
    write_info.data = data.data();
    write_info.size = data.size() * sizeof(GpuComponentData<T>);

    buffer->write(write_info);
}

template <typename T>
template <typename... TArgs>
void ComponentGpuBuffer<T>::write_component(ComponentStorage<T>& storage, Entity entity, FrameResources& frame,
                                            orendergraph::RenderGraph& rg, TArgs&&... args)
{
    const uint32_t index = storage.get_index(entity);
    const T* component = storage.get(entity);

    if (!component)
        return;

    const GpuComponentData<T> gpu_data = gpu_component_data(*component, entity, args...);

    ogfx::Buffer* buffer = rg.get_resource(frame.buffer.get_handle());

    ogfx::BufferWriteInfo write_info{};
    write_info.data = &gpu_data;
    write_info.size = sizeof(GpuComponentData<T>);
    write_info.offset = index * sizeof(GpuComponentData<T>);

    buffer->write(write_info);
}

template <typename T>
template <typename... TArgs>
void ComponentGpuBuffer<T>::process_added(ComponentStorage<T>& storage, FrameResources& frame,
                                          orendergraph::RenderGraph& rg, TArgs&&... args)
{
    for (const auto& event : frame.pending_added)
    {
        write_component(storage, event.entity, frame, rg, args...);
    }
}

template <typename T>
void ComponentGpuBuffer<T>::process_removed(FrameResources& frame, ogfx::CommandBuffer& cmd,
                                            orendergraph::RenderGraph& rg)
{
    ogfx::Buffer* buffer = rg.get_resource(frame.buffer.get_handle());

    constexpr uint64_t element_size = sizeof(GpuComponentData<T>);

    for (const auto& event : frame.pending_removed)
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
template <typename... TArgs>
void ComponentGpuBuffer<T>::process_modified(ComponentStorage<T>& storage, FrameResources& frame,
                                             orendergraph::RenderGraph& rg, TArgs&&... args)
{
    for (const auto& event : frame.pending_modified)
    {
        write_component(storage, event.entity, frame, rg, args...);
    }
}

template <typename T> 
DynamicBuffer& ComponentGpuBuffer<T>::get_buffer(uint32_t frame_index)
{
    return m_frames.at(frame_index).buffer;
}

template <typename T> 
const DynamicBuffer& ComponentGpuBuffer<T>::get_buffer(uint32_t frame_index) const
{
    return m_frames.at(frame_index).buffer;
}

} // namespace omega

#endif // COMPONENT_GPU_BUFFER_TPP