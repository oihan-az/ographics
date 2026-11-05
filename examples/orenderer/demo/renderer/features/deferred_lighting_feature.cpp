#include "deferred_lighting_feature.hpp"

#include <orenderer/render_feature_context.hpp>
#include <orendergraph/render_graph.hpp>
#include <ogfx/resource_allocator.hpp>
#include <ogfx/image_view.hpp>

#include "../../scene/geometry_generation.hpp"
#include "../../loader/loader.hpp"
#include "to_swapchain_feature.hpp"

struct DrawConstants
{
    float model[16];
    float view[16];
    float projection[16];
    float color[4];
};

struct LightingConstants
{
    uint32_t width;
    uint32_t height;
    float ambient_strength;
    float padding;

    float light_direction[4];
    float light_color[4];
};

DeferredLighting::DeferredLighting(const DeferredLightingDesc& desc) : m_desc(desc) {}

void DeferredLighting::build(orenderer::RenderFeatureContext& ctx)
{
    orendergraph::RenderGraph& graph = ctx.render_graph();

    ogfx::DescriptorPoolDesc pool_desc;
    pool_desc.max_sets = 2;
    pool_desc.sizes = {{ogfx::ResourceType::StorageTexture, 1},
                       {ogfx::ResourceType::SampledTexture, 3}};

    graph.get_resource_allocator().create_pool(pool_desc);

    create_resources(graph);

    add_geometry_node(graph);
    add_lighting_node(graph);
}

void DeferredLighting::create_resources(orendergraph::RenderGraph& rg)
{
    // Albedo
    {
        ogfx::ImageDesc image_desc{};
        image_desc.width = m_desc.swapchain_width;
        image_desc.height = m_desc.swapchain_height;
        image_desc.format = ogfx::Format::RGBA8_UNORM;
        image_desc.usage = ogfx::ImageUsage::ColorAttachment | ogfx::ImageUsage::Sampled;
        m_albedo_image = rg.create_resource<ogfx::Image>(image_desc);

        ogfx::ImageViewDesc image_view_desc{};
        image_view_desc.image = rg.get_resource(m_albedo_image);
        image_view_desc.aspect = ogfx::ImageAspect::Color;
        m_albedo_image_view = rg.create_resource<ogfx::ImageView>(image_view_desc);
    }

    // Normal
    {
        ogfx::ImageDesc desc{};
        desc.width = m_desc.swapchain_width;
        desc.height = m_desc.swapchain_height;
        desc.format = ogfx::Format::RGBA16_FLOAT;
        desc.usage = ogfx::ImageUsage::ColorAttachment | ogfx::ImageUsage::Sampled;
        m_normal_image = rg.create_resource<ogfx::Image>(desc);

        ogfx::ImageViewDesc image_view_desc{};
        image_view_desc.image = rg.get_resource(m_normal_image);
        image_view_desc.aspect = ogfx::ImageAspect::Color;
        m_normal_image_view = rg.create_resource<ogfx::ImageView>(image_view_desc);
    }

    // Depth
    {
        ogfx::ImageDesc desc{};
        desc.width = m_desc.swapchain_width;
        desc.height = m_desc.swapchain_height;
        desc.format = ogfx::Format::D32_FLOAT;
        desc.usage = ogfx::ImageUsage::DepthStencil | ogfx::ImageUsage::Sampled;
        m_depth_image = rg.create_resource<ogfx::Image>(desc);

        ogfx::ImageViewDesc image_view_desc{};
        image_view_desc.image = rg.get_resource(m_depth_image);
        image_view_desc.aspect = ogfx::ImageAspect::Depth;
        m_depth_image_view = rg.create_resource<ogfx::ImageView>(image_view_desc);
    }

    // Lighting
    {
        ogfx::ImageDesc image_desc{};
        image_desc.width = m_desc.swapchain_width;
        image_desc.height = m_desc.swapchain_height;
        image_desc.format = ogfx::Format::RGBA16_FLOAT;
        image_desc.usage = ogfx::ImageUsage::Storage | ogfx::ImageUsage::Sampled | ogfx::ImageUsage::TransferSrc;
        m_lighting_image = rg.create_resource<ogfx::Image>(image_desc);

        ogfx::ImageViewDesc image_view_desc{};
        image_view_desc.image = rg.get_resource(m_lighting_image);
        image_view_desc.aspect = ogfx::ImageAspect::Color;
        m_lighting_image_view = rg.create_resource<ogfx::ImageView>(image_view_desc);
    }
}

