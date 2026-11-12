#ifndef OMEGA_GRAPHICS_RESOURCES_DYNAMIC_BUFFER_HPP
#define OMEGA_GRAPHICS_RESOURCES_DYNAMIC_BUFFER_HPP

#include <orendergraph/resource_handle.hpp>

#include <ogfx/buffer.hpp>

#include <cstdint>

namespace orendergraph
{
class RenderGraph;
}

namespace omega
{

class DynamicBuffer
{
  public:
    bool create(orendergraph::RenderGraph& graph, const ogfx::BufferDesc& desc);

    bool ensure_capacity(orendergraph::RenderGraph& graph, uint64_t required_size);

    void release(orendergraph::RenderGraph& graph);

    orendergraph::ResourceHandle<ogfx::Buffer> get_handle() const;
    uint64_t get_capacity() const;

  private:
    orendergraph::ResourceHandle<ogfx::Buffer> m_handle;
    ogfx::BufferDesc m_desc{};
};

} // namespace omega

#endif // OMEGA_GRAPHICS_RESOURCES_DYNAMIC_BUFFER_HPP