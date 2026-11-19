#include <omega_editor/rendering/ui_feature.hpp>

#include <omega/graphics/graphics_system.hpp>

#include <ogfx/swapchain.hpp>

#include <orenderer/render_feature_context.hpp>
#include <orendergraph/render_graph.hpp>

#include <omega/loader/loader.hpp>

#include <omega_editor/ui/imgui_layer.hpp>

#include <filesystem>

namespace omega_editor
{

UIFeature::UIFeature(const UIFeatureDesc& desc) : m_desc(desc) {}

void UIFeature::build(orenderer::RenderFeatureContext& ctx)
{
    orendergraph::RenderGraph& graph = ctx.render_graph();
    ogfx::Swapchain& swapchain = m_desc.graphics.swapchain();

    m_swapchain_image_views.resize(swapchain.image_count());
    for (uint32_t i = 0; i < swapchain.image_count(); i++)
    {
        m_swapchain_image_views[i] = graph.import_resource(&swapchain.image_view(i));
    }

    add_node(graph);
}

void UIFeature::on_swapchain_recreated(orenderer::RenderFeatureContext& ctx)
{
    orendergraph::RenderGraph& graph = ctx.render_graph();
    ogfx::Swapchain& swapchain = m_desc.graphics.swapchain();

    for (uint32_t i = 0; i < swapchain.image_count(); i++)
    {
        graph.replace_resource(m_swapchain_image_views[i], &swapchain.image_view(i));
        graph.reset_resource_state(m_swapchain_image_views[i]);
    }

    orendergraph::RasterContext& raster_ctx = m_node->context();
    orendergraph::RasterContext::Desc& raster_desc = m_node->descriptor();

    // Update frame buffer
    raster_desc.framebuffers_desc.clear();
    for (const auto& image_view : m_swapchain_image_views)
    {
        ogfx::FramebufferDesc framebuffer_desc;
        framebuffer_desc.attachments.push_back(graph.get_resource(image_view));
        framebuffer_desc.width = swapchain.width();
        framebuffer_desc.height = swapchain.height();

        raster_desc.framebuffers_desc.push_back(framebuffer_desc);
    }

    raster_ctx.create_framebuffers(graph, raster_desc);

    // Update renderpass rendering width and height
    raster_desc.width = swapchain.width();
    raster_desc.height = swapchain.height();
}

void UIFeature::add_node(orendergraph::RenderGraph& rg)
{
    m_node = rg.add_node<orendergraph::RasterContext>
    (
        "UI",
        [this](orendergraph::RenderGraph& rg, orendergraph::RasterContext::Desc& desc)
        {
            ogfx::Swapchain& swapchain = m_desc.graphics.swapchain();

            // Dependencies
            m_swapchain_reads.clear();
            m_swapchain_writes.clear();
            for (const auto& image : m_desc.m_swapchain_images)
            {
                m_swapchain_reads.push_back(&rg.read(image, ogfx::ImageState::RenderTarget, {ogfx::ImageAspect::Color}));
                m_swapchain_writes.push_back(&rg.write(image, ogfx::ImageState::RenderTarget, {ogfx::ImageAspect::Color}, ogfx::ImageState::Present, true));
            }

            // Render pass
            ogfx::RenderPassAttachmentDesc swapchain_attachment;
            swapchain_attachment.format = swapchain.format();
            swapchain_attachment.load_op = ogfx::LoadOp::Load;
            swapchain_attachment.store_op = ogfx::StoreOp::Store;
            swapchain_attachment.initial_state = ogfx::ImageState::RenderTarget;
            swapchain_attachment.final_state = ogfx::ImageState::Present;
            swapchain_attachment.image_aspect = ogfx::ImageAspect::Color;

            desc.render_pass_desc.attachments = {swapchain_attachment};

            // Framebuffer
            for (const auto& image_view : m_swapchain_image_views)
            {
                ogfx::FramebufferDesc framebuffer_desc;
                framebuffer_desc.attachments.push_back(rg.get_resource(image_view));
                framebuffer_desc.width = swapchain.width();
                framebuffer_desc.height = swapchain.height();

                desc.framebuffers_desc.push_back(framebuffer_desc);
            }

            // Runtime render settings
            desc.width = swapchain.width();
            desc.height = swapchain.height();
        }
    );

    m_desc.imgui_layer.initialize(m_desc.platform, m_desc.graphics, *this);

    m_node->set_prepare
    (
        [this](orendergraph::RenderGraph& rg)
        {
            const uint32_t current_image = m_desc.graphics.current_image_index();
            for (uint32_t i = 0; i < m_swapchain_writes.size(); ++i)
            {
                m_swapchain_reads[i]->set_active(i == current_image);
                m_swapchain_writes[i]->set_active(i == current_image);
            }
        }
    );

    m_node->set_pre_execute
    (
        [this](orendergraph::RenderGraph& rg, ogfx::CommandBuffer& cmd,
            orendergraph::RenderGraphNode<orendergraph::RasterContext>& node)
        {
            orendergraph::RasterContext& ctx = node.context();
            orendergraph::RasterContext::Desc& desc = node.descriptor();
            desc.framebuffer_index = m_desc.graphics.current_image_index();

            ogfx::RenderPassBeginInfo render_pass_begin_info;
            render_pass_begin_info.offset_x = desc.offset_x;
            render_pass_begin_info.offset_y = desc.offset_y;
            render_pass_begin_info.width = desc.width;
            render_pass_begin_info.height = desc.height;
            render_pass_begin_info.clear_values = desc.clear_values;
            render_pass_begin_info.render_pass = ctx.render_pass;
            render_pass_begin_info.framebuffer = ctx.framebuffers[desc.framebuffer_index];
            cmd.begin_render_pass(render_pass_begin_info);
        }
    );

    m_node->set_execute
    (
        [this](orendergraph::RenderGraph& rg, ogfx::CommandBuffer& cmd,
            orendergraph::RenderGraphNode<orendergraph::RasterContext>& node)
        {
            m_desc.imgui_layer.render(cmd);
        }
    );
}

} // namespace omega