#ifndef ORENDERGRAPH_CONTEXT_RAY_TRACING_HPP
#define ORENDERGRAPH_CONTEXT_RAY_TRACING_HPP

#include <orendergraph/node_context/pipeline_context.hpp>

#include <ogfx/shader.hpp>
#include <ogfx/ray_tracing_pipeline.hpp>

#include <vector>

namespace ogfx
{

class RayTracingPipeline;
class Buffer;

} // namespace ogfx

namespace orendergraph
{

class RenderGraph;

struct RayTracingContext : public PipelineContext
{
    struct Desc : public PipelineContext::Desc
    {
        ogfx::ShaderDesc raygen_shader_desc;
        std::vector<ogfx::ShaderDesc> miss_shader_descs;
        std::vector<RayTracingHitShadersDesc> hit_shader_descs;

        uint32_t max_recursion_depth = 1;

        uint32_t width = 0;
        uint32_t height = 0;
    };

    void create(RenderGraph& graph, const Desc& desc);

    void create_pipeline(RenderGraph& graph, const Desc& desc);
    void create_shaders(RenderGraph& graph, const Desc& desc);
    void create_stb_buffer(RenderGraph& graph, const Desc& desc);

    void release_shaders(RenderGraph& graph);
    void release_pipeline(RenderGraph& graph);
    void release_stb_buffer(RenderGraph& graph);

    const ogfx::Shader* raygen_shader = nullptr;
    std::vector<const ogfx::Shader*> miss_shaders;
    std::vector<ogfx::RayTracingHitShaders> hit_shaders;
    ogfx::RayTracingPipeline* pipeline = nullptr;
    ogfx::Buffer* sbt_buffer = nullptr;
    ResourceHandle<ogfx::Buffer> sbt_buffer_handle;
};

} // namespace orendergraph

#endif // ORENDERGRAPH_CONTEXT_RAY_TRACING_HPP
