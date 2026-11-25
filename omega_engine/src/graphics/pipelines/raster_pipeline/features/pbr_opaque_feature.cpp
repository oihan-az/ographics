#include <omega/graphics/pipelines/raster_pipeline/features/geometry_feature.hpp>

#include <orenderer/render_feature_context.hpp>
#include <orendergraph/render_graph.hpp>
#include <ogfx/image.hpp>
#include <ogfx/image_view.hpp>
#include <ogfx/resource_allocator.hpp>
#include <ogfx/swapchain.hpp>

#include <omega/core/math/matrix4.hpp>
#include <omega/core/math/vector4.hpp>
#include <omega/core/math/vector3.hpp>
#include <omega/loader/loader.hpp>

#include <omega/core/component/component_system.hpp>
#include <omega/transform/transform_system.hpp>
#include <omega/camera/camera_system.hpp>
#include <omega/mesh/gpu_mesh_system.hpp>
#include <omega/mesh/vertex.hpp>
#include <omega/texture/gpu_texture_system.hpp>
#include <omega/material/material_system.hpp>

#include <omega/graphics/graphics_system.hpp>
#include <omega/graphics/resources/gpu_light_data.hpp>

#include <filesystem>
#include <array>

namespace omega
{

namespace
{

constexpr uint32_t material_texture_count = 5;

std::array<TextureHandle, material_texture_count> get_material_textures(const PbrOpaqueMaterial& material)
{
    return {material.base_color_texture, material.metallic_roughness_texture, material.normal_texture,
            material.occlusion_texture, material.emissive_texture};
}

struct DrawConstants
{
    Matrix4 model;
    Matrix4 view_projection;
};

struct LightingConstants
{
    uint32_t directional_light_count = 0;
    uint32_t point_light_count = 0;
    uint32_t spot_light_count = 0;
    uint32_t padding = 0;
};

struct MaterialConstants
{
    Vector4 factors;
    Vector4 base_color;
    Vector4 emissive;

    uint32_t texture_flags = 0;
    uint32_t padding0 = 0;
    uint32_t padding1 = 0;
    uint32_t padding2 = 0;
};

constexpr uint32_t has_base_color = 1u << 0;
constexpr uint32_t has_metallic_roughness = 1u << 1;
constexpr uint32_t has_normal = 1u << 2;
constexpr uint32_t has_occlusion = 1u << 3;
constexpr uint32_t has_emissive = 1u << 4;

} // namespace

PbrOpaqueRenderFeature::PbrOpaqueRenderFeature(const PbrOpaqueRenderFeatureDesc& desc)
    : m_desc(desc), 
      m_renderable_events(m_desc.graphics.max_frames_in_flight()), 
      m_directional_light_events(m_desc.graphics.max_frames_in_flight()),
      m_point_light_events(m_desc.graphics.max_frames_in_flight()),
      m_spot_light_events(m_desc.graphics.max_frames_in_flight())
{}

void PbrOpaqueRenderFeature::build(orenderer::RenderFeatureContext& ctx)
{
    orendergraph::RenderGraph& rg = ctx.render_graph();

    m_frame_resources.reserve(m_desc.graphics.max_frames_in_flight());
    for (uint32_t i = 0; i < m_desc.graphics.max_frames_in_flight(); i++)
    {
        m_frame_resources.emplace_back(m_desc.components, m_desc.events);

        m_frame_resources[i].directional_lights.create(rg);
        m_frame_resources[i].point_lights.create(rg);
        m_frame_resources[i].spot_lights.create(rg);
    }

    constexpr uint32_t light_type_count = 3;

    ogfx::DescriptorPoolDesc pool_desc;
    pool_desc.max_sets = m_desc.graphics.max_frames_in_flight() + 2;
    pool_desc.sizes = 
    {
        {ogfx::ResourceType::StorageBuffer, m_desc.graphics.max_frames_in_flight() * light_type_count},
        {ogfx::ResourceType::Sampler, 3},
        {ogfx::ResourceType::SampledTexture, 3}
    };
    rg.get_resource_allocator().create_pool(pool_desc);

    create_resources(rg);

    m_default_material = m_desc.graphics.materials().create(PbrOpaqueMaterial{});

    add_node(rg);
}

void PbrOpaqueRenderFeature::on_swapchain_recreated(orenderer::RenderFeatureContext& ctx)
{
    create_output_resources(ctx.render_graph());

    ogfx::Swapchain& swapchain = m_desc.graphics.swapchain();
    orendergraph::RasterContext& raster_ctx = m_geometry_node->context();
    orendergraph::RasterContext::Desc& raster_desc = m_geometry_node->descriptor();

    // Update frame buffer
    ogfx::FramebufferDesc framebuffer_desc;
    framebuffer_desc.attachments = 
    {
        ctx.render_graph().get_resource(m_out_image_view),                             
        ctx.render_graph().get_resource(m_depth_image_view)
    };
    framebuffer_desc.width = swapchain.width();
    framebuffer_desc.height = swapchain.height();
    raster_desc.framebuffers_desc = {framebuffer_desc};

    raster_ctx.create_framebuffers(ctx.render_graph(), raster_desc);

    // Update viewport
    ogfx::Viewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapchain.width());
    viewport.height = static_cast<float>(swapchain.height());
    viewport.min_depth = 0.0f;
    viewport.max_depth = 1.0f;