void DeferredLighting::add_geometry_node(orendergraph::RenderGraph& rg)
{
    // Geometry pass
    auto geometry_node = rg.add_node<orendergraph::RasterContext>
    (
        "Geometry",
        [this](orendergraph::RenderGraph& rg, orendergraph::RasterContext::Desc& desc)
        {
            // Dependencies
            rg.write(m_albedo_image, ogfx::ImageState::RenderTarget, { ogfx::ImageAspect::Color }, ogfx::ImageState::ShaderRead, true );
            rg.write(m_normal_image, ogfx::ImageState::RenderTarget, { ogfx::ImageAspect::Color }, ogfx::ImageState::ShaderRead, true );
            rg.write(m_depth_image, ogfx::ImageState::DepthStencil, { ogfx::ImageAspect::Depth }, ogfx::ImageState::ShaderRead, true);

            // Render pass
            ogfx::RenderPassAttachmentDesc albedo_attachment;
            albedo_attachment.format = ogfx::Format::RGBA8_UNORM;
            albedo_attachment.load_op = ogfx::LoadOp::Clear;
            albedo_attachment.store_op = ogfx::StoreOp::Store;
            albedo_attachment.initial_state = ogfx::ImageState::Undefined;
            albedo_attachment.final_state = ogfx::ImageState::ShaderRead;
            albedo_attachment.image_aspect = ogfx::ImageAspect::Color;

            ogfx::RenderPassAttachmentDesc normal_attachment;
            normal_attachment.format = ogfx::Format::RGBA16_FLOAT;
            normal_attachment.load_op = ogfx::LoadOp::Clear;
            normal_attachment.store_op = ogfx::StoreOp::Store;
            normal_attachment.initial_state = ogfx::ImageState::Undefined;
            normal_attachment.final_state = ogfx::ImageState::ShaderRead;
            normal_attachment.image_aspect = ogfx::ImageAspect::Color;

            ogfx::RenderPassAttachmentDesc depth_attachment;
            depth_attachment.format = ogfx::Format::D32_FLOAT;
            depth_attachment.load_op = ogfx::LoadOp::Clear;
            depth_attachment.store_op = ogfx::StoreOp::Store;
            depth_attachment.initial_state = ogfx::ImageState::Undefined;
            depth_attachment.final_state = ogfx::ImageState::ShaderRead;
            depth_attachment.image_aspect = ogfx::ImageAspect::Depth;

            desc.render_pass_desc.attachments = {albedo_attachment, normal_attachment, depth_attachment};

            // Framebuffer
            ogfx::FramebufferDesc framebuffer_desc;
            framebuffer_desc.attachments = 
            {
                rg.get_resource(m_albedo_image_view), 
                rg.get_resource(m_normal_image_view),                               
                rg.get_resource(m_depth_image_view)
            };
            framebuffer_desc.width = m_desc.swapchain_width;
            framebuffer_desc.height = m_desc.swapchain_height;
            desc.framebuffers_desc = {framebuffer_desc};

            // Shaders
            const std::filesystem::path shader_dir = "shaders/";
            const std::filesystem::path vertex_shader_path = shader_dir / ("deferred_geometry.vert." OGRAPHICS_SHADER_FORMAT);
            const std::filesystem::path fragment_shader_path = shader_dir / ("deferred_geometry.frag." OGRAPHICS_SHADER_FORMAT);
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

            // Inline constants
            ogfx::InlineConstantRange inline_constant_range;
            inline_constant_range.size = sizeof(DrawConstants);
            inline_constant_range.stages = ogfx::ShaderStageFlags::Vertex;
            desc.inline_constants_ranges.push_back(inline_constant_range);

            // Vertex input
            ogfx::VertexBindingDesc vertex_binding;
            vertex_binding.binding = 0;
            vertex_binding.stride = sizeof(Vertex);
            vertex_binding.input_rate = ogfx::VertexInputRate::Vertex;

            desc.vertex_input.bindings = {vertex_binding};

            ogfx::VertexAttributeDesc position_attribute;
            position_attribute.location = 0;
            position_attribute.binding = 0;
            position_attribute.format = ogfx::Format::RGBA32_FLOAT;
            position_attribute.offset = offsetof(Vertex, position);

            ogfx::VertexAttributeDesc normal_attribute;
            normal_attribute.location = 1;
            normal_attribute.binding = 0;
            normal_attribute.format = ogfx::Format::RGBA32_FLOAT;
            normal_attribute.offset = offsetof(Vertex, normal);

            desc.vertex_input.attributes = {position_attribute, normal_attribute};

            // Input assembly
            desc.input_assembly.topology = ogfx::PrimitiveTopology::TriangleList;
            desc.input_assembly.primitive_restart_enable = false;

            // Viewport
            desc.viewport.dynamic_viewport = false;
            desc.viewport.dynamic_scissor = false;

            ogfx::Viewport viewport;
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(m_desc.swapchain_width);
            viewport.height = static_cast<float>(m_desc.swapchain_height);
            viewport.min_depth = 0.0f;
            viewport.max_depth = 1.0f;

            desc.viewport.viewports = {viewport};

            ogfx::Scissor scissor;
            scissor.x = 0;
            scissor.y = 0;
            scissor.width = m_desc.swapchain_width;
            scissor.height = m_desc.swapchain_height;

            desc.viewport.scissors = {scissor};

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
            ogfx::ColorBlendAttachmentDesc albedo_blend;
            albedo_blend.blend_enable = false;
            albedo_blend.write_mask = ogfx::ColorComponent::All;

            ogfx::ColorBlendAttachmentDesc normal_blend;
            normal_blend.blend_enable = false;
            normal_blend.write_mask = ogfx::ColorComponent::All;

            desc.color_blend.attachments = {albedo_blend, normal_blend};

            // Runtime render settings
            desc.width = m_desc.swapchain_width;
            desc.height = m_desc.swapchain_height;
            desc.clear_values = 
            {
                // Albedo
                {
                    .type = ogfx::ClearValueType::Color,
                    .color = {0.0f, 0.0f, 0.0f, 1.0f}
                },

                // Normal
                {
                    .type = ogfx::ClearValueType::Color,
                    .color = {0.0f, 0.0f, 0.0f, 0.0f}
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

    geometry_node->set_execute
    (
        [this](orendergraph::RenderGraph& rg, ogfx::CommandBuffer& cmd,
            orendergraph::RenderGraphNode<orendergraph::RasterContext>& node)
        {
            orendergraph::RasterContext& ctx = node.context();

            // Bind vertex buffer
            ogfx::VertexBufferBindInfo vertex_buffer_bind_info;
            vertex_buffer_bind_info.buffer = rg.get_resource(m_desc.vertex_buffer);
            vertex_buffer_bind_info.binding = 0;
            vertex_buffer_bind_info.offset = 0;

            cmd.bind_vertex_buffer(vertex_buffer_bind_info);

            // Bind index buffer
            ogfx::IndexBufferBindInfo index_buffer_bind_info;
            index_buffer_bind_info.buffer = rg.get_resource(m_desc.index_buffer);
            index_buffer_bind_info.offset = 0;
            index_buffer_bind_info.index_type = ogfx::IndexType::UInt32;

            cmd.bind_index_buffer(index_buffer_bind_info);

            // Draw each instance
            for (std::size_t i = 0; i < m_desc.clusters->size(); ++i)
            {
                const MeshCluster& cluster = (*m_desc.clusters)[i];

                DrawConstants constants{};
                
                const float model[16] = 
                {
                    1.0f, 0.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, 0.0f,
                    0.0f, 0.0f, 0.0f, 1.0f
                };
                
                const float view[16] = 
                {
                    1.0f, 0.0f,  0.0f, 0.0f,
                    0.0f, 1.0f,  0.0f, 0.0f,
                    0.0f, 0.0f,  1.0f, 0.0f,
                    0.0f, 0.0f, -5.0f, 1.0f
                };
                
                const float fov = 1.04719755f;
                const float aspect = 16.0f / 9.0f;
                const float near_plane = 0.1f;
                const float far_plane = 100.0f;
                
                const float f = 1.0f / std::tan(fov * 0.5f);
                
                const float projection[16] = 
                {
                    f / aspect, 0.0f,                                                0.0f,  0.0f,
                          0.0f,   -f,                                                0.0f,  0.0f,
                          0.0f, 0.0f,                far_plane / (near_plane - far_plane), -1.0f,
                          0.0f, 0.0f, (near_plane * far_plane) / (near_plane - far_plane), 0.0f
                };
                
                std::copy(std::begin(model), std::end(model), constants.model);
                std::copy(std::begin(view), std::end(view), constants.view);
                std::copy(std::begin(projection), std::end(projection), constants.projection);
                std::copy(std::begin(cluster.color), std::end(cluster.color), constants.color);

                ogfx::InlineConstantsInfo constants_info;
                constants_info.layout = ctx.pipeline_layout;
                constants_info.stages = ogfx::ShaderStageFlags::Vertex;
                constants_info.offset = 0;
                constants_info.size = sizeof(DrawConstants);
                constants_info.data = &constants;

                cmd.set_inline_constants(constants_info);

                ogfx::DrawIndexedInfo draw_info;
                draw_info.index_count = cluster.index_count;
                draw_info.first_index = cluster.first_index;

                cmd.draw_indexed(draw_info);
            }
        }
    );
}

void DeferredLighting::add_lighting_node(orendergraph::RenderGraph& rg)
{
    // Lighting pass
    auto lighting_node = rg.add_node<orendergraph::ComputeContext>
    (
        "Deferred Lighting",
        [this](orendergraph::RenderGraph& rg, orendergraph::ComputeContext::Desc& desc)
        {
            // Dependencies
            rg.read(m_albedo_image, ogfx::ImageState::ShaderRead, {ogfx::ImageAspect::Color});
            rg.read(m_normal_image, ogfx::ImageState::ShaderRead, {ogfx::ImageAspect::Color});
            rg.read(m_depth_image, ogfx::ImageState::ShaderRead, {ogfx::ImageAspect::Depth});
            rg.write(m_lighting_image, ogfx::ImageState::Common, {ogfx::ImageAspect::Color});

            // Shaders
            const std::filesystem::path shader_dir = "shaders/";
            const std::filesystem::path compute_shader_path = shader_dir / ("deferred_lighting.comp." OGRAPHICS_SHADER_FORMAT);
            const std::vector<uint8_t> compute_bytecode = load_shader(compute_shader_path);

            ogfx::ShaderDesc compute_shader_desc;
            compute_shader_desc.stage = ogfx::ShaderStage::Compute;
            compute_shader_desc.entry_point = "CSMain";
            compute_shader_desc.bytecode = compute_bytecode;

            desc.compute_shader_desc = compute_shader_desc;

            // Resource set bindings
            ogfx::ResourceSetLayoutDesc set_layout_desc;

            for (uint32_t binding = 0; binding < 3; ++binding)
            {
                ogfx::ResourceBindingDesc texture_binding;
                texture_binding.binding = binding;
                texture_binding.type = ogfx::ResourceType::SampledTexture;
                texture_binding.stages = ogfx::ShaderStageFlags::Compute;

                set_layout_desc.bindings.push_back(texture_binding);
            }

            ogfx::ResourceBindingDesc output_binding;
            output_binding.binding = 3;
            output_binding.type = ogfx::ResourceType::StorageTexture;
            output_binding.array_count = 1;
            output_binding.stages = ogfx::ShaderStageFlags::Compute;

            set_layout_desc.bindings.push_back(output_binding);

            desc.resource_set_layouts_desc.push_back(set_layout_desc);

            // Inline constants
            ogfx::InlineConstantRange constants_range;
            constants_range.stages = ogfx::ShaderStageFlags::Compute;
            constants_range.offset = 0;
            constants_range.size = sizeof(LightingConstants);

            desc.inline_constants_ranges.push_back(constants_range);

            // Resource set
            orendergraph::ResourceSetDesc resource_set_desc;
            resource_set_desc.resource_set_layout_idx = 0;

            desc.resource_sets_desc.push_back(resource_set_desc);

            // Resource set writes
            orendergraph::ResourceSetWriteInfo write_info;

            orendergraph::ImageBinding albedo_binding;
            albedo_binding.binding = 0;
            albedo_binding.image_view = m_albedo_image_view;
            albedo_binding.image_state = ogfx::ImageState::ShaderRead;

            orendergraph::ImageBinding normal_binding;
            normal_binding.binding = 1;
            normal_binding.image_view = m_normal_image_view;
            normal_binding.image_state = ogfx::ImageState::ShaderRead;

            orendergraph::ImageBinding depth_binding;
            depth_binding.binding = 2;
            depth_binding.image_view = m_depth_image_view;
            depth_binding.image_state = ogfx::ImageState::ShaderRead;

            orendergraph::ImageBinding lighting_binding;
            lighting_binding.binding = 3;
            lighting_binding.image_view = m_lighting_image_view;
            lighting_binding.image_state = ogfx::ImageState::Common;

            write_info.textures = {albedo_binding, normal_binding, depth_binding, lighting_binding};

            orendergraph::ResourceWriteDesc resource_write_desc;
            resource_write_desc.resource_set_idx = 0;
            resource_write_desc.write_info = write_info;

            desc.resource_writes_desc.push_back(resource_write_desc);

            // Dispatch dimensions
            constexpr uint32_t workgroup_size_x = 8;
            constexpr uint32_t workgroup_size_y = 8;
            desc.group_count_x = (m_desc.swapchain_width + workgroup_size_x - 1) / workgroup_size_x;
            desc.group_count_y = (m_desc.swapchain_height + workgroup_size_y - 1) / workgroup_size_y;
            desc.group_count_z = 1;
        }
    );

    lighting_node->set_execute
    (
        [this](orendergraph::RenderGraph& rg, ogfx::CommandBuffer& cmd,
            orendergraph::RenderGraphNode<orendergraph::ComputeContext>& node)
        {
            orendergraph::ComputeContext& ctx = node.context();
            orendergraph::ComputeContext::Desc& desc = node.descriptor();

            ogfx::ResourceSetBindInfo bind_info;
            bind_info.layout = ctx.pipeline_layout;
            bind_info.resource_set = ctx.resource_sets[0];
            bind_info.set_index = 0;
            bind_info.pipeline_type = ogfx::PipelineType::Compute;

            cmd.bind_resource_set(bind_info);

            LightingConstants constants{};

            constants.width = m_desc.swapchain_width;
            constants.height = m_desc.swapchain_height;
            constants.ambient_strength = 0.1f;

            constants.light_direction[0] = -0.5f;
            constants.light_direction[1] = -1.0f;
            constants.light_direction[2] = -0.5f;

            constants.light_color[0] = 1.0f;
            constants.light_color[1] = 1.0f;
            constants.light_color[2] = 1.0f;
            constants.light_color[3] = 1.0f;

            ogfx::InlineConstantsInfo constants_info;
            constants_info.layout = ctx.pipeline_layout;
            constants_info.stages = ogfx::ShaderStageFlags::Compute;
            constants_info.size = sizeof(LightingConstants);
            constants_info.data = &constants;

            cmd.set_inline_constants(constants_info);

            cmd.dispatch({desc.group_count_x, desc.group_count_y, desc.group_count_z});
        }
    );
}
