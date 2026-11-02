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

        ogfx::ImageToImageCopyInfo copy_info{};
        copy_info.src_image = graph.get_resource(desc.source_image);
        copy_info.src_mip_level = desc.src_mip_level;
        copy_info.src_base_array_layer = desc.src_base_array_layer;

        copy_info.dst_image = graph.get_resource(desc.destination_images[desc.destination_image_index]);
        copy_info.dst_mip_level = desc.dst_mip_level;
        copy_info.dst_base_array_layer = desc.dst_base_array_layer;

        copy_info.width = desc.width;
        copy_info.height = desc.height;
        copy_info.depth = desc.depth;
        copy_info.array_layer_count = desc.array_layer_count;

        cmd.blit_image(copy_info);
    }
};

} // namespace orendergraph

#endif // ORENDERGRAPH_NODE_EXECUTE_BLIT_HPP