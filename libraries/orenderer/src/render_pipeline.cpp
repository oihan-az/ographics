#include <orenderer/render_pipeline.hpp>

namespace orenderer
{

RenderPipeline::RenderPipeline(ogfx::PhysicalDevice& physical_device, ogfx::Device& device,
                               ogfx::ResourceAllocator& resource_allocator, orendergraph::ResourcePoolRegistry& shared_registry)
    : m_render_graph(physical_device, device, resource_allocator), m_shared_registry(shared_registry)
{
}

void RenderPipeline::begin_setup()
{
    m_features.clear();
}

void RenderPipeline::compile()
{
    m_render_graph.compile();
}

void RenderPipeline::execute(ogfx::CommandBuffer& command_buffer)
{
    m_render_graph.execute(command_buffer);
}

orendergraph::RenderGraph& RenderPipeline::render_graph()
{
    return m_render_graph;
}

} // namespace orenderer