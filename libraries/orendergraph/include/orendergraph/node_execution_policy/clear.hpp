#ifndef ORENDERGRAPH_NODE_EXECUTE_CLEAR_HPP
#define ORENDERGRAPH_NODE_EXECUTE_CLEAR_HPP

#include <orendergraph/node.hpp>
#include <orendergraph/node_context/clear_context.hpp>
#include <orendergraph/node_execution_policy.hpp>
#include <orendergraph/render_graph_core.hpp>

#include <ogfx/command_buffer.hpp>

namespace orendergraph
{

template <> struct NodeExecute<ClearContext>
{
    static void execute(RenderGraph& graph, ogfx::CommandBuffer& cmd, RenderGraphNode<ClearContext>& node)
    {
        const auto& desc = node.descriptor();

        ogfx::ImageClearInfo info{};
        info.image = graph.get_resource(desc.images[desc.image_index]);
        for (uint32_t i = 0; i < 4; ++i)
            info.color[i] = desc.color[i];

        cmd.clear_image(info);
    }
};

} // namespace orendergraph

#endif // ORENDERGRAPH_NODE_EXECUTE_CLEAR_HPP