#include "to_swapchain_feature.hpp"

#include <orenderer/render_feature_context.hpp>
#include <orendergraph/render_graph.hpp>
#include <ogfx/swapchain.hpp>
#include <ogfx/image.hpp>
#include <ogfx/image_view.hpp>

#include "../../scene/geometry_generation.hpp"
#include "../../loader/loader.hpp"

ToSwapchain::ToSwapchain(const ToSwapchainDesc& desc) : m_desc(desc) {}

void ToSwapchain::build(orenderer::RenderFeatureContext& ctx)
{
    orendergraph::RenderGraph& graph = ctx.render_graph();

    add_blit_node(graph);

    for (const auto& image : m_desc.swapchain_images)
    {
        graph.add_end_frame_transition(image, ogfx::ImageState::Present);
    }
}

void ToSwapchain::add_blit_node(orendergraph::RenderGraph& rg)
{
    auto* blit_node = rg.add_node<orendergraph::BlitContext>
    (
        "Blit to Swapchain",
        [this](orendergraph::RenderGraph& rg, orendergraph::BlitContext::Desc& desc)
        {
            desc.source_image = m_desc.image;
            desc.destination_images = m_desc.swapchain_images;
            desc.src_mip_level = 0;
            desc.src_base_array_layer = 0;
            desc.dst_mip_level = 0;
            desc.dst_base_array_layer = 0;
            desc.width = m_desc.swapchain_width;
            desc.height = m_desc.swapchain_height;
            desc.depth = 1;
            desc.array_layer_count = 1;

            rg.read(m_desc.image, ogfx::ImageState::TransferSrc);
            for (const auto& image : m_desc.swapchain_images)
            {
                rg.write(image, ogfx::ImageState::TransferDst, {ogfx::ImageAspect::Color});
            }
        }
    );

    blit_node->set_pre_execute(
    [this, &blit_node](orendergraph::RenderGraph& rg, ogfx::CommandBuffer& cmd, orendergraph::RenderGraphNode<orendergraph::BlitContext>& node)
    {
        orendergraph::BlitContext::Desc& desc = node.descriptor();
        desc.destination_image_index = *m_desc.image_index;

        orendergraph::NodePreExecute<orendergraph::BlitContext>::execute(rg, cmd, *blit_node);
    });
}