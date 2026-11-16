#ifndef OMEGA_GPU_COMPONENT_BUFFER_TPP
#define OMEGA_GPU_COMPONENT_BUFFER_TPP

#include <omega/core/component/component_system.hpp>
#include <omega/graphics/resources/gpu_component_data.hpp>

#include <orendergraph/render_graph.hpp>

#include <algorithm>
#include <cstring>

namespace omega
{

template <typename T>
ComponentGpuBuffer<T>::ComponentGpuBuffer(ComponentSystem& components)
    : m_components(components)
{}

template <typename T> 
bool ComponentGpuBuffer<T>::create(orendergraph::RenderGraph& rg)
{
    const auto& storage = m_components.get_storage<T>();
    const uint64_t required_size = std::max<uint64_t>(storage.size() * sizeof(GpuComponentData<T>), sizeof(GpuComponentData<T>));

    ogfx::BufferDesc desc{};
    desc.size = required_size;
    desc.usage = ogfx::BufferUsage::Storage | ogfx::BufferUsage::TransferSrc | ogfx::BufferUsage::TransferDst;
    desc.memory_properties = ogfx::MemoryProperty::HostVisible | ogfx::MemoryProperty::HostCoherent;
    if (!m_buffer.create(rg, desc))
        return false;

    return true;
}

template <typename T>
template <typename... TArgs>
bool ComponentGpuBuffer<T>::process_frame(ogfx::CommandBuffer& cmd, orendergraph::RenderGraph& rg,
                                          const std::vector<ComponentEvent<T>>& events,
                                          TArgs&&... args)
{
    auto& storage = m_components.get_storage<T>();

    const uint64_t required_size = std::max<uint64_t>(storage.size() * sizeof(GpuComponentData<T>), sizeof(GpuComponentData<T>));
    const uint64_t previous_capacity = m_buffer.get_capacity();

    if (!m_buffer.ensure_capacity(rg, required_size))
        return false;

    const bool buffer_changed = m_buffer.get_capacity() != previous_capacity;
    if (buffer_changed)
        upload_all(storage, rg, args...);
    else
        process_events(storage, cmd, rg, events, args...);

    return buffer_changed;
}

template <typename T>
template <typename... TArgs>
void ComponentGpuBuffer<T>::upload_all(ComponentStorage<T>& storage, orendergraph::RenderGraph& rg, TArgs&&... args)
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

    ogfx::Buffer* buffer = rg.get_resource(m_buffer.get_handle());

    ogfx::BufferWriteInfo write_info{};
    write_info.data = data.data();
    write_info.size = data.size() * sizeof(GpuComponentData<T>);
    buffer->write(write_info);
}

template <typename T>
template <typename... TArgs>
void ComponentGpuBuffer<T>::write_component(ComponentStorage<T>& storage, Entity entity, orendergraph::RenderGraph& rg, TArgs&&... args)
{
    const uint32_t index = storage.get_index(entity);
    const T* component = storage.get(entity);
    if (!component)
        return;

    const GpuComponentData<T> gpu_data = gpu_component_data(*component, entity, args...);

    ogfx::Buffer* buffer = rg.get_resource(m_buffer.get_handle());

    ogfx::BufferWriteInfo write_info{};
    write_info.data = &gpu_data;
    write_info.size = sizeof(GpuComponentData<T>);
    write_info.offset = index * sizeof(GpuComponentData<T>);
    buffer->write(write_info);
}

template <typename T>
template <typename... TArgs>
void ComponentGpuBuffer<T>::process_events(ComponentStorage<T>& storage, 
                            ogfx::CommandBuffer& cmd, orendergraph::RenderGraph& rg,
                            const std::vector<ComponentEvent<T>>& events,
                            TArgs&&... args)
{
    for (const auto& event : events)
    {
        switch (event.type)
        {
        case ComponentEventType::Added:
        case ComponentEventType::Modified:
        {
            write_component(storage, event.entity, rg, args...);
            break;
        }

        case ComponentEventType::Removed:
        {
            ogfx::Buffer* buffer = rg.get_resource(m_buffer.get_handle());
            constexpr uint64_t element_size = sizeof(GpuComponentData<T>);

            const uint64_t removed_offset = event.swap_info.new_index * element_size;
            const uint64_t last_offset = event.swap_info.old_index * element_size;
            if (removed_offset == last_offset)
                break;

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

            break;
        }
        }
    }
}

template <typename T> 
DynamicBuffer& ComponentGpuBuffer<T>::get_buffer()
{
    return m_buffer;
}

template <typename T> 
const DynamicBuffer& ComponentGpuBuffer<T>::get_buffer() const
{
    return m_buffer;
}

} // namespace omega

#endif // OMEGA_GPU_COMPONENT_BUFFER_TPP