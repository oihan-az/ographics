#ifndef ORENDERGRAPH_NODE_EXECUTE_RASTER_HPP
#define ORENDERGRAPH_NODE_EXECUTE_RASTER_HPP

#include <orendergraph/node_execution_policy.hpp>
#include <orendergraph/node.hpp>
#include <orendergraph/node_context/raster_context.hpp>

#include <ogfx/command_buffer.hpp>
#include <ogfx/commands/render_pass_commands.hpp>
#include <ogfx/commands/binding_commands.hpp>

namespace orendergraph
{

template <> 
struct NodePreExecute<RasterContext>
{
    static void execute(RenderGraph&, ogfx::CommandBuffer& cmd, RenderGraphNode<RasterContext>& node)
    {
        RasterContext& ctx = node.context();
        RasterContext::Desc& desc = node.descriptor();

        ogfx::RenderPassBeginInfo render_pass_begin_info;
        render_pass_begin_info.width = desc.width;
        render_pass_begin_info.height = desc.height;
        render_pass_begin_info.clear_color = desc.clear_color;
        render_pass_begin_info.render_pass = ctx.render_pass;
        render_pass_begin_info.framebuffer = ctx.framebuffers[desc.framebuffer_index];
        cmd.begin_render_pass(render_pass_begin_info);

        ogfx::PipelineBindInfo graphics_pipeline_bind_info;
        graphics_pipeline_bind_info.pipeline = ctx.pipeline;
        cmd.bind_pipeline(graphics_pipeline_bind_info);
    }
};

template <> 
struct NodePostExecute<RasterContext>
{
    static void execute(RenderGraph&, ogfx::CommandBuffer& cmd, RenderGraphNode<RasterContext>& node)
    {
        cmd.end_render_pass();
    }
};

} // namespace orendergraph

#endif // ORENDERGRAPH_NODE_EXECUTE_RASTER_HPP