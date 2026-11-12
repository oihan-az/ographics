#ifndef COMPONENT_GPU_BUFFER_HPP
#define COMPONENT_GPU_BUFFER_HPP

#include <omega/core/component/component_event.hpp>
#include <omega/core/entity/entity.hpp>
#include <omega/graphics/resources/dynamic_buffer.hpp>

#include <orendergraph/resource_handle.hpp>

#include <cstdint>
#include <vector>

namespace ogfx
{
class Buffer;
class CommandBuffer;
} // namespace ogfx

namespace orendergraph
{
class RenderGraph;
}

namespace omega
{

class ComponentSystem;
class EventSystem;
template <typename T> class ComponentStorage;

template <typename T> class ComponentGpuBuffer
{
  public:
    ComponentGpuBuffer(ComponentSystem& components, EventSystem& events, uint32_t frames_in_flight);

    bool create(orendergraph::RenderGraph& rg);

    void capture_events();

    template <typename... TArgs>
    bool process_frame(uint32_t frame_index, ogfx::CommandBuffer& cmd, orendergraph::RenderGraph& rg, TArgs&&... args);

    DynamicBuffer& get_buffer(uint32_t frame_index);
    const DynamicBuffer& get_buffer(uint32_t frame_index) const;

  private:

    struct FrameResources
    {
        DynamicBuffer buffer;

        std::vector<ComponentAddedEvent<T>> pending_added;
        std::vector<ComponentRemovedEvent<T>> pending_removed;
        std::vector<ComponentModifiedEvent<T>> pending_modified;
    };

    ComponentSystem& m_components;
    EventSystem& m_events;

    std::vector<FrameResources> m_frames;

    template <typename... TArgs>
    void upload_all(ComponentStorage<T>& storage, FrameResources& frame, orendergraph::RenderGraph& rg, TArgs&&... args);

    template <typename... TArgs>
    void write_component(ComponentStorage<T>& storage, Entity entity, FrameResources& frame, orendergraph::RenderGraph& rg, TArgs&&... args);

    template <typename... TArgs>
    void process_added(ComponentStorage<T>& storage, FrameResources& frame, orendergraph::RenderGraph& rg, TArgs&&... args);

    void process_removed(FrameResources& frame, ogfx::CommandBuffer& cmd, orendergraph::RenderGraph& rg);

    template <typename... TArgs>
    void process_modified(ComponentStorage<T>& storage, FrameResources& frame, orendergraph::RenderGraph& rg, TArgs&&... args);
};

} // namespace omega

#include "component_gpu_buffer.tpp"

#endif // COMPONENT_GPU_BUFFER_HPP