    // Update scissor
    raster_desc.viewport.viewports = {viewport};

    ogfx::Scissor scissor;
    scissor.x = 0;
    scissor.y = 0;
    scissor.width = swapchain.width();
    scissor.height = swapchain.height();

    raster_desc.viewport.scissors = {scissor};

    // Update the pipeline
    raster_ctx.create_pipeline(ctx.render_graph(), raster_desc);

    // Update renderpass rendering width and height
    raster_desc.width = swapchain.width();
    raster_desc.height = swapchain.height();
}

void PbrOpaqueRenderFeature::set_camera(omega::Entity camera, CameraType camera_type)
{
    m_desc.camera = camera;
    m_desc.camera_type = camera_type;
}

void PbrOpaqueRenderFeature::create_output_resources(orendergraph::RenderGraph& rg)
{
    ogfx::Swapchain& swapchain = m_desc.graphics.swapchain();

    // Output image
    {
        ogfx::ImageDesc image_desc{};
        image_desc.width = swapchain.width();
        image_desc.height = swapchain.height();
        image_desc.format = ogfx::Format::RGBA16_FLOAT;
        image_desc.usage = ogfx::ImageUsage::ColorAttachment | ogfx::ImageUsage::Sampled | ogfx::ImageUsage::TransferSrc;
        if (!m_out_image.is_valid())
            m_out_image = rg.create_resource<ogfx::Image>(image_desc);
        else if(!rg.replace_resource<ogfx::Image>(m_out_image, image_desc)) 
            throw std::runtime_error("Failed to recreate output image");
        else
            rg.reset_resource_state(m_out_image);

        ogfx::ImageViewDesc image_view_desc{};
        image_view_desc.image = rg.get_resource(m_out_image);
        image_view_desc.aspect = ogfx::ImageAspect::Color;
        if (!m_out_image_view.is_valid())
            m_out_image_view = rg.create_resource<ogfx::ImageView>(image_view_desc);
        else if (!rg.replace_resource<ogfx::ImageView>(m_out_image_view, image_view_desc))
            throw std::runtime_error("Failed to recreate output image view");
        else
            rg.reset_resource_state(m_out_image_view);
    }

    // Depth
    {
        ogfx::ImageDesc image_desc{};
        image_desc.width = swapchain.width();
        image_desc.height = swapchain.height();
        image_desc.format = ogfx::Format::D32_FLOAT;
        image_desc.usage = ogfx::ImageUsage::DepthStencil;
        if (!m_depth_image.is_valid())
            m_depth_image = rg.create_resource<ogfx::Image>(image_desc);
        else if (!rg.replace_resource<ogfx::Image>(m_depth_image, image_desc))
            throw std::runtime_error("Failed to recreate depth image");
        else
            rg.reset_resource_state(m_depth_image);

        ogfx::ImageViewDesc image_view_desc{};
        image_view_desc.image = rg.get_resource(m_depth_image);
        image_view_desc.aspect = ogfx::ImageAspect::Depth;
        if (!m_depth_image_view.is_valid())
            m_depth_image_view = rg.create_resource<ogfx::ImageView>(image_view_desc);
        else if (!rg.replace_resource<ogfx::ImageView>(m_depth_image_view, image_view_desc))
            throw std::runtime_error("Failed to recreate depth image view");
        else
            rg.reset_resource_state(m_depth_image_view);
    }
}

void PbrOpaqueRenderFeature::create_resources(orendergraph::RenderGraph& rg)
{
    create_output_resources(rg);

    // Sampler
    {
        ogfx::SamplerDesc desc{};
        m_sampler = rg.create_resource<ogfx::Sampler>(desc);
    }

    // Environment sampler
    {
        ogfx::SamplerDesc desc{};
        desc.min_filter = ogfx::Filter::Linear;
        desc.mag_filter = ogfx::Filter::Linear;
        desc.mip_filter = ogfx::MipmapFilter::Linear;
        desc.address_u = ogfx::AddressMode::ClampToEdge;
        desc.address_v = ogfx::AddressMode::ClampToEdge;
        desc.address_w = ogfx::AddressMode::ClampToEdge;
        m_environment_sampler = rg.create_resource<ogfx::Sampler>(desc);
    }

    // BRDF LUT sampler
    {
        ogfx::SamplerDesc desc{};
        desc.min_filter = ogfx::Filter::Linear;
        desc.mag_filter = ogfx::Filter::Linear;
        desc.mip_filter = ogfx::MipmapFilter::Linear;
        desc.address_u = ogfx::AddressMode::ClampToEdge;
        desc.address_v = ogfx::AddressMode::ClampToEdge;
        desc.address_w = ogfx::AddressMode::ClampToEdge;
        m_brdf_lut_sampler = rg.create_resource<ogfx::Sampler>(desc);
    }
}

