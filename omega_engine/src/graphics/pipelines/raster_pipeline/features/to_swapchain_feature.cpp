#include <omega/graphics/pipelines/raster_pipeline/features/to_swapchain_feature.hpp>
#include <omega/graphics/graphics_system.hpp>

#include <ogfx/swapchain.hpp>

#include <orenderer/render_feature_context.hpp>
#include <orendergraph/render_graph.hpp>

#include <omega/loader/loader.hpp>

#include <filesystem>

namespace omega
{

ToSwapchain::ToSwapchain(const ToSwapchainDesc& desc) : m_desc(desc) {}

void ToSwapchain::build(orenderer::RenderFeatureContext& ctx)
{
    orendergraph::RenderGraph& graph = ctx.render_graph();
    ogfx::Swapchain& swapchain = m_desc.graphics.swapchain();

    m_swapchain_images.resize(swapchain.image_count());
    for (uint32_t i = 0; i < swapchain.image_count(); i++)
        m_swapchain_images[i] = graph.import_resource(&swapchain.image(i));

    add_blit_node(graph);

    for (const auto& image : m_swapchain_images)
        graph.add_end_frame_transition(image, ogfx::ImageState::Present);
}

void ToSwapchain::on_swapchain_recreated(orenderer::RenderFeatureContext& ctx)
{
    orendergraph::RenderGraph& graph = ctx.render_graph();
    ogfx::Swapchain& swapchain = m_desc.graphics.swapchain();

    for (uint32_t i = 0; i < swapchain.image_count(); i++)
    {
        graph.replace_resource(m_swapchain_images[i], &swapchain.image(i));
        graph.reset_resource_state(m_swapchain_images[i]);
    }

    orendergraph::BlitContext::Desc& blit_desc = m_to_swapchain_node->descriptor();
    blit_desc.destination_images = m_swapchain_images;
    blit_desc.width = swapchain.width();
    blit_desc.height = swapchain.height();
}

void ToSwapchain::add_blit_node(orendergraph::RenderGraph& rg)
{
    m_to_swapchain_node = rg.add_node<orendergraph::BlitContext>
    (
        "Blit to Swapchain",
        [this](orendergraph::RenderGraph& rg, orendergraph::BlitContext::Desc& desc)
        {
            ogfx::Swapchain& swapchain = m_desc.graphics.swapchain();

            desc.source_image = m_desc.image;
            desc.destination_images = m_swapchain_images;
            desc.src_mip_level = 0;
            desc.src_base_array_layer = 0;
            desc.dst_mip_level = 0;
            desc.dst_base_array_layer = 0;
            desc.width = swapchain.width();
            desc.height = swapchain.height();
            desc.depth = 1;
            desc.array_layer_count = 1;

            rg.read(m_desc.image, ogfx::ImageState::TransferSrc);
            for (const auto& image : m_swapchain_images)
            {
                rg.write(image, ogfx::ImageState::TransferDst, {ogfx::ImageAspect::Color});
            }
        }
    );

    m_to_swapchain_node->set_pre_execute(
    [this](orendergraph::RenderGraph& rg, ogfx::CommandBuffer& cmd, orendergraph::RenderGraphNode<orendergraph::BlitContext>& node)
    {
        orendergraph::BlitContext::Desc& desc = node.descriptor();
        desc.destination_image_index = m_desc.graphics.current_image_index();

        orendergraph::NodePreExecute<orendergraph::BlitContext>::execute(rg, cmd, *m_to_swapchain_node);
    });
}

}