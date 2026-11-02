#ifndef ORENDERGRAPH_CONTEXT_COMPUTE_HPP
#define ORENDERGRAPH_CONTEXT_COMPUTE_HPP

#include <orendergraph/node_context/pipeline_context.hpp>

#include <ogfx/shader.hpp>

#include <cstdint>

namespace ogfx
{

class ComputePipeline;

} // namespace ogfx

namespace orendergraph
{

class RenderGraph;

struct ComputeContext : public PipelineContext
{
    struct Desc : public PipelineContext::Desc
    {
        ogfx::ShaderDesc compute_shader_desc;

        uint32_t group_count_x = 1;
        uint32_t group_count_y = 1;
        uint32_t group_count_z = 1;
    };

    void create(RenderGraph& graph, const Desc& desc);
    void clear();

    ogfx::ComputePipeline* pipeline = nullptr;
};

} // namespace orendergraph

#endif // ORENDERGRAPH_CONTEXT_COMPUTE_HPP
