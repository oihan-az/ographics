#include <omega/graphics/pipelines/raster_pipeline/features/geometry_feature.hpp>

#include <orenderer/render_feature_context.hpp>
#include <orendergraph/render_graph.hpp>
#include <ogfx/image.hpp>
#include <ogfx/image_view.hpp>

#include <omega/core/math/matrix4.hpp>
#include <omega/core/math/vector4.hpp>
#include <omega/core/math/vector3.hpp>
#include <omega/loader/loader.hpp>

#include <omega/core/component/component_system.hpp>
#include <omega/transform/transform_system.hpp>
#include <omega/camera/camera_system.hpp>
#include <omega/mesh/gpu_mesh_system.hpp>

#include <omega/renderable/renderable.hpp>

#include <filesystem>

namespace omega
{

struct DrawConstants
{
    Matrix4 model;
    Matrix4 view_projection;
    Vector4 color;
};

GeometryRenderFeature::GeometryRenderFeature(const GeometryRenderFeatureDesc& desc) : m_desc(desc) 
{}

void GeometryRenderFeature::build(orenderer::RenderFeatureContext& ctx) 
{
    orendergraph::RenderGraph& graph = ctx.render_graph();

    create_resources(graph);

    add_node(graph);
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

            // Inline constants
            ogfx::InlineConstantRange inline_constant_range;
            inline_constant_range.size = sizeof(DrawConstants);
            inline_constant_range.stages = ogfx::ShaderStageFlags::Vertex | ogfx::ShaderStageFlags::Fragment;
            desc.inline_constants_ranges.push_back(inline_constant_range);

            // Vertex input
            ogfx::VertexBindingDesc vertex_binding;
            vertex_binding.binding = 0;
            vertex_binding.stride = sizeof(Vector3);
            vertex_binding.input_rate = ogfx::VertexInputRate::Vertex;

            desc.vertex_input.bindings = {vertex_binding};

            ogfx::VertexAttributeDesc position_attribute{};
            position_attribute.location = 0;
            position_attribute.binding = 0;
            position_attribute.format = ogfx::Format::RGB32_FLOAT;
            position_attribute.offset = 0;

            desc.vertex_input.attributes = {position_attribute};

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

    geometry_node->set_execute
    (
        [this](orendergraph::RenderGraph& rg, ogfx::CommandBuffer& cmd,
            orendergraph::RenderGraphNode<orendergraph::RasterContext>& node)
        {
            orendergraph::RasterContext& ctx = node.context();

            // Camera
            const CameraData camera = m_desc.cameras.get_camera_data(m_desc.camera);

            // Renderables
            const std::vector<Entity> entities = m_desc.components.entities_with<Renderable>();

            for (Entity entity : entities)
            {
                const Renderable* renderable = m_desc.components.get<Renderable>(entity);

                // GPU mesh
                if (!m_desc.gpu_meshes.is_resident(renderable->mesh))
                    continue;

                const MeshData* mesh = m_desc.gpu_meshes.get(renderable->mesh);
                if (!mesh)
                    continue;

                // GPU layout group
                const GpuMeshLayoutGroup* group = m_desc.gpu_meshes.layout_group(mesh->layout);
                if (!group)
                    continue;

                // GPU page
                const GpuMeshPage* page = group->page(mesh->allocation.page_index);
                if (!page)
                    continue;

                // Vertex buffer
                {
                    // Binding 0
                    const ogfx::Buffer* vertex_buffer = page->vertex_buffer(0);
                    if (!vertex_buffer)
                        continue;

                    const VertexLayout* layout = m_desc.gpu_meshes.vertex_layout(mesh->layout);
                    if (!layout || layout->desc.bindings.empty())
                        continue;

                    const uint32_t vertex_stride = layout->desc.bindings[0].stride;
                    const uint64_t vertex_offset = static_cast<uint64_t>(mesh->allocation.allocation.vertex_offsets[0]) * vertex_stride;

                    ogfx::VertexBufferBindInfo vertex_bind_info{};
                    vertex_bind_info.buffer = vertex_buffer;
                    vertex_bind_info.binding = 0;
                    vertex_bind_info.offset = vertex_offset;

                    cmd.bind_vertex_buffer(vertex_bind_info);
                }

                // Index buffer
                if (mesh->is_indexed())
                {
                    const ogfx::Buffer* index_buffer = page->index_buffer();
                    if (!index_buffer)
                        continue;

                    ogfx::IndexBufferBindInfo index_bind_info{};
                    index_bind_info.buffer = index_buffer;
                    index_bind_info.offset = static_cast<uint64_t>(mesh->allocation.allocation.index_offset) * sizeof(uint32_t);
                    index_bind_info.index_type = ogfx::IndexType::UInt32;

                    cmd.bind_index_buffer(index_bind_info);
                }

                // Draw constants
                DrawConstants constants{};
                constants.model = m_desc.transforms.get_world_matrix(entity);
                constants.view_projection = camera.view_projection;
                constants.color = Vector4(renderable->color.x, renderable->color.y, renderable->color.z, 1.0f);

                ogfx::InlineConstantsInfo constants_info{};
                constants_info.layout = ctx.pipeline_layout;
                constants_info.stages = ogfx::ShaderStageFlags::Vertex | ogfx::ShaderStageFlags::Fragment;
                constants_info.offset = 0;
                constants_info.size = sizeof(DrawConstants);
                constants_info.data = &constants;

                cmd.set_inline_constants(constants_info);

                // Draw
                if (mesh->is_indexed())
                {
                    ogfx::DrawIndexedInfo draw_info{};
                    draw_info.index_count = mesh->index_count;
                    draw_info.first_index = 0;

                    cmd.draw_indexed(draw_info);
                }
                else
                {
                    ogfx::DrawInfo draw_info{};
                    draw_info.vertex_count = mesh->vertex_count;
                    draw_info.first_vertex = 0;

                    cmd.draw(draw_info);
                }
            }
        }
    );
}

}