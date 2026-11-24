#include <omega/graphics/pipelines/raster_pipeline/features/skybox_feature.hpp>

#include <orenderer/render_feature_context.hpp>
#include <orendergraph/render_graph.hpp>

#include <ogfx/image.hpp>
#include <ogfx/image_view.hpp>
#include <ogfx/resource_allocator.hpp>
#include <ogfx/swapchain.hpp>

#include <omega/camera/camera_system.hpp>
#include <omega/core/math/matrix4.hpp>
#include <omega/graphics/graphics_system.hpp>
#include <omega/loader/loader.hpp>

#include <filesystem>

namespace omega
{

namespace
{

struct SkyboxConstants
{
    Matrix4 view_projection;
};

} // namespace

SkyboxRenderFeature::SkyboxRenderFeature(const SkyboxRenderFeatureDesc& desc) : m_desc(desc) {}

void SkyboxRenderFeature::build(orenderer::RenderFeatureContext& ctx)
{
    orendergraph::RenderGraph& rg = ctx.render_graph();

    ogfx::DescriptorPoolDesc pool_desc{};
    pool_desc.max_sets = 1;
    pool_desc.sizes = {{ogfx::ResourceType::SampledTexture, 1}, {ogfx::ResourceType::Sampler, 1}};

    rg.get_resource_allocator().create_pool(pool_desc);

    create_resources(rg);
    add_node(rg);
}

void SkyboxRenderFeature::set_camera(Entity camera, CameraType camera_type)
{
    m_desc.camera = camera;
    m_desc.camera_type = camera_type;
}

void SkyboxRenderFeature::create_resources(orendergraph::RenderGraph& rg)
{
    ogfx::SamplerDesc sampler_desc{};
    sampler_desc.min_filter = ogfx::Filter::Linear;
    sampler_desc.mag_filter = ogfx::Filter::Linear;
    sampler_desc.mip_filter = ogfx::MipmapFilter::Linear;
    sampler_desc.address_u = ogfx::AddressMode::ClampToEdge;
    sampler_desc.address_v = ogfx::AddressMode::ClampToEdge;
    sampler_desc.address_w = ogfx::AddressMode::ClampToEdge;

    m_sampler = rg.create_resource<ogfx::Sampler>(sampler_desc);
}

void SkyboxRenderFeature::add_node(orendergraph::RenderGraph& rg)
{
    m_skybox_node = rg.add_node<orendergraph::RasterContext>
    (
        "Skybox",
        [this](orendergraph::RenderGraph& rg, orendergraph::RasterContext::Desc& desc)
        {
            rg.read(m_desc.depth_image, ogfx::ImageState::DepthStencil, {ogfx::ImageAspect::Depth});
            rg.read(m_desc.environment_map, ogfx::ImageState::ShaderRead, {ogfx::ImageAspect::Color});
            rg.write(m_desc.color_image, ogfx::ImageState::RenderTarget, {ogfx::ImageAspect::Color}, ogfx::ImageState::ShaderRead, true);

            // Color attachment: preserve geometry results
            ogfx::RenderPassAttachmentDesc color_attachment{};
            color_attachment.format = ogfx::Format::RGBA16_FLOAT;
            color_attachment.load_op = ogfx::LoadOp::Load;
            color_attachment.store_op = ogfx::StoreOp::Store;
            color_attachment.initial_state = ogfx::ImageState::RenderTarget;
            color_attachment.final_state = ogfx::ImageState::ShaderRead;
            color_attachment.image_aspect = ogfx::ImageAspect::Color;

            // Depth attachment: preserve existing geometry depth
            ogfx::RenderPassAttachmentDesc depth_attachment{};
            depth_attachment.format = ogfx::Format::D32_FLOAT;
            depth_attachment.load_op = ogfx::LoadOp::Load;
            depth_attachment.store_op = ogfx::StoreOp::Store;
            depth_attachment.initial_state = ogfx::ImageState::DepthStencil;
            depth_attachment.final_state = ogfx::ImageState::DepthStencil;
            depth_attachment.image_aspect = ogfx::ImageAspect::Depth;

            desc.render_pass_desc.attachments = {color_attachment, depth_attachment};

            ogfx::Swapchain& swapchain = m_desc.graphics.swapchain();

            ogfx::FramebufferDesc framebuffer_desc{};
            framebuffer_desc.attachments = {rg.get_resource(m_desc.color_image_view),
                                            rg.get_resource(m_desc.depth_image_view)};
            framebuffer_desc.width = swapchain.width();
            framebuffer_desc.height = swapchain.height();

            desc.framebuffers_desc = {framebuffer_desc};

            // Shaders
            const std::filesystem::path shader_dir = "shaders/";
            const auto vertex_bytecode = load_shader(shader_dir / ("skybox/skybox.vert." OGRAPHICS_SHADER_FORMAT));
            const auto fragment_bytecode = load_shader(shader_dir / ("skybox/skybox.frag." OGRAPHICS_SHADER_FORMAT));

            ogfx::ShaderDesc vertex_shader_desc{};
            vertex_shader_desc.stage = ogfx::ShaderStage::Vertex;
            vertex_shader_desc.entry_point = "VSMain";
            vertex_shader_desc.bytecode = vertex_bytecode;

            ogfx::ShaderDesc fragment_shader_desc{};
            fragment_shader_desc.stage = ogfx::ShaderStage::Fragment;
            fragment_shader_desc.entry_point = "PSMain";
            fragment_shader_desc.bytecode = fragment_bytecode;

            desc.vertex_shader_desc = vertex_shader_desc;
            desc.fragment_shader_desc = fragment_shader_desc;

            // Viewport and scissor
            desc.viewport.dynamic_viewport = false;
            desc.viewport.dynamic_scissor = false;

            ogfx::Viewport viewport{};
            viewport.width = static_cast<float>(swapchain.width());
            viewport.height = static_cast<float>(swapchain.height());
            viewport.min_depth = 0.0f;
            viewport.max_depth = 1.0f;

            desc.viewport.viewports = {viewport};

            ogfx::Scissor scissor{};
            scissor.width = swapchain.width();
            scissor.height = swapchain.height();

            desc.viewport.scissors = {scissor};

            // Vertex push constants
            ogfx::InlineConstantRange constants_range{};
            constants_range.stages = ogfx::ShaderStageFlags::Vertex;
            constants_range.offset = 0;
            constants_range.size = sizeof(SkyboxConstants);

            desc.inline_constants_ranges.push_back(constants_range);

            // Resource set 0: cubemap and sampler
            ogfx::ResourceBindingDesc texture_binding{};
            texture_binding.binding = 0;
            texture_binding.type = ogfx::ResourceType::SampledTexture;
            texture_binding.stages = ogfx::ShaderStageFlags::Fragment;

            ogfx::ResourceBindingDesc sampler_binding{};
            sampler_binding.binding = 1;
            sampler_binding.type = ogfx::ResourceType::Sampler;
            sampler_binding.stages = ogfx::ShaderStageFlags::Fragment;

            ogfx::ResourceSetLayoutDesc set_layout_desc{};
            set_layout_desc.bindings = {texture_binding, sampler_binding};

            desc.resource_set_layouts_desc.push_back(set_layout_desc);

            orendergraph::ResourceSetDesc resource_set_desc{};
            resource_set_desc.resource_set_layout_idx = 0;

            desc.resource_sets_desc.push_back(resource_set_desc);

            orendergraph::ImageBinding image_binding{};
            image_binding.binding = 0;
            image_binding.image_view = m_desc.environment_map_view;
            image_binding.image_state = ogfx::ImageState::ShaderRead;

            orendergraph::SamplerBinding resource_sampler_binding{};
            resource_sampler_binding.binding = 1;
            resource_sampler_binding.sampler = m_sampler;

            orendergraph::ResourceWriteDesc write_desc{};
            write_desc.resource_set_idx = 0;
            write_desc.write_info.textures = {image_binding};
            write_desc.write_info.samplers = {resource_sampler_binding};

            desc.resource_writes_desc.push_back(write_desc);

            // No vertex buffers
            desc.vertex_input = {};

            desc.input_assembly.topology = ogfx::PrimitiveTopology::TriangleList;
            desc.input_assembly.primitive_restart_enable = false;

            desc.rasterization.polygon_mode = ogfx::PolygonMode::Fill;
            desc.rasterization.cull_mode = ogfx::CullMode::None;
            desc.rasterization.front_face = ogfx::FrontFace::CounterClockwise;

            desc.multisampling.rasterization_samples = ogfx::SampleCount::Count1;

            desc.depth_stencil.depth_test_enable = true;
            desc.depth_stencil.depth_write_enable = false;
            desc.depth_stencil.depth_compare_op = ogfx::CompareOp::LessOrEqual;
            desc.depth_stencil.stencil_test_enable = false;

            ogfx::ColorBlendAttachmentDesc color_blend{};
            color_blend.blend_enable = false;
            color_blend.write_mask = ogfx::ColorComponent::All;

            desc.color_blend.attachments = {color_blend};

            desc.width = swapchain.width();
            desc.height = swapchain.height();
        });

    m_skybox_node->set_execute
    (
        [this](orendergraph::RenderGraph& rg, ogfx::CommandBuffer& cmd,
               orendergraph::RenderGraphNode<orendergraph::RasterContext>& node)
        {
            orendergraph::RasterContext& ctx = node.context();

            // Bind environment cubemap and sampler
            ogfx::ResourceSetBindInfo bind_info{};
            bind_info.layout = ctx.pipeline_layout;
            bind_info.pipeline_type = ogfx::PipelineType::Graphics;
            bind_info.set_index = 0;
            bind_info.resource_set = ctx.resource_sets[0];

            cmd.bind_resource_set(bind_info);

            // Camera
            const CameraData camera = m_desc.cameras.get_camera_data(m_desc.camera, m_desc.camera_type);

            SkyboxConstants constants{};
            constants.view_projection = camera.view_projection;

            ogfx::InlineConstantsInfo constants_info{};
            constants_info.layout = ctx.pipeline_layout;
            constants_info.stages = ogfx::ShaderStageFlags::Vertex;
            constants_info.offset = 0;
            constants_info.size = sizeof(SkyboxConstants);
            constants_info.data = &constants;

            cmd.set_inline_constants(constants_info);

            // Six faces, two triangles each, three vertices per triangle
            ogfx::DrawInfo draw_info{};
            draw_info.vertex_count = 36;
            draw_info.first_vertex = 0;

            cmd.draw(draw_info);
        });
}

void SkyboxRenderFeature::on_swapchain_recreated(orenderer::RenderFeatureContext& ctx)
{
    orendergraph::RenderGraph& rg = ctx.render_graph();
    ogfx::Swapchain& swapchain = m_desc.graphics.swapchain();

    auto& raster_ctx = m_skybox_node->context();
    auto& raster_desc = m_skybox_node->descriptor();

    ogfx::FramebufferDesc framebuffer_desc{};
    framebuffer_desc.attachments = {rg.get_resource(m_desc.color_image_view), rg.get_resource(m_desc.depth_image_view)};
    framebuffer_desc.width = swapchain.width();
    framebuffer_desc.height = swapchain.height();

    raster_desc.framebuffers_desc = {framebuffer_desc};

    raster_ctx.create_framebuffers(rg, raster_desc);

    ogfx::Viewport viewport{};
    viewport.width = static_cast<float>(swapchain.width());
    viewport.height = static_cast<float>(swapchain.height());
    viewport.min_depth = 0.0f;
    viewport.max_depth = 1.0f;

    raster_desc.viewport.viewports = {viewport};

    ogfx::Scissor scissor{};
    scissor.width = swapchain.width();
    scissor.height = swapchain.height();

    raster_desc.viewport.scissors = {scissor};

    raster_ctx.create_pipeline(rg, raster_desc);

    raster_desc.width = swapchain.width();
    raster_desc.height = swapchain.height();
}

} // namespace omega