#ifndef ORENDERER_RENDER_PIPELINE_HPP
#define ORENDERER_RENDER_PIPELINE_HPP

#include <orendergraph/render_graph.hpp>
#include <orenderer/render_feature.hpp>
#include <orenderer/render_feature_context.hpp>

#include <memory>
#include <vector>

namespace ogfx
{

class PhysicalDevice;
class Device;
class ResourceAllocator;
class CommandBuffer;

} // namespace ogfx

namespace orendergraph
{

class ResourcePoolRegistry;

} // namespace orendergraph

namespace orenderer
{

class RenderPipeline
{
  public:
    RenderPipeline(ogfx::PhysicalDevice& physical_device, ogfx::Device& device,
                   ogfx::ResourceAllocator& resource_allocator, orendergraph::ResourcePoolRegistry& shared_registry);

    virtual ~RenderPipeline() = default;

    template <typename TFeature, typename TDesc> TFeature& add_feature(const TDesc& desc);

    void begin_setup();
    void compile();
    void execute(ogfx::CommandBuffer& command_buffer);

    orendergraph::RenderGraph& render_graph();

  protected:
    orendergraph::RenderGraph m_render_graph;
    orendergraph::ResourcePoolRegistry& m_shared_registry;

  private:
    std::vector<std::unique_ptr<IRenderFeature>> m_features;
};

} // namespace orenderer

#include <orenderer/render_pipeline.tpp>

#endif // ORENDERER_RENDER_PIPELINE_HPP