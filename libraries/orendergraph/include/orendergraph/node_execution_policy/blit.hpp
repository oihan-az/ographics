#ifndef ORENDERGRAPH_NODE_EXECUTE_BLIT_HPP
#define ORENDERGRAPH_NODE_EXECUTE_BLIT_HPP

#include <orendergraph/node_execution_policy.hpp>
#include <orendergraph/node.hpp>
#include <orendergraph/node_context/blas_context.hpp>
#include <orendergraph/render_graph_core.hpp>

#include <ogfx/command_buffer.hpp>

namespace orendergraph
{

template <> 
struct NodeExecute<BlitContext>
{
    static void execute(RenderGraph& graph, ogfx::CommandBuffer& cmd, RenderGraphNode<BlitContext>& node)
    {
        BlitContext::Desc& desc = node.descriptor();

        ogfx::ImageBlitInfo info{};
        info.src_image = graph.get_resource(desc.source_image);
        info.src_mip_level = desc.src_mip_level;
        info.src_base_array_layer = desc.src_base_array_layer;
        info.src_x = desc.src_x;
        info.src_y = desc.src_y;
        info.src_width = desc.src_width;
        info.src_height = desc.src_height;

        info.dst_image = graph.get_resource(desc.destination_images[desc.destination_image_index]);
        info.dst_mip_level = desc.dst_mip_level;
        info.dst_base_array_layer = desc.dst_base_array_layer;
        info.dst_x = desc.dst_x;
        info.dst_y = desc.dst_y;
        info.dst_width = desc.dst_width;
        info.dst_height = desc.dst_height;

        info.depth = desc.depth;
        info.array_layer_count = desc.array_layer_count;

        cmd.blit_image(info);
    }
};

} // namespace orendergraph

#endif // ORENDERGRAPH_NODE_EXECUTE_BLIT_HPP