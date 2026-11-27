#include <omega/graphics/pipelines/raster_pipeline/features/shadow_map_gen.hpp>

#include <orenderer/render_feature_context.hpp>
#include <orendergraph/render_graph.hpp>
#include <orendergraph/node_context/raster_context.hpp>

#include <ogfx/command_buffer.hpp>
#include <ogfx/image.hpp>
#include <ogfx/image_view.hpp>
#include <ogfx/commands/barrier_commands.hpp>
#include <ogfx/commands/render_pass_commands.hpp>
#include <ogfx/commands/binding_commands.hpp>

#include <omega/core/component/component_system.hpp>
#include <omega/core/math/matrix4.hpp>
#include <omega/transform/transform_system.hpp>
#include <omega/transform/transform.hpp>
#include <omega/graphics/graphics_system.hpp>
#include <omega/mesh/gpu_mesh_system.hpp>
#include <omega/mesh/vertex.hpp>
#include <omega/renderable/pbr_opaque_renderable.hpp>
#include <omega/loader/loader.hpp>

#include <omega/graphics/resources/gpu_shadow_data.hpp>

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <numbers>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace omega
{
namespace
{

struct ShadowDrawConstants
{
    Matrix4 model;
    Matrix4 light_view_projection;
};

uint32_t sanitize_resolution(uint32_t value, uint32_t minimum, uint32_t maximum)
{
    return std::clamp(std::bit_floor(value), minimum, maximum);
}

void transition_depth_image(ogfx::CommandBuffer& cmd, ogfx::Image* image,
                            ogfx::ImageState before, ogfx::ImageState after,
                            uint32_t layer_count)
{
    if (before == after)
        return;

    ogfx::ImageTransitionInfo info{};
    info.image = image;
    info.old_state = before;
    info.new_state = after;
    info.aspect = ogfx::ImageAspect::Depth;
    info.base_mip_level = 0;
    info.mip_level_count = 1;
    info.base_array_layer = 0;
    info.array_layer_count = layer_count;
    cmd.transition_image(info);
}

} // namespace

ShadowMapGen::ShadowMapGen(const ShadowMapGenDesc& desc)
    : m_desc(desc),
      m_directional_events(desc.graphics.max_frames_in_flight()),
      m_spot_events(desc.graphics.max_frames_in_flight()),
      m_point_events(desc.graphics.max_frames_in_flight())
{
}

void ShadowMapGen::build(orenderer::RenderFeatureContext& ctx)
{
    orendergraph::RenderGraph& rg = ctx.render_graph();
    add_node(rg);

    // Persistent fallback shadow maps
    m_fallback_2d = create_shadow_map(rg, 1, false);
    m_fallback_cube = create_shadow_map(rg, 1, true);

    create_existing_maps(rg);
}

const ShadowMapGen::ShadowRegistry& ShadowMapGen::shadow_registry(ShadowMapType type) const
{
    switch (type)
    {
    case ShadowMapType::Directional: return m_directional_maps;
    case ShadowMapType::Spot:        return m_spot_maps;
    case ShadowMapType::Point:       return m_point_maps;
    }
    throw std::runtime_error("ShadowMapGen: Invalid shadow map type");
}

orendergraph::ResourceHandle<ogfx::Image>
ShadowMapGen::image(ShadowMapType type, Entity light) const
{
    const auto& maps = shadow_registry(type);
    const auto it = maps.find(light);
    return it == maps.end() ? orendergraph::ResourceHandle<ogfx::Image>{} : it->second.image;
}

orendergraph::ResourceHandle<ogfx::ImageView>
ShadowMapGen::sampled_view(ShadowMapType type, Entity light) const
{
    const auto& maps = shadow_registry(type);
    const auto it = maps.find(light);
    return it == maps.end() ? orendergraph::ResourceHandle<ogfx::ImageView>{} : it->second.sampled_view;
}

orendergraph::ResourceHandle<ogfx::ImageView>
ShadowMapGen::fallback_view(ShadowMapType type) const
{
    switch (type)
    {
    case ShadowMapType::Directional:
    case ShadowMapType::Spot:
        return m_fallback_2d.sampled_view;

    case ShadowMapType::Point:
        return m_fallback_cube.sampled_view;
    }

    throw std::runtime_error(
        "ShadowMapGen: Invalid fallback shadow map type"
    );
}

ShadowProjectionSettings ShadowMapGen::projection_settings() const
{
    return m_desc.proj_setting;
}

void ShadowMapGen::initialize_fallback_maps(orendergraph::RenderGraph& rg, ogfx::CommandBuffer& cmd)
{
    auto& ctx = m_shadow_node->context();
    const auto& desc = m_shadow_node->descriptor();

    const auto initialize = [&](ShadowMap& map, uint32_t layer_count)
    {
        if (map.was_rendered)
            return;

        // Undefined to DepthStencil
        transition_depth_image(cmd, rg.get_resource(map.image), ogfx::ImageState::Undefined, ogfx::ImageState::DepthStencil, layer_count);

        // Clear every layer to depth 1.0
        for (uint32_t framebuffer_index : map.framebuffer_indices)
        {
            if (framebuffer_index >= ctx.framebuffers.size())
            {
                throw std::runtime_error("ShadowMapGen: Invalid fallback framebuffer index");
            }

            ogfx::Framebuffer* framebuffer = ctx.framebuffers[framebuffer_index];

            if (!framebuffer)
            {
                throw std::runtime_error("ShadowMapGen: Missing fallback framebuffer");
            }

            ogfx::RenderPassBeginInfo begin_info{};
            begin_info.render_pass = ctx.render_pass;
            begin_info.framebuffer = framebuffer;
            begin_info.offset_x = 0;
            begin_info.offset_y = 0;
            begin_info.width = map.resolution;
            begin_info.height = map.resolution;
            begin_info.clear_values = desc.clear_values;

            cmd.begin_render_pass(begin_info);
            cmd.end_render_pass();
        }

        // DepthStencil to ShaderRead
        transition_depth_image(cmd, rg.get_resource(map.image), ogfx::ImageState::DepthStencil, ogfx::ImageState::ShaderRead, layer_count);

        map.was_rendered = true;
    };

    initialize(m_fallback_2d, 1);
    initialize(m_fallback_cube, 6);
}

ShadowMapGen::ShadowMap ShadowMapGen::create_shadow_map( orendergraph::RenderGraph& rg, uint32_t resolution, bool cubemap)
{
    ShadowMap map{};
    map.resolution = resolution;

    ogfx::ImageDesc image_desc{};
    image_desc.width = resolution;
    image_desc.height = resolution;
    image_desc.format = ogfx::Format::D32_FLOAT;
    image_desc.usage = ogfx::ImageUsage::DepthStencil | ogfx::ImageUsage::Sampled;
    image_desc.mip_levels = 1;
    image_desc.array_layers = cubemap ? 6u : 1u;
    image_desc.type = cubemap ? ogfx::ImageType::Cube : ogfx::ImageType::Image2D;
    map.image = rg.create_resource<ogfx::Image>(image_desc);

    const uint32_t face_count = cubemap ? 6u : 1u;
    map.attachment_views.reserve(face_count);
    map.framebuffer_indices.reserve(face_count);
    for (uint32_t face = 0; face < face_count; ++face)
    {
        ogfx::ImageViewDesc view_desc{};
        view_desc.image = rg.get_resource(map.image);
        view_desc.type = ogfx::ImageViewType::View2D;
        view_desc.aspect = ogfx::ImageAspect::Depth;
        view_desc.base_array_layer = face;
        view_desc.array_layer_count = 1;
        view_desc.base_mip_level = 0;
        view_desc.mip_level_count = 1;
        map.attachment_views.push_back(rg.create_resource<ogfx::ImageView>(view_desc));
    }

    if (cubemap)
    {
        ogfx::ImageViewDesc view_desc{};
        view_desc.image = rg.get_resource(map.image);
        view_desc.type = ogfx::ImageViewType::Cube;
        view_desc.aspect = ogfx::ImageAspect::Depth;
        view_desc.base_array_layer = 0;
        view_desc.array_layer_count = 6;
        view_desc.base_mip_level = 0;
        view_desc.mip_level_count = 1;
        map.sampled_view = rg.create_resource<ogfx::ImageView>(view_desc);
    }
    else
    {
        map.sampled_view = map.attachment_views.front();
    }

    auto& raster_ctx = m_shadow_node->context();
    auto& raster_desc = m_shadow_node->descriptor();
    for (const auto& attachment_view : map.attachment_views)
    {
        ogfx::FramebufferDesc framebuffer_desc{};
        framebuffer_desc.attachments = {rg.get_resource(attachment_view)};
        framebuffer_desc.width = resolution;
        framebuffer_desc.height = resolution;

        map.framebuffer_indices.push_back(raster_ctx.add_framebuffer(rg, raster_desc, framebuffer_desc));
    }

    return map;
}

void ShadowMapGen::retire_shadow_map(ShadowMap&& map)
{
    const uint32_t delay = m_desc.graphics.max_frames_in_flight() + 1u;
    m_retired_maps.push_back({std::move(map), delay});
}

void ShadowMapGen::destroy_shadow_map(orendergraph::RenderGraph& rg, ShadowMap& map)
{
    auto& raster_ctx = m_shadow_node->context();
    auto& raster_desc = m_shadow_node->descriptor();

    for (const uint32_t index : map.framebuffer_indices)
        raster_ctx.remove_framebuffer(rg, raster_desc, index);

    if (map.attachment_views.size() > 1 && map.sampled_view.is_valid())
        rg.release_resource<ogfx::ImageView>(map.sampled_view);

    for (auto view : map.attachment_views)
        rg.release_resource<ogfx::ImageView>(view);

    if (map.image.is_valid())
        rg.release_resource<ogfx::Image>(map.image);
}

void ShadowMapGen::collect_retired_maps(orendergraph::RenderGraph& rg)
{
    while (!m_retired_maps.empty())
    {
        for (auto& retired : m_retired_maps)
        {
            if (retired.frames_remaining > 0)
                --retired.frames_remaining;
        }

        while (!m_retired_maps.empty() && m_retired_maps.front().frames_remaining == 0)
        {
            destroy_shadow_map(rg, m_retired_maps.front().map);
            m_retired_maps.pop_front();
        }
    }
}

void ShadowMapGen::upsert_shadow_map(orendergraph::RenderGraph& rg, ShadowRegistry& registry, Entity light,
                                     uint32_t resolution, bool cubemap)
{
    auto it = registry.find(light);
    if (it != registry.end() && it->second.resolution == resolution)
        return;

    ShadowMap replacement = create_shadow_map(rg, resolution, cubemap);
    if (it != registry.end())
    {
        retire_shadow_map(std::move(it->second));
        it->second = std::move(replacement);
    }
    else
    {
        registry.emplace(light, std::move(replacement));
    }
}

void ShadowMapGen::remove_shadow_map(ShadowRegistry& registry, Entity light)
{
    const auto it = registry.find(light);
    if (it == registry.end())
        return;

    retire_shadow_map(std::move(it->second));
    registry.erase(it);
}

void ShadowMapGen::create_existing_maps(orendergraph::RenderGraph& rg)
{
    for (Entity light : m_desc.components.entities_with<DirectionalShadow>())
    {
        const auto* shadow = m_desc.components.get<DirectionalShadow>(light);
        if (shadow && m_desc.components.get<DirectionalLight>(light))
            upsert_shadow_map(rg, m_directional_maps, light, sanitize_resolution(shadow->resolution, 256, 4096), false);
    }
    for (Entity light : m_desc.components.entities_with<SpotShadow>())
    {
        const auto* shadow = m_desc.components.get<SpotShadow>(light);
        if (shadow && m_desc.components.get<SpotLight>(light))
            upsert_shadow_map(rg, m_spot_maps, light, sanitize_resolution(shadow->resolution, 128, 1024), false);
    }
    for (Entity light : m_desc.components.entities_with<PointShadow>())
    {
        const auto* shadow = m_desc.components.get<PointShadow>(light);
        if (shadow && m_desc.components.get<PointLight>(light))
            upsert_shadow_map(rg, m_point_maps, light, sanitize_resolution(shadow->resolution, 128, 2048), true);
    }
}

template <typename TShadow>
void ShadowMapGen::process_shadow_events(orendergraph::RenderGraph& rg, FrameEventQueue<ComponentEvent<TShadow>>& queue,
                                         ShadowRegistry& registry, uint32_t min_resolution, uint32_t max_resolution, bool cubemap)
{
    const uint32_t frame_index = m_desc.graphics.current_frame();
    queue.capture(m_desc.events);

    for (const auto& event : queue.get_events(frame_index))
    {
        if (event.type == ComponentEventType::Removed)
        {
            remove_shadow_map(registry, event.entity);
            continue;
        }

        const TShadow* shadow = m_desc.components.get<TShadow>(event.entity);
        bool has_matching_light = false;
        if constexpr (std::is_same_v<TShadow, DirectionalShadow>)
            has_matching_light = m_desc.components.get<DirectionalLight>(event.entity) != nullptr;
        else if constexpr (std::is_same_v<TShadow, SpotShadow>)
            has_matching_light = m_desc.components.get<SpotLight>(event.entity) != nullptr;
        else if constexpr (std::is_same_v<TShadow, PointShadow>)
            has_matching_light = m_desc.components.get<PointLight>(event.entity) != nullptr;

        if (!shadow || !has_matching_light)
        {
            remove_shadow_map(registry, event.entity);
            continue;
        }

        upsert_shadow_map(rg, registry, event.entity, sanitize_resolution(shadow->resolution, min_resolution, max_resolution), cubemap);
    }

    queue.clear(frame_index);
}

void ShadowMapGen::process_events(orendergraph::RenderGraph& rg)
{
    process_shadow_events(rg, m_directional_events, m_directional_maps, 256, 4096, false);
    process_shadow_events(rg, m_spot_events, m_spot_maps, 128, 1024, false);
    process_shadow_events(rg, m_point_events, m_point_maps, 128, 2048, true);
}

void ShadowMapGen::transition_all(orendergraph::RenderGraph& rg, ogfx::CommandBuffer& cmd, bool to_depth_attachment)
{
    const auto transition_registry = [&](ShadowRegistry& registry, uint32_t layer_count)
    {
        for (auto& [entity, map] : registry)
        {
            (void)entity;
            const auto before = to_depth_attachment
                ? (map.was_rendered ? ogfx::ImageState::ShaderRead : ogfx::ImageState::Undefined)
                : ogfx::ImageState::DepthStencil;
            const auto after = to_depth_attachment
                ? ogfx::ImageState::DepthStencil
                : ogfx::ImageState::ShaderRead;

            transition_depth_image(cmd, rg.get_resource(map.image), before, after, layer_count);
            if (!to_depth_attachment)
                map.was_rendered = true;
        }
    };

    transition_registry(m_directional_maps, 1);
    transition_registry(m_spot_maps, 1);
    transition_registry(m_point_maps, 6);
}

void ShadowMapGen::draw_shadow_casters(orendergraph::RenderGraph& rg, ogfx::CommandBuffer& cmd, ShadowMapType type, Entity light, uint32_t face)
{
    Matrix4 view_projection{};

    const ShadowProjectionSettings settings = projection_settings();

    switch (type)
    {
    case ShadowMapType::Directional:
    {
        const auto* shadow = m_desc.components.get<DirectionalShadow>(light);
        if (!shadow)
            return;

        const auto gpu_data = gpu_component_data(*shadow, light, m_desc.transforms, m_desc.camera, settings);
        view_projection = gpu_data.view_projection;
        break;
    }

    case ShadowMapType::Spot:
    {
        const auto* shadow = m_desc.components.get<SpotShadow>(light);
        const auto* light_component = m_desc.components.get<SpotLight>(light);
        if (!shadow)
            return;

        const auto gpu_data = gpu_component_data(*shadow, light, m_desc.components, m_desc.transforms, settings);
        view_projection = gpu_data.view_projection;
        break;
    }

    case ShadowMapType::Point:
    {
        const auto* shadow = m_desc.components.get<PointShadow>(light);
        if (!shadow || face >= 6)
            return;

        const auto gpu_data = gpu_component_data(*shadow, light, m_desc.components, m_desc.transforms, settings);
        view_projection = gpu_data.view_projections[face];
        break;
    }
    }

    auto& ctx = m_shadow_node->context();

    for (Entity entity : m_desc.components.entities_with<ShadowCaster>())
    {
        const PbrOpaqueRenderable* renderable = m_desc.components.get<PbrOpaqueRenderable>(entity);
        if (!renderable)
            continue;

        auto& gpu_meshes = m_desc.graphics.gpu_meshes();
        if (!gpu_meshes.is_resident(renderable->mesh))
        {
            gpu_meshes.make_resident(renderable->mesh);
            continue;
        }

        const GpuMeshDrawData draw_data = gpu_meshes.get_draw_data(renderable->mesh);
        if (!draw_data.is_valid())
            continue;

        for (const GpuMeshVertexBinding& binding : draw_data.vertex_bindings)
        {
            ogfx::VertexBufferBindInfo bind_info{};
            bind_info.buffer = binding.buffer;
            bind_info.binding = binding.binding;
            bind_info.offset = binding.offset;
            cmd.bind_vertex_buffer(bind_info);
        }

        if (draw_data.is_indexed())
        {
            ogfx::IndexBufferBindInfo bind_info{};
            bind_info.buffer = draw_data.index_buffer;
            bind_info.offset = draw_data.index_offset;
            bind_info.index_type = draw_data.index_type;
            cmd.bind_index_buffer(bind_info);
        }

        ShadowDrawConstants constants{};
        constants.model = m_desc.transforms.get_world_matrix(entity);
        constants.light_view_projection = view_projection;

        ogfx::InlineConstantsInfo push_info{};
        push_info.layout = ctx.pipeline_layout;
        push_info.stages = ogfx::ShaderStageFlags::Vertex;
        push_info.offset = 0;
        push_info.size = sizeof(constants);
        push_info.data = &constants;
        cmd.set_inline_constants(push_info);

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

void ShadowMapGen::execute_registry(orendergraph::RenderGraph& rg, ogfx::CommandBuffer& cmd,
                                    const ShadowRegistry& registry, ShadowMapType type)
{
    const auto& ctx = m_shadow_node->context();
    const auto& desc = m_shadow_node->descriptor();

    for (const auto& [light, map] : registry)
    {
        // A cubemap uses one pass per face
        // 2D maps use a single pass
        for (uint32_t face = 0; face < map.framebuffer_indices.size(); ++face)
        {
            const uint32_t framebuffer_index = map.framebuffer_indices[face];
            if (framebuffer_index >= ctx.framebuffers.size())
                continue;

            ogfx::Framebuffer* framebuffer = ctx.framebuffers[framebuffer_index];
            if (!framebuffer)
                continue;

            ogfx::RenderPassBeginInfo begin_info{};
            begin_info.render_pass = ctx.render_pass;
            begin_info.framebuffer = framebuffer;
            begin_info.offset_x = 0;
            begin_info.offset_y = 0;
            begin_info.width = map.resolution;
            begin_info.height = map.resolution;
            begin_info.clear_values = desc.clear_values;
            cmd.begin_render_pass(begin_info);

            ogfx::PipelineBindInfo pipeline_info{};
            pipeline_info.pipeline = ctx.pipeline;
            cmd.bind_pipeline(pipeline_info);

            // OGFX ADAPTER: Match the exact names/signatures of Omega's dynamic
            // viewport and scissor commands.
            ogfx::Viewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(map.resolution);
            viewport.height = static_cast<float>(map.resolution);
            viewport.min_depth = 0.0f;
            viewport.max_depth = 1.0f;
            cmd.set_viewport(viewport);

            ogfx::Scissor scissor{};
            scissor.x = 0;
            scissor.y = 0;
            scissor.width = map.resolution;
            scissor.height = map.resolution;
            cmd.set_scissor(scissor);

            draw_shadow_casters(rg, cmd, type, light, face);

            cmd.end_render_pass();
        }
    }
}

void ShadowMapGen::execute_shadow_passes(orendergraph::RenderGraph& rg, ogfx::CommandBuffer& cmd)
{
    execute_registry(rg, cmd, m_directional_maps, ShadowMapType::Directional);
    execute_registry(rg, cmd, m_spot_maps, ShadowMapType::Spot);
    execute_registry(rg, cmd, m_point_maps, ShadowMapType::Point);
}

void ShadowMapGen::add_node(orendergraph::RenderGraph& rg)
{
    m_shadow_node = rg.add_node<orendergraph::RasterContext>
    (
        "Shadow Map Generation",
        [this](orendergraph::RenderGraph&, orendergraph::RasterContext::Desc& desc)
        {
            ogfx::RenderPassAttachmentDesc depth{};
            depth.format = ogfx::Format::D32_FLOAT;
            depth.load_op = ogfx::LoadOp::Clear;
            depth.store_op = ogfx::StoreOp::Store;
            depth.initial_state = ogfx::ImageState::DepthStencil;
            depth.final_state = ogfx::ImageState::DepthStencil;
            depth.image_aspect = ogfx::ImageAspect::Depth;
            desc.render_pass_desc.attachments = {depth};

            desc.framebuffers_desc.clear();

            const std::filesystem::path shader_path = std::filesystem::path("shaders") / ("shadows/shadow_map.vert." OGRAPHICS_SHADER_FORMAT);

            ogfx::ShaderDesc vertex_shader{};
            vertex_shader.stage = ogfx::ShaderStage::Vertex;
            vertex_shader.entry_point = "VSMain";
            vertex_shader.bytecode = load_shader(shader_path);
            desc.vertex_shader_desc = vertex_shader;

            ogfx::InlineConstantRange draw_constants{};
            draw_constants.stages = ogfx::ShaderStageFlags::Vertex;
            draw_constants.offset = 0;
            draw_constants.size = sizeof(ShadowDrawConstants);
            desc.inline_constants_ranges.push_back(draw_constants);

            desc.vertex_input = standard_vertex_layout();
            desc.input_assembly.topology = ogfx::PrimitiveTopology::TriangleList;
            desc.input_assembly.primitive_restart_enable = false;

            desc.rasterization.polygon_mode = ogfx::PolygonMode::Fill;
            desc.rasterization.cull_mode = ogfx::CullMode::Front;
            desc.rasterization.front_face = ogfx::FrontFace::CounterClockwise;
            desc.multisampling.rasterization_samples = ogfx::SampleCount::Count1;

            desc.depth_stencil.depth_test_enable = true;
            desc.depth_stencil.depth_write_enable = true;
            desc.depth_stencil.depth_compare_op = ogfx::CompareOp::Less;
            desc.depth_stencil.stencil_test_enable = false;
            desc.color_blend.attachments.clear();

            desc.viewport.dynamic_viewport = true;
            desc.viewport.dynamic_scissor = true;

            // A valid static placeholder satisfies graphics-pipeline creation
            ogfx::Viewport viewport{};
            viewport.width = 1.0f;
            viewport.height = 1.0f;
            viewport.max_depth = 1.0f;
            desc.viewport.viewports = {viewport};

            ogfx::Scissor scissor{};
            scissor.width = 1;
            scissor.height = 1;
            desc.viewport.scissors = {scissor};

            desc.width = 1;
            desc.height = 1;
            desc.clear_values = {{.type = ogfx::ClearValueType::DepthStencil, .depth = 1.0f, .stencil = 0}};
        });

    m_shadow_node->set_pre_execute
    (
        [this](orendergraph::RenderGraph& rg, ogfx::CommandBuffer& cmd,
               orendergraph::RenderGraphNode<orendergraph::RasterContext>&)
        {
            collect_retired_maps(rg);
            process_events(rg);

            // First execution only
            initialize_fallback_maps(rg, cmd);

            transition_all(rg, cmd, true);
        }
    );

    m_shadow_node->set_execute
    (
        [this](orendergraph::RenderGraph& rg, ogfx::CommandBuffer& cmd,
               orendergraph::RenderGraphNode<orendergraph::RasterContext>&)
        {
            execute_shadow_passes(rg, cmd);
            transition_all(rg, cmd, false);
        }
    );

    m_shadow_node->set_post_execute
    (
        [](orendergraph::RenderGraph&, ogfx::CommandBuffer&,
           orendergraph::RenderGraphNode<orendergraph::RasterContext>&)
        {
            // Every shadow pass was ended explicitly during execution
        }
    );
}

} // namespace omega
