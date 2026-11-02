#ifndef ORENDERGRAPH_CONTEXT_RAY_TRACING_HPP
#define ORENDERGRAPH_CONTEXT_RAY_TRACING_HPP

#include <orendergraph/node_context/pipeline_context.hpp>

#include <ogfx/shader.hpp>

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
    void clear();

    ogfx::RayTracingPipeline* pipeline = nullptr;
    ogfx::Buffer* sbt_buffer = nullptr;
};

} // namespace orendergraph

#endif // ORENDERGRAPH_CONTEXT_RAY_TRACING_HPP
