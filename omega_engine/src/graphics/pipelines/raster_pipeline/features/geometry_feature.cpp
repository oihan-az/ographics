#include <omega/graphics/pipelines/raster_pipeline/features/geometry_feature.hpp>

#include <orenderer/render_feature_context.hpp>
#include <orendergraph/render_graph.hpp>
#include <ogfx/image.hpp>
#include <ogfx/image_view.hpp>
#include <ogfx/resource_allocator.hpp>

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

#include <omega/renderable/renderable.hpp>
#include <omega/lighting/gpu_light_data.hpp>

#include <filesystem>

namespace omega
{

struct DrawConstants
{
    Matrix4 model;
    Matrix4 view_projection;
    Vector4 color;
};

struct LightingConstants
{
    uint32_t directional_light_count = 0;
    uint32_t point_light_count = 0;
    uint32_t spot_light_count = 0;
    uint32_t padding = 0;
};

GeometryRenderFeature::GeometryRenderFeature(const GeometryRenderFeatureDesc& desc)
    : m_desc(desc), m_directional_lights(desc.components, desc.events, desc.frames_in_flight),
      m_point_lights(desc.components, desc.events, desc.frames_in_flight),
      m_spot_lights(desc.components, desc.events, desc.frames_in_flight)
{}

void GeometryRenderFeature::update_texture_resource_set(FrameResourceSets& frame, TextureHandle texture) 
{
    auto view_it = m_texture_views.find(texture);
    if (view_it == m_texture_views.end())
        return;

    const ogfx::ImageView* image_view = m_desc.gpu_textures.get_view(view_it->second);
    if (!image_view)
    {
        frame.pending_textures.insert(texture);
        return;
    }

    ogfx::ImageBinding texture_binding{};
    texture_binding.binding = 0;
    texture_binding.image_state = ogfx::ImageState::ShaderRead;
    texture_binding.image_view = image_view;

    ogfx::ResourceSetWriteInfo write_info{};
    write_info.textures.push_back(texture_binding);

    frame.m_texture_resource_sets->update(texture, write_info);
    frame.pending_textures.erase(texture);
}

void GeometryRenderFeature::build(orenderer::RenderFeatureContext& ctx) 
{
    orendergraph::RenderGraph& rg = ctx.render_graph();

    m_frames.resize(m_desc.frames_in_flight);
    m_directional_lights.create(rg);
    m_point_lights.create(rg);
    m_spot_lights.create(rg);

    constexpr uint32_t light_type_count = 3;

    ogfx::DescriptorPoolDesc pool_desc;
    pool_desc.max_sets = m_desc.frames_in_flight + 1;
    pool_desc.sizes = 
    {
        {ogfx::ResourceType::StorageBuffer, m_desc.frames_in_flight * light_type_count},
        {ogfx::ResourceType::Sampler, 1}
    };
    rg.get_resource_allocator().create_pool(pool_desc);

    create_resources(rg);
    add_node(rg);
}

void GeometryRenderFeature::create_resources(orendergraph::RenderGraph& rg)
{
    // Output image
    {
        ogfx::ImageDesc image_desc{};
        image_desc.width = m_desc.width;
        image_desc.height = m_desc.height;
        image_desc.format = ogfx::Format::RGBA8_UNORM;
        image_desc.usage = ogfx::ImageUsage::ColorAttachment | ogfx::ImageUsage::TransferSrc;
        m_out_image = rg.create_resource<ogfx::Image>(image_desc);

        ogfx::ImageViewDesc image_view_desc{};
        image_view_desc.image = rg.get_resource(m_out_image);
        image_view_desc.aspect = ogfx::ImageAspect::Color;
        m_out_image_view = rg.create_resource<ogfx::ImageView>(image_view_desc);
    }

    // Depth
    {
        ogfx::ImageDesc desc{};
        desc.width = m_desc.width;
        desc.height = m_desc.height;
        desc.format = ogfx::Format::D32_FLOAT;
        desc.usage = ogfx::ImageUsage::DepthStencil;
        m_depth_image = rg.create_resource<ogfx::Image>(desc);

        ogfx::ImageViewDesc image_view_desc{};
        image_view_desc.image = rg.get_resource(m_depth_image);
        image_view_desc.aspect = ogfx::ImageAspect::Depth;
        m_depth_image_view = rg.create_resource<ogfx::ImageView>(image_view_desc);
    }

    // Sampler
    {
        ogfx::SamplerDesc desc{};
        m_sampler = rg.create_resource<ogfx::Sampler>(desc);
    }
}

void GeometryRenderFeature::add_node(orendergraph::RenderGraph& rg)
{
    // Geometry pass
    auto geometry_node = rg.add_node<orendergraph::RasterContext>
    (
        "Geometry",
        [this](orendergraph::RenderGraph& rg, orendergraph::RasterContext::Desc& desc)
        {
            // Dependencies
            rg.write(m_out_image, ogfx::ImageState::RenderTarget, {ogfx::ImageAspect::Color}, ogfx::ImageState::TransferSrc, true );
            rg.write(m_depth_image, ogfx::ImageState::DepthStencil, {ogfx::ImageAspect::Depth}, ogfx::ImageState::Undefined, true);

            // Render pass
            ogfx::RenderPassAttachmentDesc color_attachment;
            color_attachment.format = ogfx::Format::RGBA8_UNORM;
            color_attachment.load_op = ogfx::LoadOp::Clear;
            color_attachment.store_op = ogfx::StoreOp::Store;
            color_attachment.initial_state = ogfx::ImageState::Undefined;
            color_attachment.final_state = ogfx::ImageState::TransferSrc;
            color_attachment.image_aspect = ogfx::ImageAspect::Color;

            ogfx::RenderPassAttachmentDesc depth_attachment;
            depth_attachment.format = ogfx::Format::D32_FLOAT;
            depth_attachment.load_op = ogfx::LoadOp::Clear;
            depth_attachment.store_op = ogfx::StoreOp::Store;
            depth_attachment.initial_state = ogfx::ImageState::Undefined;
            depth_attachment.final_state = ogfx::ImageState::DepthStencil;
            depth_attachment.image_aspect = ogfx::ImageAspect::Depth;

            desc.render_pass_desc.attachments = {color_attachment, depth_attachment};

            // Framebuffer
            ogfx::FramebufferDesc framebuffer_desc;
            framebuffer_desc.attachments = 
            {
                rg.get_resource(m_out_image_view),                             
                rg.get_resource(m_depth_image_view)
            };
            framebuffer_desc.width = m_desc.width;
            framebuffer_desc.height = m_desc.height;
            desc.framebuffers_desc = {framebuffer_desc};

            // Shaders
            const std::filesystem::path shader_dir = "shaders/";
            const std::filesystem::path vertex_shader_path = shader_dir / ("geometry.vert." OGRAPHICS_SHADER_FORMAT);
            const std::filesystem::path fragment_shader_path = shader_dir / ("geometry.frag." OGRAPHICS_SHADER_FORMAT);
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
            viewport.width = static_cast<float>(m_desc.width);
            viewport.height = static_cast<float>(m_desc.height);
            viewport.min_depth = 0.0f;
            viewport.max_depth = 1.0f;

            desc.viewport.viewports = {viewport};

            ogfx::Scissor scissor;
            scissor.x = 0;
            scissor.y = 0;
            scissor.width = m_desc.width;
            scissor.height = m_desc.height;

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
            fragment_constants_range.size = sizeof(LightingConstants);
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

            ogfx::ResourceSetLayoutDesc texture_set_layout_desc;

            ogfx::ResourceBindingDesc texture_binding;
            texture_binding.binding = 0;
            texture_binding.type = ogfx::ResourceType::SampledTexture;
            texture_binding.stages = ogfx::ShaderStageFlags::Fragment;
            texture_set_layout_desc.bindings.push_back(texture_binding);

            desc.resource_set_layouts_desc.push_back(texture_set_layout_desc);

            ogfx::ResourceSetLayoutDesc sampler_set_layout_desc;

            ogfx::ResourceBindingDesc sampler_binding;
            sampler_binding.binding = 0;
            sampler_binding.type = ogfx::ResourceType::Sampler;
            sampler_binding.stages = ogfx::ShaderStageFlags::Fragment;
            sampler_set_layout_desc.bindings.push_back(sampler_binding);

            desc.resource_set_layouts_desc.push_back(sampler_set_layout_desc);

            // One resource set and one write description per frame
            for (uint32_t i = 0; i < m_frames.size(); ++i)
            {
                FrameResource& frame = m_frames[i];

                orendergraph::ResourceSetDesc resource_set_desc;
                resource_set_desc.resource_set_layout_idx = 0;

                desc.resource_sets_desc.push_back(resource_set_desc);

                orendergraph::BufferBinding directional_binding;
                directional_binding.binding = 0;
                directional_binding.buffer = m_directional_lights.get_buffer(i).get_handle();
                directional_binding.offset = 0;
                directional_binding.range = m_directional_lights.get_buffer(i).get_capacity();

                orendergraph::BufferBinding point_binding;
                point_binding.binding = 1;
                point_binding.buffer = m_point_lights.get_buffer(i).get_handle();
                point_binding.offset = 0;
                point_binding.range = m_point_lights.get_buffer(i).get_capacity();

                orendergraph::BufferBinding spot_binding;
                spot_binding.binding = 2;
                spot_binding.buffer = m_spot_lights.get_buffer(i).get_handle();
                spot_binding.offset = 0;
                spot_binding.range = m_spot_lights.get_buffer(i).get_capacity();

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
            write_desc.resource_set_idx = m_desc.frames_in_flight;
            write_desc.write_info.samplers = {resoruce_sampler_binding};

            desc.resource_writes_desc.push_back(write_desc);

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
            desc.width = m_desc.width;
            desc.height = m_desc.height;
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

    auto& ctx = geometry_node->context();
    m_resource_sets.resize(m_desc.frames_in_flight);
    for (FrameResourceSets& frame : m_resource_sets)
    {
        frame.m_texture_resource_sets = std::make_unique<ResourceSetCache<TextureHandle, TextureHandleHash>>(rg.device(), rg.get_resource_allocator(), *ctx.resource_set_layouts[1]);
    }

    geometry_node->set_pre_execute
    (
        [this](orendergraph::RenderGraph& rg, ogfx::CommandBuffer& cmd,
            orendergraph::RenderGraphNode<orendergraph::RasterContext>& node)
        {
            const uint32_t frame_index = *m_desc.current_frame;

            // Update light buffers
            {
                m_directional_lights.capture_events();
                m_point_lights.capture_events();
                m_spot_lights.capture_events();

                if (m_directional_lights.process_frame(frame_index, cmd, rg, m_desc.transforms))
                {
                    update_resource_set(m_directional_lights.get_buffer(frame_index),
                                        m_frames[frame_index].directional_buffer_write, rg, node);
                }

                if (m_point_lights.process_frame(frame_index, cmd, rg, m_desc.transforms))
                {
                    update_resource_set(m_directional_lights.get_buffer(frame_index),
                                        m_frames[frame_index].point_buffer_write, rg, node);
                }

                if (m_spot_lights.process_frame(frame_index, cmd, rg, m_desc.transforms))
                {
                    update_resource_set(m_directional_lights.get_buffer(frame_index),
                                        m_frames[frame_index].spot_buffer_write, rg, node);
                }
            }

            // Update texture resource sets
            {
                const auto& added = m_desc.events.get_queue<ComponentAddedEvent<Renderable>>().get_events();
                const auto& removed = m_desc.events.get_queue<ComponentRemovedEvent<Renderable>>().get_events();
                const auto& modified = m_desc.events.get_queue<ComponentModifiedEvent<Renderable>>().get_events();

                for (FrameResourceSets& frame : m_resource_sets)
                {
                    frame.pending_added.insert(frame.pending_added.end(), added.begin(), added.end());
                    frame.pending_removed.insert(frame.pending_removed.end(), removed.begin(), removed.end());
                    frame.pending_modified.insert(frame.pending_modified.end(), modified.begin(), modified.end());
                }

                FrameResourceSets& resource_sets = m_resource_sets[frame_index];
                auto texture_resource_sets = resource_sets.m_texture_resource_sets.get();
                for (const auto& event : resource_sets.pending_added)
                {
                    const Renderable* renderable = m_desc.components.get<Renderable>(event.entity);
                    if (!renderable)
                        continue;

                    const TextureHandle texture = renderable->texture;
                    if (!texture_resource_sets->acquire(texture))
                        continue;

                    // Request residency if necessary
                    if (!m_desc.gpu_textures.is_resident(texture) && !m_desc.gpu_textures.is_pending(texture) &&
                        !m_desc.gpu_textures.is_uploading(texture))
                    {
                        m_desc.gpu_textures.make_resident(texture);
                    }

                    // Request a view if one doesn't already exist
                    if (!m_texture_views.contains(texture))
                    {
                        const TextureViewHandle view = m_desc.gpu_textures.create_view(texture, TextureViewDesc{});
                        if (view.is_valid())
                            m_texture_views.emplace(texture, view);
                    }

                    update_texture_resource_set(resource_sets, texture);
                }

                for (const auto& event : resource_sets.pending_modified)
                {
                    const Renderable* renderable = m_desc.components.get<Renderable>(event.entity);
                    if (!renderable)
                        continue;

                    const TextureHandle texture = renderable->texture;
                    if (!texture_resource_sets->acquire(texture))
                        continue;

                    // Request residency if necessary
                    if (!m_desc.gpu_textures.is_resident(texture) && !m_desc.gpu_textures.is_pending(texture) &&
                        !m_desc.gpu_textures.is_uploading(texture))
                    {
                        m_desc.gpu_textures.make_resident(texture);
                    }

                    // Request a view if one doesn't already exist
                    if (!m_texture_views.contains(texture))
                    {
                        const TextureViewHandle view = m_desc.gpu_textures.create_view(texture, TextureViewDesc{});
                        if (view.is_valid())
                            m_texture_views.emplace(texture, view);
                    }

                    update_texture_resource_set(resource_sets, texture);
                }

                for (const auto& event : resource_sets.pending_removed)
                {
                    const Renderable* renderable = m_desc.components.get<Renderable>(event.entity);
                    if (!renderable)
                        continue;

                    const TextureHandle texture = renderable->texture;
                    if (!texture_resource_sets->acquire(texture))
                        continue;

                    if (!texture_resource_sets->release(texture))
                        continue;

                    resource_sets.pending_textures.erase(texture);

                    bool has_references = false;
                    for (const FrameResourceSets& frame : m_resource_sets)
                    {
                        if (frame.m_texture_resource_sets->reference_count(texture) > 0)
                        {
                            has_references = true;
                            break;
                        }
                    }

                    if (!has_references)
                    {
                        const auto view_it = m_texture_views.find(texture);
                        if (view_it != m_texture_views.end())
                        {
                            m_desc.gpu_textures.release_view(view_it->second);
                            m_texture_views.erase(view_it);
                        }
                    }
                }

                const std::vector<TextureHandle> pending(resource_sets.pending_textures.begin(), resource_sets.pending_textures.end());
                for (TextureHandle texture : pending)
                {
                    update_texture_resource_set(resource_sets, texture);
                }

                resource_sets.pending_added.clear();
                resource_sets.pending_removed.clear();
                resource_sets.pending_modified.clear();
            }

            orendergraph::NodePreExecute<orendergraph::RasterContext>::execute(rg, cmd, node);
        }
    );

    geometry_node->set_execute
    (
        [this](orendergraph::RenderGraph& rg, ogfx::CommandBuffer& cmd,
            orendergraph::RenderGraphNode<orendergraph::RasterContext>& node)
        {
            orendergraph::RasterContext& ctx = node.context();

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
                resource_set_info.resource_set = ctx.resource_sets[*m_desc.current_frame];

                cmd.bind_resource_set(resource_set_info);
            }

            // Resource sets
            {
                ogfx::ResourceSetBindInfo resource_set_info{};
                resource_set_info.layout = ctx.pipeline_layout;
                resource_set_info.pipeline_type = ogfx::PipelineType::Graphics;
                resource_set_info.set_index = 2;
                resource_set_info.resource_set = ctx.resource_sets[m_desc.frames_in_flight];

                cmd.bind_resource_set(resource_set_info);
            }

            // Camera
            const CameraData camera = m_desc.cameras.get_camera_data(m_desc.camera);

            // Renderables
            const std::vector<Entity> entities = m_desc.components.entities_with<Renderable>();
            for (Entity entity : entities)
            {
                const Renderable* renderable = m_desc.components.get<Renderable>(entity);

                // GPU mesh
                if (!m_desc.gpu_meshes.is_resident(renderable->mesh))
                {
                    m_desc.gpu_meshes.make_resident(renderable->mesh);
                    continue;
                }

                // Draw data
                const GpuMeshDrawData draw_data = m_desc.gpu_meshes.get_draw_data(renderable->mesh);
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
                constants.color = Vector4(renderable->color.x, renderable->color.y, renderable->color.z, 1.0f);

                ogfx::InlineConstantsInfo constants_info{};
                constants_info.layout = ctx.pipeline_layout;
                constants_info.stages = ogfx::ShaderStageFlags::Vertex;
                constants_info.offset = 0;
                constants_info.size = sizeof(DrawConstants);
                constants_info.data = &constants;

                cmd.set_inline_constants(constants_info);

                // Texture
                if (m_resource_sets[*m_desc.current_frame].pending_textures.contains(renderable->texture))
                    continue;

                const ogfx::ResourceSet* texture_resource_set = m_resource_sets[*m_desc.current_frame].m_texture_resource_sets->get(renderable->texture);
                if (!texture_resource_set)
                    continue;

                ogfx::ResourceSetBindInfo texture_resource_set_info{};
                texture_resource_set_info.layout = ctx.pipeline_layout;
                texture_resource_set_info.pipeline_type = ogfx::PipelineType::Graphics;
                texture_resource_set_info.set_index = 1;
                texture_resource_set_info.resource_set = const_cast<ogfx::ResourceSet*>(texture_resource_set);

                cmd.bind_resource_set(texture_resource_set_info);

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

void GeometryRenderFeature::update_resource_set(DynamicBuffer& buffer, orendergraph::ResourceWriteDesc& write_desc,
                                                orendergraph::RenderGraph& rg,
                                                orendergraph::RenderGraphNode<orendergraph::RasterContext>& node)
{
    write_desc.write_info.buffers[0].range = buffer.get_capacity();
    node.context().update_resource_set(rg, write_desc);
}

}