void PbrOpaqueRenderFeature::add_node(orendergraph::RenderGraph& rg)
{
    // Geometry pass
    m_geometry_node = rg.add_node<orendergraph::RasterContext>
    (
        "Geometry",
        [this](orendergraph::RenderGraph& rg, orendergraph::RasterContext::Desc& desc)
        {
            // Dependencies
            rg.read(m_desc.irradiance_map, ogfx::ImageState::ShaderRead);
            rg.read(m_desc.environment_map, ogfx::ImageState::ShaderRead);
            rg.read(m_desc.brdf_lut, ogfx::ImageState::ShaderRead);
            rg.write(m_out_image, ogfx::ImageState::RenderTarget, {ogfx::ImageAspect::Color}, ogfx::ImageState::RenderTarget, true);
            rg.write(m_depth_image, ogfx::ImageState::DepthStencil, {ogfx::ImageAspect::Depth}, ogfx::ImageState::DepthStencil, true);

            // Render pass
            ogfx::RenderPassAttachmentDesc color_attachment;
            color_attachment.format = ogfx::Format::RGBA16_FLOAT;
            color_attachment.load_op = ogfx::LoadOp::Clear;
            color_attachment.store_op = ogfx::StoreOp::Store;
            color_attachment.initial_state = ogfx::ImageState::RenderTarget;
            color_attachment.final_state = ogfx::ImageState::RenderTarget;
            color_attachment.image_aspect = ogfx::ImageAspect::Color;

            ogfx::RenderPassAttachmentDesc depth_attachment;
            depth_attachment.format = ogfx::Format::D32_FLOAT;
            depth_attachment.load_op = ogfx::LoadOp::Clear;
            depth_attachment.store_op = ogfx::StoreOp::Store;
            depth_attachment.initial_state = ogfx::ImageState::DepthStencil;
            depth_attachment.final_state = ogfx::ImageState::DepthStencil;
            depth_attachment.image_aspect = ogfx::ImageAspect::Depth;

            desc.render_pass_desc.attachments = {color_attachment, depth_attachment};

            // Framebuffer
            ogfx::Swapchain& swapchain = m_desc.graphics.swapchain();
            ogfx::FramebufferDesc framebuffer_desc;
            framebuffer_desc.attachments = 
            {
                rg.get_resource(m_out_image_view),                             
                rg.get_resource(m_depth_image_view)
            };
            framebuffer_desc.width = swapchain.width();
            framebuffer_desc.height = swapchain.height();
            desc.framebuffers_desc = {framebuffer_desc};

            // Shaders
            const std::filesystem::path shader_dir = "shaders/";
            const std::filesystem::path vertex_shader_path = shader_dir / ("pbr/pbr_opaque.vert." OGRAPHICS_SHADER_FORMAT);
            const std::filesystem::path fragment_shader_path = shader_dir / ("pbr/pbr_opaque.frag." OGRAPHICS_SHADER_FORMAT);
            const std::vector<uint8_t> vertex_bytecode = load_shader(vertex_shader_path);
            const std::vector<uint8_t> fragment_bytecode = load_shader(fragment_shader_path);

            ogfx::ShaderDesc vertex_shader_desc;
            vertex_shader_desc.stage = ogfx::ShaderStage::Vertex;
            vertex_shader_desc.entry_point = "VSMain";
            vertex_shader_desc.bytecode = vertex_bytecode;

            ogfx::ShaderDesc fragment_shader_desc;
            fragment_shader_desc.stage = ogfx::ShaderStage::Fragment;
            fragment_shader_desc.entry_point = "PSMain";
            fragment_shader_desc.bytecode = fragment_bytecode;

            desc.vertex_shader_desc = vertex_shader_desc;
            desc.fragment_shader_desc = fragment_shader_desc;

            // Viewport and scissor
            desc.viewport.dynamic_viewport = false;
            desc.viewport.dynamic_scissor = false;

            ogfx::Viewport viewport;
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(swapchain.width());
            viewport.height = static_cast<float>(swapchain.height());
            viewport.min_depth = 0.0f;
            viewport.max_depth = 1.0f;

            desc.viewport.viewports = {viewport};

            ogfx::Scissor scissor;
            scissor.x = 0;
            scissor.y = 0;
            scissor.width = swapchain.width();
            scissor.height = swapchain.height();

            desc.viewport.scissors = {scissor};

            // Vertex shader constants
            ogfx::InlineConstantRange vertex_constants_range;
            vertex_constants_range.offset = 0;
            vertex_constants_range.size = sizeof(DrawConstants);
            vertex_constants_range.stages = ogfx::ShaderStageFlags::Vertex;

            desc.inline_constants_ranges.push_back(vertex_constants_range);

            // Fragment shader constants
            ogfx::InlineConstantRange fragment_constants_range;
            fragment_constants_range.offset = sizeof(DrawConstants);
            fragment_constants_range.size = sizeof(LightingConstants) + sizeof(MaterialConstants) + sizeof(Vector4);
            fragment_constants_range.stages = ogfx::ShaderStageFlags::Fragment;

            desc.inline_constants_ranges.push_back(fragment_constants_range);

            // Resource set bindings
            ogfx::ResourceSetLayoutDesc set_layout_desc;

            ogfx::ResourceBindingDesc directional_light_binding;
            directional_light_binding.binding = 0;
            directional_light_binding.type = ogfx::ResourceType::StorageBuffer;
            directional_light_binding.stages = ogfx::ShaderStageFlags::Fragment;
            set_layout_desc.bindings.push_back(directional_light_binding);

            ogfx::ResourceBindingDesc point_light_binding;
            point_light_binding.binding = 1;
            point_light_binding.type = ogfx::ResourceType::StorageBuffer;
            point_light_binding.stages = ogfx::ShaderStageFlags::Fragment;
            set_layout_desc.bindings.push_back(point_light_binding);

            ogfx::ResourceBindingDesc spot_light_binding;
            spot_light_binding.binding = 2;
            spot_light_binding.type = ogfx::ResourceType::StorageBuffer;
            spot_light_binding.stages = ogfx::ShaderStageFlags::Fragment;
            set_layout_desc.bindings.push_back(spot_light_binding);

            desc.resource_set_layouts_desc.push_back(set_layout_desc);

            ogfx::ResourceSetLayoutDesc material_set_layout_desc;

            for (uint32_t i = 0; i < material_texture_count; ++i)
            {
                ogfx::ResourceBindingDesc texture_binding{};
                texture_binding.binding = i;
                texture_binding.type = ogfx::ResourceType::SampledTexture;
                texture_binding.stages = ogfx::ShaderStageFlags::Fragment;

                material_set_layout_desc.bindings.push_back(texture_binding);
            }

            desc.resource_set_layouts_desc.push_back(material_set_layout_desc);

            ogfx::ResourceSetLayoutDesc sampler_set_layout_desc;

            ogfx::ResourceBindingDesc sampler_binding;
            sampler_binding.binding = 0;
            sampler_binding.type = ogfx::ResourceType::Sampler;
            sampler_binding.stages = ogfx::ShaderStageFlags::Fragment;
            sampler_set_layout_desc.bindings.push_back(sampler_binding);

            desc.resource_set_layouts_desc.push_back(sampler_set_layout_desc);

            // Environment resource set layout (set 3)
            ogfx::ResourceSetLayoutDesc ibl_layout_desc{};

            ogfx::ResourceBindingDesc environment_texture_binding{};
            environment_texture_binding.binding = 0;
            environment_texture_binding.type = ogfx::ResourceType::SampledTexture;
            environment_texture_binding.stages = ogfx::ShaderStageFlags::Fragment;

            ogfx::ResourceBindingDesc environment_sampler_binding{};
            environment_sampler_binding.binding = 1;
            environment_sampler_binding.type = ogfx::ResourceType::Sampler;
            environment_sampler_binding.stages = ogfx::ShaderStageFlags::Fragment;

            ogfx::ResourceBindingDesc brdf_lut_texture_binding{};
            brdf_lut_texture_binding.binding = 2;
            brdf_lut_texture_binding.type = ogfx::ResourceType::SampledTexture;
            brdf_lut_texture_binding.stages = ogfx::ShaderStageFlags::Fragment;

            ogfx::ResourceBindingDesc brdf_lut_sampler_binding{};
            brdf_lut_sampler_binding.binding = 3;
            brdf_lut_sampler_binding.type = ogfx::ResourceType::Sampler;
            brdf_lut_sampler_binding.stages = ogfx::ShaderStageFlags::Fragment;

            ogfx::ResourceBindingDesc irradiance_map_texture_binding{};
            irradiance_map_texture_binding.binding = 4;
            irradiance_map_texture_binding.type = ogfx::ResourceType::SampledTexture;
            irradiance_map_texture_binding.stages = ogfx::ShaderStageFlags::Fragment;

            ibl_layout_desc.bindings = {environment_texture_binding, environment_sampler_binding,
                                        brdf_lut_texture_binding, brdf_lut_sampler_binding,
                                        irradiance_map_texture_binding};

            desc.resource_set_layouts_desc.push_back(ibl_layout_desc);

            // One resource set and one write description per frame
            for (uint32_t i = 0; i < m_desc.graphics.max_frames_in_flight(); ++i)
            {
                FrameResources& frame = m_frame_resources[i];

                orendergraph::ResourceSetDesc resource_set_desc;
                resource_set_desc.resource_set_layout_idx = 0;

                desc.resource_sets_desc.push_back(resource_set_desc);

                orendergraph::BufferBinding directional_binding;
                directional_binding.binding = 0;
                directional_binding.buffer = frame.directional_lights.get_buffer().get_handle();
                directional_binding.offset = 0;
                directional_binding.range = frame.directional_lights.get_buffer().get_capacity();

                orendergraph::BufferBinding point_binding;
                point_binding.binding = 1;
                point_binding.buffer = frame.point_lights.get_buffer().get_handle();
                point_binding.offset = 0;
                point_binding.range = frame.point_lights.get_buffer().get_capacity();

                orendergraph::BufferBinding spot_binding;
                spot_binding.binding = 2;
                spot_binding.buffer = frame.spot_lights.get_buffer().get_handle();
                spot_binding.offset = 0;
                spot_binding.range = frame.spot_lights.get_buffer().get_capacity();

                frame.directional_buffer_write.resource_set_idx = i;
                frame.directional_buffer_write.write_info.buffers = {directional_binding};
                frame.point_buffer_write.resource_set_idx = i;
                frame.point_buffer_write.write_info.buffers = {point_binding};
                frame.spot_buffer_write.resource_set_idx = i;
                frame.spot_buffer_write.write_info.buffers = {spot_binding};

                orendergraph::ResourceWriteDesc write_desc;
                write_desc.resource_set_idx = i;
                write_desc.write_info.buffers = {directional_binding, point_binding, spot_binding};

                desc.resource_writes_desc.push_back(write_desc);
            }

            orendergraph::ResourceSetDesc resource_set_desc;
            resource_set_desc.resource_set_layout_idx = 2;

            desc.resource_sets_desc.push_back(resource_set_desc);

            orendergraph::SamplerBinding resoruce_sampler_binding;
            resoruce_sampler_binding.sampler = m_sampler;

            orendergraph::ResourceWriteDesc write_desc;
            write_desc.resource_set_idx = m_desc.graphics.max_frames_in_flight();
            write_desc.write_info.samplers = {resoruce_sampler_binding};

            desc.resource_writes_desc.push_back(write_desc);

            // Environment resource set
            orendergraph::ResourceSetDesc environment_set_desc{};
            environment_set_desc.resource_set_layout_idx = 3;

            desc.resource_sets_desc.push_back(environment_set_desc);

            // Environment cubemap binding
            orendergraph::ImageBinding environment_image_binding{};
            environment_image_binding.binding = 0;
            environment_image_binding.image_view = m_desc.environment_map_view;
            environment_image_binding.image_state = ogfx::ImageState::ShaderRead;

            // Environment sampler binding
            orendergraph::SamplerBinding environment_sampler_binding_write{};
            environment_sampler_binding_write.binding = 1;
            environment_sampler_binding_write.sampler = m_environment_sampler;

            // BRDF LUT binding
            orendergraph::ImageBinding brdf_lut_image_binding{};
            brdf_lut_image_binding.binding = 2;
            brdf_lut_image_binding.image_view = m_desc.brdf_lut_view;
            brdf_lut_image_binding.image_state = ogfx::ImageState::ShaderRead;

            // BRDF LUT sampler binding
            orendergraph::SamplerBinding brdf_lut_sampler_binding_write{};
            brdf_lut_sampler_binding_write.binding = 3;
            brdf_lut_sampler_binding_write.sampler = m_brdf_lut_sampler;

            // Irradiance map binding
            orendergraph::ImageBinding irradiance_map_binding{};
            irradiance_map_binding.binding = 4;
            irradiance_map_binding.image_view = m_desc.irradiance_map_view;
            irradiance_map_binding.image_state = ogfx::ImageState::ShaderRead;

            // Resource set write
            orendergraph::ResourceWriteDesc ibl_write_desc{};
            ibl_write_desc.resource_set_idx = m_desc.graphics.max_frames_in_flight() + 1;

            ibl_write_desc.write_info.textures = {environment_image_binding, brdf_lut_image_binding, irradiance_map_binding};
            ibl_write_desc.write_info.samplers = {environment_sampler_binding_write, brdf_lut_sampler_binding_write};

            desc.resource_writes_desc.push_back(ibl_write_desc);

            // Vertex input
            desc.vertex_input = standard_vertex_layout();

            // Input assembly
            desc.input_assembly.topology = ogfx::PrimitiveTopology::TriangleList;
            desc.input_assembly.primitive_restart_enable = false;

            // Rasterization
            desc.rasterization.polygon_mode = ogfx::PolygonMode::Fill;
            desc.rasterization.cull_mode = ogfx::CullMode::Back;
            desc.rasterization.front_face = ogfx::FrontFace::CounterClockwise;

            // Multisampling
            desc.multisampling.rasterization_samples = ogfx::SampleCount::Count1;

            // Depth stencil
            desc.depth_stencil.depth_test_enable = true;
            desc.depth_stencil.depth_write_enable = true;
            desc.depth_stencil.depth_compare_op = ogfx::CompareOp::Less;
            desc.depth_stencil.stencil_test_enable = false;

            // Color blending
            ogfx::ColorBlendAttachmentDesc color_blend;
            color_blend.blend_enable = false;
            color_blend.write_mask = ogfx::ColorComponent::All;

            desc.color_blend.attachments = {color_blend};

            // Runtime render settings
            desc.width = swapchain.width();
            desc.height = swapchain.height();
            desc.clear_values = 
            {
                // Color
                {
                    .type = ogfx::ClearValueType::Color,
                    .color = {0.0f, 0.0f, 0.0f, 1.0f}
                },

                // Depth
                {
                    .type = ogfx::ClearValueType::DepthStencil,
                    .depth = 1.0f,
                    .stencil = 0
                }
            };
        }
    );

    auto& ctx = m_geometry_node->context();
    for (FrameResources& frame : m_frame_resources)
    {
        frame.material_resource_sets = std::make_unique<ResourceSetCache<PbrMaterialHandle, PbrMaterialHash>>(rg.device(), rg.get_resource_allocator(), *ctx.resource_set_layouts[1]);

        // TODO: handle default pbr material
        //if (!frame.m_texture_resource_sets->acquire(m_desc.graphics.missing_texture()))
        //    continue;
        //
        //// Request residency if necessary
        //if (!m_desc.graphics.gpu_textures().is_resident(m_desc.graphics.missing_texture()) &&
        //    !m_desc.graphics.gpu_textures().is_pending(m_desc.graphics.missing_texture()) &&
        //    !m_desc.graphics.gpu_textures().is_uploading(m_desc.graphics.missing_texture()))
        //{
        //    m_desc.graphics.gpu_textures().make_resident(m_desc.graphics.missing_texture());
        //}
        //
        //// Update the resource set
        //ogfx::ImageBinding texture_binding{};
        //texture_binding.binding = 0;
        //texture_binding.image_state = ogfx::ImageState::ShaderRead;
        //texture_binding.image_view = m_desc.graphics.gpu_textures().get_view(m_desc.graphics.missing_texture_view());
        //
        //ogfx::ResourceSetWriteInfo write_info{};
        //write_info.textures.push_back(texture_binding);
        //frame.m_texture_resource_sets->update(m_desc.graphics.missing_texture(), write_info);
    }

    m_geometry_node->set_pre_execute
    (
        [this](orendergraph::RenderGraph& rg, ogfx::CommandBuffer& cmd,
            orendergraph::RenderGraphNode<orendergraph::RasterContext>& node)
        {
            const uint32_t frame_index = m_desc.graphics.current_frame();
            FrameResources& frame = m_frame_resources[frame_index];

            // Update light buffers
            {
                m_directional_light_events.capture(m_desc.events);
                m_point_light_events.capture(m_desc.events);
                m_spot_light_events.capture(m_desc.events);

                if (frame.directional_lights.process_frame(cmd, rg,
                                                           m_directional_light_events.get_events(frame_index),
                                                           m_desc.transforms))
                {
                    frame.directional_buffer_write.write_info.buffers[0].range = frame.directional_lights.get_buffer().get_capacity();
                    node.context().update_resource_set(rg, frame.directional_buffer_write);
                }

                if (frame.point_lights.process_frame(cmd, rg, 
                                                     m_point_light_events.get_events(frame_index), 
                                                     m_desc.transforms))
                {
                    frame.point_buffer_write.write_info.buffers[0].range = frame.point_lights.get_buffer().get_capacity();
                    node.context().update_resource_set(rg, frame.point_buffer_write);
                }

                if (frame.spot_lights.process_frame(cmd, rg, 
                                                    m_spot_light_events.get_events(frame_index),
                                                    m_desc.transforms))
                {
                    frame.spot_buffer_write.write_info.buffers[0].range = frame.spot_lights.get_buffer().get_capacity();
                    node.context().update_resource_set(rg, frame.spot_buffer_write);
                }

                m_directional_light_events.clear(frame_index);
                m_point_light_events.clear(frame_index);
                m_spot_light_events.clear(frame_index);
            }

            // Update texture resource sets
            {
                m_renderable_events.capture(m_desc.events);

                auto* material_resource_sets = frame.material_resource_sets.get();
                auto release_renderable = [&](Entity entity)
                {
                    const auto it = frame.renderable_materials.find(entity);
                    if (it == frame.renderable_materials.end())
                        return;

                    const PbrMaterialHandle previous_material = it->second;

                    frame.renderable_materials.erase(it);

                    material_resource_sets->release(previous_material);

                    if (material_resource_sets->reference_count(previous_material) == 0)
                        frame.pending_materials.erase(previous_material);
                };

                const auto& frame_events = m_renderable_events.get_events(frame_index);
                for (const auto& event : frame_events)
                {
                    switch (event.type)
                    {
                    case ComponentEventType::Removed:
                    {
                        release_renderable(event.entity);
                        break;
                    }

                    case ComponentEventType::Added:
                    case ComponentEventType::Modified:
                    {
                        const PbrOpaqueRenderable* renderable = m_desc.components.get<PbrOpaqueRenderable>(event.entity);

                        if (!renderable || !m_desc.graphics.materials().get(renderable->material))
                        {
                            release_renderable(event.entity);
                            break;
                        }

                        const PbrMaterialHandle material = renderable->material;

                        const auto it = frame.renderable_materials.find(event.entity);
                        if (it != frame.renderable_materials.end())
                        {
                            if (it->second == material)
                            {
                                if (event.type == ComponentEventType::Modified)
                                    frame.pending_materials.insert(material);

                                break;
                            }

                            release_renderable(event.entity);
                        }

                        const bool needs_update = material_resource_sets->reference_count(material) == 0;
                        if (!material_resource_sets->acquire(material))
                            break;

                        frame.renderable_materials[event.entity] = material;

                        if (needs_update)
                            frame.pending_materials.insert(material);

                        break;
                    }
                    }
                }

                m_renderable_events.clear(frame_index);

                auto& gpu_textures = m_desc.graphics.gpu_textures();

                const std::vector<PbrMaterialHandle> pending_materials(frame.pending_materials.begin(), frame.pending_materials.end());
                for (PbrMaterialHandle material_handle : pending_materials)
                {
                    const PbrOpaqueMaterial* material = m_desc.graphics.materials().get(material_handle);
                    if (!material)
                        continue;

                    const auto textures = get_material_textures(*material);

                    bool all_views_ready = true;

                    ogfx::ResourceSetWriteInfo write_info{};

                    for (uint32_t i = 0; i < textures.size(); ++i)
                    {
                        TextureHandle texture = textures[i];

                        // Missing optional textures use the default texture
                        if (!texture.is_valid())
                        {
                            texture = m_desc.graphics.missing_texture();
                        }

                        // Request residency if necessary
                        if (!gpu_textures.is_resident(texture) && 
                            !gpu_textures.is_pending(texture) &&
                            !gpu_textures.is_uploading(texture))
                        {
                            gpu_textures.make_resident(texture);
                        }

                        const ogfx::ImageView* image_view = nullptr;
                        if (texture == m_desc.graphics.missing_texture())
                        {
                            image_view = gpu_textures.get_view(m_desc.graphics.missing_texture_view());
                        }
                        else
                        {
                            // Request a view if it doesn't exist yet
                            auto view_it = m_texture_views.find(texture);
                            if (view_it == m_texture_views.end() || !gpu_textures.is_view_valid(view_it->second))
                            {
                                const ogfx::Image* image = gpu_textures.get_image(texture);
                                if (image)
                                {
                                    TextureViewDesc view_desc{};
                                    view_desc.mip_level_count = image->mip_levels();
                                    const TextureViewHandle view = gpu_textures.create_view(texture, view_desc);
                                    if (view.is_valid())
                                        m_texture_views[texture] = view;
                                }
                            }

                            view_it = m_texture_views.find(texture);
                            if (view_it != m_texture_views.end())
                                image_view = gpu_textures.get_view(view_it->second);
                        }

                        // This texture isn't ready yet
                        if (!image_view)
                        {
                            all_views_ready = false;
                            continue;
                        }

                        // This texture is ready
                        ogfx::ImageBinding binding{};
                        binding.binding = i;
                        binding.image_state = ogfx::ImageState::ShaderRead;
                        binding.image_view = image_view;

                        write_info.textures.push_back(binding);
                    }

                    // Update every texture binding that is ready
                    if (!write_info.textures.empty())
                    {
                        frame.material_resource_sets->update(material_handle, write_info);
                    }

                    // The material is ready only when all bindings have been initialized
                    if (all_views_ready)
                    {
                        frame.pending_materials.erase(material_handle);
                    }
                }

            }

            orendergraph::NodePreExecute<orendergraph::RasterContext>::execute(rg, cmd, node);
        }
    );

    m_geometry_node->set_execute
    (
        [this](orendergraph::RenderGraph& rg, ogfx::CommandBuffer& cmd,
            orendergraph::RenderGraphNode<orendergraph::RasterContext>& node)
        {
            orendergraph::RasterContext& ctx = node.context();
            const uint32_t frame_index = m_desc.graphics.current_frame();
            FrameResources& frame = m_frame_resources[frame_index];

            // Fragment constants
            LightingConstants lighting_constants{};
            lighting_constants.directional_light_count = static_cast<uint32_t>(m_desc.components.get_storage<DirectionalLight>().size());
            lighting_constants.point_light_count = static_cast<uint32_t>(m_desc.components.get_storage<PointLight>().size());
            lighting_constants.spot_light_count = static_cast<uint32_t>(m_desc.components.get_storage<SpotLight>().size());

            ogfx::InlineConstantsInfo lighting_constants_info{};
            lighting_constants_info.layout = ctx.pipeline_layout;
            lighting_constants_info.stages = ogfx::ShaderStageFlags::Fragment;
            lighting_constants_info.offset = sizeof(DrawConstants);
            lighting_constants_info.size = sizeof(LightingConstants);
            lighting_constants_info.data = &lighting_constants;

            cmd.set_inline_constants(lighting_constants_info);

            // Resource sets
            {
                ogfx::ResourceSetBindInfo resource_set_info{};
                resource_set_info.layout = ctx.pipeline_layout;
                resource_set_info.pipeline_type = ogfx::PipelineType::Graphics;
                resource_set_info.set_index = 0;
                resource_set_info.resource_set = ctx.resource_sets[m_desc.graphics.current_frame()];

                cmd.bind_resource_set(resource_set_info);
            }

            // Resource sets
            {
                ogfx::ResourceSetBindInfo resource_set_info{};
                resource_set_info.layout = ctx.pipeline_layout;
                resource_set_info.pipeline_type = ogfx::PipelineType::Graphics;
                resource_set_info.set_index = 2;
                resource_set_info.resource_set = ctx.resource_sets[m_desc.graphics.max_frames_in_flight()];

                cmd.bind_resource_set(resource_set_info);
            }

            // Prefiltered environment cubemap
            {
                ogfx::ResourceSetBindInfo resource_set_info{};
                resource_set_info.layout = ctx.pipeline_layout;
                resource_set_info.pipeline_type = ogfx::PipelineType::Graphics;
                resource_set_info.set_index = 3;
                resource_set_info.resource_set = ctx.resource_sets[m_desc.graphics.max_frames_in_flight() + 1];

                cmd.bind_resource_set(resource_set_info);
            }

            // Camera
            const CameraData camera = m_desc.cameras.get_camera_data(m_desc.camera, m_desc.camera_type);
            Transform* transform = m_desc.components.get<Transform>(m_desc.camera);
            const Vector4 camera_position = Vector4(transform->position, 1.0f);

            ogfx::InlineConstantsInfo camera_constants_info{};
            camera_constants_info.layout = ctx.pipeline_layout;
            camera_constants_info.stages = ogfx::ShaderStageFlags::Fragment;
            camera_constants_info.offset = sizeof(DrawConstants) + sizeof(LightingConstants) + sizeof(MaterialConstants);
            camera_constants_info.size = sizeof(Vector4);
            camera_constants_info.data = &camera_position;

            cmd.set_inline_constants(camera_constants_info);

            // Renderables
            const std::vector<Entity> entities = m_desc.components.entities_with<PbrOpaqueRenderable>();
            for (Entity entity : entities)
            {
                const PbrOpaqueRenderable* renderable = m_desc.components.get<PbrOpaqueRenderable>(entity);

                // GPU mesh
                if (!m_desc.graphics.gpu_meshes().is_resident(renderable->mesh))
                {
                    m_desc.graphics.gpu_meshes().make_resident(renderable->mesh);
                    continue;
                }

                // Draw data
                const GpuMeshDrawData draw_data = m_desc.graphics.gpu_meshes().get_draw_data(renderable->mesh);
                if (!draw_data.is_valid())
                    continue;

                // Bind vertex buffers
                for (const GpuMeshVertexBinding& binding : draw_data.vertex_bindings)
                {
                    ogfx::VertexBufferBindInfo bind_info{};
                    bind_info.buffer = binding.buffer;
                    bind_info.binding = binding.binding;
                    bind_info.offset = binding.offset;

                    cmd.bind_vertex_buffer(bind_info);
                }

                // Index buffer
                if (draw_data.is_indexed())
                {
                    ogfx::IndexBufferBindInfo bind_info{};
                    bind_info.buffer = draw_data.index_buffer;
                    bind_info.offset = draw_data.index_offset;
                    bind_info.index_type = draw_data.index_type;

                    cmd.bind_index_buffer(bind_info);
                }

                // Draw constants
                DrawConstants constants{};
                constants.model = m_desc.transforms.get_world_matrix(entity);
                constants.view_projection = camera.view_projection;

                ogfx::InlineConstantsInfo constants_info{};
                constants_info.layout = ctx.pipeline_layout;
                constants_info.stages = ogfx::ShaderStageFlags::Vertex;
                constants_info.offset = 0;
                constants_info.size = sizeof(DrawConstants);
                constants_info.data = &constants;

                cmd.set_inline_constants(constants_info);

                const PbrOpaqueMaterial* material = m_desc.graphics.materials().get(renderable->material);
                if (!material)
                    continue;

                PbrMaterialHandle material_handle = renderable->material;

                // An unresolved material uses the default material instead
                if (frame.pending_materials.contains(material_handle) || !frame.material_resource_sets->get(material_handle))
                {
                    material_handle = m_default_material;
                }

                const ogfx::ResourceSet* material_resource_set = frame.material_resource_sets->get(material_handle);
                if (!material_resource_set)
                    continue;

                ogfx::ResourceSetBindInfo texture_resource_set_info{};
                texture_resource_set_info.layout = ctx.pipeline_layout;
                texture_resource_set_info.pipeline_type = ogfx::PipelineType::Graphics;
                texture_resource_set_info.set_index = 1;
                texture_resource_set_info.resource_set = const_cast<ogfx::ResourceSet*>(material_resource_set);

                cmd.bind_resource_set(texture_resource_set_info);

                // Material constants
                PbrOpaqueMaterial* draw_material = m_desc.graphics.materials().get(material_handle);
                MaterialConstants material_constants{};

                material_constants.base_color = draw_material->base_color_factor;

                material_constants.factors = Vector4(draw_material->metallic_factor, draw_material->roughness_factor,
                                                     draw_material->normal_scale, draw_material->occlusion_strength);

                material_constants.emissive = Vector4(draw_material->emissive_factor.x, draw_material->emissive_factor.y,
                                                      draw_material->emissive_factor.z, 0.0f);

                if (draw_material->base_color_texture.is_valid())
                    material_constants.texture_flags |= has_base_color;

                if (draw_material->metallic_roughness_texture.is_valid())
                    material_constants.texture_flags |= has_metallic_roughness;

                if (draw_material->normal_texture.is_valid())
                    material_constants.texture_flags |= has_normal;

                if (draw_material->occlusion_texture.is_valid())
                    material_constants.texture_flags |= has_occlusion;

                if (draw_material->emissive_texture.is_valid())
                    material_constants.texture_flags |= has_emissive;

                ogfx::InlineConstantsInfo material_info{};
                material_info.layout = ctx.pipeline_layout;
                material_info.stages = ogfx::ShaderStageFlags::Fragment;
                material_info.offset = sizeof(DrawConstants) + sizeof(LightingConstants);
                material_info.size = sizeof(MaterialConstants);
                material_info.data = &material_constants;

                cmd.set_inline_constants(material_info);

                // Draw
                if (draw_data.is_indexed())
                {
                    ogfx::DrawIndexedInfo draw_info{};
                    draw_info.index_count = draw_data.index_count;
                    draw_info.first_index = 0;

                    cmd.draw_indexed(draw_info);
                }
                else
                {
                    ogfx::DrawInfo draw_info{};
                    draw_info.vertex_count = draw_data.vertex_count;
                    draw_info.first_vertex = 0;

                    cmd.draw(draw_info);
                }
            }
        }
    );
}

} // omega