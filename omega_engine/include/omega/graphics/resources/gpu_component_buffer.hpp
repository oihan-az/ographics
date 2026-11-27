#ifndef OMEGA_GPU_COMPONENT_BUFFER_HPP
#define OMEGA_GPU_COMPONENT_BUFFER_HPP

#include <omega/core/component/component_event.hpp>
#include <omega/core/entity/entity.hpp>
#include <omega/graphics/resources/gpu_dynamic_buffer.hpp>

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
template <typename T> class ComponentStorage;

template <typename T> class ComponentGpuBuffer
{
  public:
    ComponentGpuBuffer(ComponentSystem& components);

    bool create(orendergraph::RenderGraph& rg);

    template <typename... TArgs>
    bool process_frame(ogfx::CommandBuffer& cmd, orendergraph::RenderGraph& rg,
                       const std::vector<ComponentEvent<T>>& events,
                       TArgs&&... args);

    template <typename... TArgs>
    void upload_all(ComponentStorage<T>& storage, orendergraph::RenderGraph& rg, TArgs&&... args);

    template <typename... TArgs>
    void write_component(ComponentStorage<T>& storage, Entity entity, orendergraph::RenderGraph& rg, TArgs&&... args);

    template <typename... TArgs>
    void process_events(ComponentStorage<T>& storage, ogfx::CommandBuffer& cmd, orendergraph::RenderGraph& rg,
                        const std::vector<ComponentEvent<T>>& events,
                        TArgs&&... args);

    DynamicBuffer& get_buffer();
    const DynamicBuffer& get_buffer() const;

  private:
    ComponentSystem& m_components;

    DynamicBuffer m_buffer;
};

} // namespace omega

#include "gpu_component_buffer.tpp"

#endif // OMEGA_GPU_COMPONENT_BUFFER_HPP