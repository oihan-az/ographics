#ifndef ORENDERGRAPH_NODE_EXECUTE_RAY_TRACING_HPP
#define ORENDERGRAPH_NODE_EXECUTE_RAY_TRACING_HPP

#include <orendergraph/node_execution_policy.hpp>
#include <orendergraph/node.hpp>
#include <orendergraph/node_context/ray_tracing_context.hpp>

#include <ogfx/command_buffer.hpp>
#include <ogfx/commands/binding_commands.hpp>

namespace orendergraph
{

template <> 
struct NodePreExecute<RayTracingContext>
{
    static void execute(RenderGraph&, ogfx::CommandBuffer& cmd, RenderGraphNode<RayTracingContext>& node)
    {
        RayTracingContext& ctx = node.context();

        ogfx::PipelineBindInfo graphics_pipeline_bind_info;
        graphics_pipeline_bind_info.pipeline = ctx.pipeline;
        cmd.bind_pipeline(graphics_pipeline_bind_info);
    }
};

} // namespace orendergraph

#endif // ORENDERGRAPH_NODE_EXECUTE_RASTER_HPP