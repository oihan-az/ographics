#ifndef ORENDERER_RENDERER_HPP
#define ORENDERER_RENDERER_HPP

#include <orenderer/render_pipeline.hpp>

#include <orendergraph/resource_pool_registry.hpp>

#include <cstdint>
#include <memory>
#include <vector>

namespace orenderer
{

static constexpr int32_t INVALID_PIPELINE_INDEX = static_cast<int32_t>(-1);

class Renderer
{
  public:
    Renderer(ogfx::PhysicalDevice& physical_device, ogfx::Device& device, ogfx::ResourceAllocator& resource_allocator);

    ~Renderer();

    template <typename TPipeline> TPipeline& create_pipeline();

    bool set_active_pipeline(uint32_t index);
    bool has_active_pipeline() const;

    std::size_t active_pipeline_index() const;
    RenderPipeline* active_pipeline();
    const RenderPipeline* active_pipeline() const;

    template <typename TPipeline> TPipeline* pipeline(uint32_t index);
    RenderPipeline* pipeline(uint32_t index);
    const RenderPipeline* pipeline(uint32_t index) const;

    uint32_t pipeline_count() const;

    orendergraph::ResourcePoolRegistry& resource_registry();

    template <typename TPipeline, typename TDesc> void setup(TDesc& desc);
    void compile();
    void execute(ogfx::CommandBuffer& command_buffer);

  private:
    ogfx::PhysicalDevice& m_physical_device;
    ogfx::Device& m_device;
    ogfx::ResourceAllocator& m_resource_allocator;

    orendergraph::ResourcePoolRegistry m_registry;

    std::vector<std::unique_ptr<RenderPipeline>> m_pipelines;
    int32_t m_active_pipeline_index = INVALID_PIPELINE_INDEX;
};

} // namespace orenderer

#include <orenderer/renderer.tpp>

#endif // ORENDERER_RENDERER_HPP