#include <omega/graphics/pipelines/raster_pipeline/features/pbr_opaque_feature.hpp>
#include <omega/graphics/pipelines/raster_pipeline/features/shadow_map_gen.hpp>

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
#include <omega/graphics/resources/gpu_shadow_data.hpp>

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

struct GpuShadowLookup
{
    uint32_t data_index = UINT32_MAX;
    uint32_t texture_index = UINT32_MAX;
};

template <typename TLight, typename TShadow, typename TGetTextureIndex>
bool update_shadow_lookup(orendergraph::RenderGraph& rg, ComponentSystem& components, DynamicBuffer& buffer, TGetTextureIndex&& get_texture_index)
{
    auto& lights = components.get_storage<TLight>();
    auto& shadows = components.get_storage<TShadow>();

    std::vector<GpuShadowLookup> lookups(std::max<size_t>(lights.size(), 1));
    for (uint32_t i = 0; i < lights.size(); ++i)
    {
        const Entity entity = lights.entity_at(i);

        if (!components.get<TShadow>(entity))
            continue;

        const auto texture_index = get_texture_index(entity);
        if (!texture_index.has_value())
            continue;

        lookups[i].data_index = shadows.get_index(entity);
        lookups[i].texture_index = *texture_index;
    }

    const uint64_t required_size = lookups.size() * sizeof(GpuShadowLookup);
    const uint64_t previous_capacity = buffer.get_capacity();

    if (!buffer.ensure_capacity(rg, required_size))
        throw std::runtime_error("Failed to resize shadow lookup buffer");

    ogfx::BufferWriteInfo write_info{};
    write_info.data = lookups.data();
    write_info.size = required_size;
    write_info.offset = 0;

    rg.get_resource(buffer.get_handle())->write(write_info);

    return buffer.get_capacity() != previous_capacity;
}

uint32_t grow_shadow_capacity(uint32_t current, uint32_t required, uint32_t maximum)
{
    uint32_t capacity = std::min(std::max(current, 1u), maximum);
    while (capacity < required)
    {
        if (capacity >= maximum / 2)
            return maximum;

        capacity *= 2;
    }

    return capacity;
}

struct ShadowTextureSyncResult
{
    bool lookup_changed = false;
    bool capacity_changed = false;
    uint32_t required_capacity = 0;

    std::vector<orendergraph::ImageBinding> descriptor_writes;
};

template <typename TShadow>
ShadowTextureSyncResult sync_shadow_texture_table(ShadowMapGen& shadows,
                                                  const std::vector<ComponentEvent<TShadow>>& events,
                                                  ShadowTextureTable& table, ShadowMapType type,
                                                  uint32_t allocated_capacity, uint32_t maximum_capacity)
{
    ShadowTextureSyncResult result{};
    result.required_capacity = allocated_capacity;

    const auto fallback_view = shadows.fallback_view(type);

    if (!fallback_view.is_valid())
        throw std::runtime_error("Shadow fallback image view is invalid");

    bool slots_freed = false;

    auto append_write = [&](uint32_t index, orendergraph::ResourceHandle<ogfx::ImageView> view)
    {
        orendergraph::ImageBinding write{};
        write.binding = 0;
        write.array_element = index;
        write.image_view = view;
        write.image_state = ogfx::ImageState::ShaderRead;

        result.descriptor_writes.push_back(write);
    };

    auto acquire_shadow = [&](Entity entity, orendergraph::ResourceHandle<ogfx::ImageView> view)
    {
        auto it = table.entries.find(entity);
        if (it == table.entries.end())
        {
            const auto slot = table.allocate_slot(maximum_capacity);
            if (!slot.has_value())
            {
                table.pending.insert(entity);
                result.lookup_changed = true;
                return;
            }

            it = table.entries.emplace(entity, ShadowTextureTable::Entry{*slot, {}}).first;

            result.lookup_changed = true;
        }

        table.pending.erase(entity);

        if (it->second.view == view)
            return;

        it->second.view = view;
        append_write(it->second.index, view);
    };

    // Process current shadow events first
    for (const auto& event : events)
    {
        const auto view = event.type == ComponentEventType::Removed ? orendergraph::ResourceHandle<ogfx::ImageView>{}
                                                                    : shadows.sampled_view(type, event.entity);
        if (!view.is_valid())
        {
            table.pending.erase(event.entity);

            const auto it = table.entries.find(event.entity);
            if (it == table.entries.end())
                continue;

            const uint32_t index = it->second.index;

            table.free_indices.push_back(index);
            table.entries.erase(it);

            slots_freed = true;
            result.lookup_changed = true;

            append_write(index, fallback_view);
            continue;
        }

        acquire_shadow(event.entity, view);
    }

    // Retry pending shadows only when slots have been freed
    if (slots_freed && !table.pending.empty())
    {
        for (auto it = table.pending.begin(); it != table.pending.end();)
        {
            const Entity entity = *it;

            // Verify that the shadow still exists
            const auto view = shadows.sampled_view(type, entity);
            if (!view.is_valid())
            {
                it = table.pending.erase(it);
                continue;
            }

            const auto slot = table.allocate_slot(maximum_capacity);
            if (!slot.has_value())
                break;

            table.entries.emplace(entity, ShadowTextureTable::Entry{*slot, view});

            append_write(*slot, view);

            result.lookup_changed = true;

            it = table.pending.erase(it);
        }
    }

    // Calculate the required allocation size
    result.required_capacity = std::max(allocated_capacity, table.next_index);
    result.capacity_changed = result.required_capacity > allocated_capacity;

    return result;
}

void write_shadow_texture_changes(orendergraph::RenderGraph& rg, orendergraph::RasterContext& context,
                                  uint32_t resource_set_index, const ShadowTextureSyncResult& changes)
{
    if (changes.descriptor_writes.empty())
        return;

    orendergraph::ResourceWriteDesc write{};
    write.resource_set_idx = resource_set_index;
    write.write_info.textures = changes.descriptor_writes;

    context.update_resource_set(rg, write);
}

void populate_shadow_texture_set(orendergraph::RenderGraph& rg, orendergraph::RasterContext& context,
                                 const ShadowTextureTable& table, uint32_t resource_set_index)
{
    orendergraph::ResourceWriteDesc write{};
    write.resource_set_idx = resource_set_index;

    write.write_info.textures.reserve(table.entries.size());

    for (const auto& [entity, entry] : table.entries)
    {
        orendergraph::ImageBinding binding{};
        binding.binding = 0;
        binding.array_element = entry.index;
        binding.image_view = entry.view;
        binding.image_state = ogfx::ImageState::ShaderRead;

        write.write_info.textures.push_back(binding);
    }

    if (!write.write_info.textures.empty())
        context.update_resource_set(rg, write);
}

void resize_shadow_texture_set(orendergraph::RenderGraph& rg, orendergraph::RasterContext& context,
                               ShadowResourceArray& resources, uint32_t layout_index, uint32_t new_capacity)
{
    if (new_capacity <= resources.capacity)
        return;

    orendergraph::ResourceSetDesc desc{};
    desc.resource_set_layout_idx = layout_index;
    desc.variable_descriptor_count = new_capacity;

    context.replace_resource_set(rg, desc, resources.resource_set_index);

    resources.capacity = new_capacity;
}

} // namespace

PbrOpaqueRenderFeature::PbrOpaqueRenderFeature(const PbrOpaqueRenderFeatureDesc& desc)
    : m_desc(desc), 
      m_renderable_events(m_desc.graphics.max_frames_in_flight()), 
      m_directional_light_events(m_desc.graphics.max_frames_in_flight()),
      m_point_light_events(m_desc.graphics.max_frames_in_flight()),
      m_spot_light_events(m_desc.graphics.max_frames_in_flight()),
      m_directional_shadow_events(m_desc.graphics.max_frames_in_flight()),
      m_point_shadow_events(m_desc.graphics.max_frames_in_flight()),
      m_spot_shadow_events(m_desc.graphics.max_frames_in_flight()),
      m_transform_events(m_desc.graphics.max_frames_in_flight())
{}

void PbrOpaqueRenderFeature::build(orenderer::RenderFeatureContext& ctx)
{
    orendergraph::RenderGraph& rg = ctx.render_graph();

    // Determine shadow limits
    {
        const auto& limits = rg.device().resource_limits();

        if (limits.max_bound_resource_sets < 8)
        {
            throw std::runtime_error("PBR requires at least 8 descriptor sets");
        }

        // Existing sampled-image descriptors accessible to the fragment shader:
        // Material set 1: 5 textures
        // IBL set 3:      3 textures
        constexpr uint32_t fixed_sampled_images = 8;

        const uint32_t total_limit = std::min(limits.max_per_stage_sampled_images, limits.max_resource_set_sampled_images);
        if (total_limit < fixed_sampled_images + 3)
        {
            throw std::runtime_error("Insufficient sampled-image descriptors for PBR shadows");
        }

        const uint32_t shadow_budget = total_limit - fixed_sampled_images;

        // Divide the available layout budget between the three arrays
        const uint32_t base = shadow_budget / 3;
        const uint32_t remainder = shadow_budget % 3;
        m_max_directional_shadow_descriptors = base + (remainder > 0 ? 1u : 0u);
        m_max_spot_shadow_descriptors = base + (remainder > 1 ? 1u : 0u);
        m_max_point_shadow_descriptors = base;
    }

    m_frame_resources.reserve(m_desc.graphics.max_frames_in_flight());
    for (uint32_t i = 0; i < m_desc.graphics.max_frames_in_flight(); i++)
    {
        m_frame_resources.emplace_back(m_desc.components, m_desc.events);

        FrameResources& frame = m_frame_resources[i];

        frame.directional_lights.create(rg);
        frame.point_lights.create(rg);
        frame.spot_lights.create(rg);
        frame.directional_shadows.create(rg);
        frame.point_shadows.create(rg);
        frame.spot_shadows.create(rg);

        ogfx::BufferDesc lookup_desc{};
        lookup_desc.size = sizeof(GpuShadowLookup);
        lookup_desc.usage = ogfx::BufferUsage::Storage;
        lookup_desc.memory_properties = ogfx::MemoryProperty::HostVisible | ogfx::MemoryProperty::HostCoherent;

        if (!frame.directional_shadow_lookup.create(rg, lookup_desc) ||
            !frame.point_shadow_lookup.create(rg, lookup_desc) || !frame.spot_shadow_lookup.create(rg, lookup_desc))
        {
            throw std::runtime_error("Failed to create shadow lookup buffers");
        }

        const GpuShadowLookup invalid_lookup{};
        for (DynamicBuffer* buffer : {&frame.directional_shadow_lookup, &frame.point_shadow_lookup, &frame.spot_shadow_lookup})
        {
            ogfx::BufferWriteInfo write{};
            write.data = &invalid_lookup;
            write.size = sizeof(invalid_lookup);
            rg.get_resource(buffer->get_handle())->write(write);
        }
    }

    create_resources(rg);

    m_default_material = m_desc.graphics.materials().create(PbrOpaqueMaterial{});

    add_node(rg);
}

void PbrOpaqueRenderFeature::on_swapchain_recreated(orenderer::RenderFeatureContext& ctx)
{
    create_output_resources(ctx.render_graph());

    ogfx::Swapchain& swapchain = m_desc.graphics.swapchain();
    orendergraph::RasterContext& raster_ctx = m_pbr_node->context();
    orendergraph::RasterContext::Desc& raster_desc = m_pbr_node->descriptor();

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

    // Shadow comparison sampler
    {
        ogfx::SamplerDesc desc{};
        desc.min_filter = ogfx::Filter::Linear;
        desc.mag_filter = ogfx::Filter::Linear;
        desc.address_u = ogfx::AddressMode::ClampToEdge;
        desc.address_v = ogfx::AddressMode::ClampToEdge;
        desc.address_w = ogfx::AddressMode::ClampToEdge;
        desc.compare_enable = true;
        desc.compare_op = ogfx::CompareOp::LessOrEqual;

        m_shadow_sampler = rg.create_resource<ogfx::Sampler>(desc);
    }
}

void PbrOpaqueRenderFeature::add_node(orendergraph::RenderGraph& rg)
{
    m_pbr_node = rg.add_node<orendergraph::RasterContext>
    (
        "PBR Opaque",
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

            // Shadow metadata and lookups (set 4)
            ogfx::ResourceSetLayoutDesc shadow_data_layout_desc{};

            auto add_shadow_data_binding = [&](uint32_t binding, ogfx::ResourceType type)
            {
                ogfx::ResourceBindingDesc resource{};
                resource.binding = binding;
                resource.type = type;
                resource.stages = ogfx::ShaderStageFlags::Fragment;

                shadow_data_layout_desc.bindings.push_back(resource);
            };

            // Shadow metadata
            add_shadow_data_binding(0, ogfx::ResourceType::StorageBuffer); // Directional
            add_shadow_data_binding(1, ogfx::ResourceType::StorageBuffer); // Spot
            add_shadow_data_binding(2, ogfx::ResourceType::StorageBuffer); // Point

            // Light-to-shadow lookups
            add_shadow_data_binding(3, ogfx::ResourceType::StorageBuffer); // Directional
            add_shadow_data_binding(4, ogfx::ResourceType::StorageBuffer); // Spot
            add_shadow_data_binding(5, ogfx::ResourceType::StorageBuffer); // Point

            // Comparison sampler
            add_shadow_data_binding(6, ogfx::ResourceType::Sampler);

            desc.resource_set_layouts_desc.push_back(shadow_data_layout_desc);

            // Variable-sized texture arrays (sets 5, 6 and 7)
            auto add_shadow_texture_layout = [&](uint32_t max_count)
            {
                ogfx::ResourceSetLayoutDesc layout{};

                ogfx::ResourceBindingDesc textures{};
                textures.binding = 0;
                textures.type = ogfx::ResourceType::SampledTexture;
                textures.array_count = max_count;
                textures.stages = ogfx::ShaderStageFlags::Fragment;
                textures.flags = ogfx::ResourceBindingFlags::PartiallyBound | ogfx::ResourceBindingFlags::VariableDescriptorCount;

                layout.bindings.push_back(textures);
                desc.resource_set_layouts_desc.push_back(layout);
            };

            add_shadow_texture_layout(m_max_directional_shadow_descriptors); // Set 5
            add_shadow_texture_layout(m_max_spot_shadow_descriptors);        // Set 6
            add_shadow_texture_layout(m_max_point_shadow_descriptors);       // Set 7

            // Resource sets
            uint32_t resource_set_idx = 0;

            // One light resource set per frame (set 0)
            for (uint32_t i = 0; i < m_desc.graphics.max_frames_in_flight(); ++i)
            {
                FrameResources& frame = m_frame_resources[i];

                orendergraph::ResourceSetDesc set_desc{};
                set_desc.resource_set_layout_idx = 0;

                const uint32_t set_index = resource_set_idx++;

                desc.resource_sets_desc.push_back(set_desc);

                auto make_buffer_binding = [](uint32_t binding, const DynamicBuffer& buffer)
                {
                    orendergraph::BufferBinding result{};
                    result.binding = binding;
                    result.buffer = buffer.get_handle();
                    result.offset = 0;
                    result.range = buffer.get_capacity();
                    return result;
                };

                const auto directional_binding = make_buffer_binding(0, frame.directional_lights.get_buffer());
                const auto point_binding = make_buffer_binding(1, frame.point_lights.get_buffer());
                const auto spot_binding = make_buffer_binding(2, frame.spot_lights.get_buffer());

                auto make_write = [&](const orendergraph::BufferBinding& binding)
                {
                    orendergraph::ResourceWriteDesc write{};
                    write.resource_set_idx = set_index;
                    write.write_info.buffers = {binding};
                    return write;
                };

                frame.directional_buffer_write = make_write(directional_binding);
                frame.point_buffer_write = make_write(point_binding);
                frame.spot_buffer_write = make_write(spot_binding);

                // Initial descriptor writes
                orendergraph::ResourceWriteDesc initial_write{};
                initial_write.resource_set_idx = set_index;
                initial_write.write_info.buffers = {directional_binding, point_binding, spot_binding};

                desc.resource_writes_desc.push_back(initial_write);
            }

            // Sampler (set 2)
            orendergraph::ResourceSetDesc resource_set_desc;
            resource_set_desc.resource_set_layout_idx = 2;

            desc.resource_sets_desc.push_back(resource_set_desc);

            orendergraph::SamplerBinding resource_sampler_binding;
            resource_sampler_binding.sampler = m_sampler;

            orendergraph::ResourceWriteDesc write_desc;
            write_desc.resource_set_idx = resource_set_idx++;
            write_desc.write_info.samplers = {resource_sampler_binding};

            desc.resource_writes_desc.push_back(write_desc);

            // Environment resource set (set 3)
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
            ibl_write_desc.resource_set_idx = resource_set_idx++;

            ibl_write_desc.write_info.textures = {environment_image_binding, brdf_lut_image_binding, irradiance_map_binding};
            ibl_write_desc.write_info.samplers = {environment_sampler_binding_write, brdf_lut_sampler_binding_write};

            desc.resource_writes_desc.push_back(ibl_write_desc);

            // One shadow data set and three texture-array sets per frame
            for (uint32_t i = 0; i < m_desc.graphics.max_frames_in_flight(); ++i)
            {
                FrameResources& frame = m_frame_resources[i];

                // Fixed shadow data set (set 4)
                orendergraph::ResourceSetDesc shadow_set_desc{};
                shadow_set_desc.resource_set_layout_idx = 4;

                const uint32_t shadow_data_set_index = resource_set_idx++;
                frame.shadow_data_resource_set_index = shadow_data_set_index;

                desc.resource_sets_desc.push_back(shadow_set_desc);

                auto make_buffer_binding = [](uint32_t binding, const DynamicBuffer& buffer)
                {
                    orendergraph::BufferBinding result{};
                    result.binding = binding;
                    result.buffer = buffer.get_handle();
                    result.offset = 0;
                    result.range = buffer.get_capacity();
                    return result;
                };

                const auto directional_data = make_buffer_binding(0, frame.directional_shadows.get_buffer());
                const auto spot_data = make_buffer_binding(1, frame.spot_shadows.get_buffer());
                const auto point_data = make_buffer_binding(2, frame.point_shadows.get_buffer());
                const auto directional_lookup = make_buffer_binding(3, frame.directional_shadow_lookup);
                const auto spot_lookup = make_buffer_binding(4, frame.spot_shadow_lookup);
                const auto point_lookup = make_buffer_binding(5, frame.point_shadow_lookup);

                orendergraph::SamplerBinding sampler_binding{};
                sampler_binding.binding = 6;
                sampler_binding.sampler = m_shadow_sampler;

                auto make_write = [&](const orendergraph::BufferBinding& buffer)
                {
                    orendergraph::ResourceWriteDesc write{};
                    write.resource_set_idx = shadow_data_set_index;
                    write.write_info.buffers = {buffer};
                    return write;
                };

                frame.directional_shadow_buffer_write = make_write(directional_data);
                frame.spot_shadow_buffer_write = make_write(spot_data);
                frame.point_shadow_buffer_write = make_write(point_data);

                frame.directional_shadow_lookup_write = make_write(directional_lookup);
                frame.spot_shadow_lookup_write = make_write(spot_lookup);
                frame.point_shadow_lookup_write = make_write(point_lookup);

                orendergraph::ResourceWriteDesc shadow_write{};
                shadow_write.resource_set_idx = shadow_data_set_index;
                shadow_write.write_info.samplers = {sampler_binding};
                shadow_write.write_info.buffers = {directional_data,   spot_data,   point_data,
                                                   directional_lookup, spot_lookup, point_lookup};

                desc.resource_writes_desc.push_back(shadow_write);

                // Variable-sized shadow texture sets (sets 5, 6, 7).
                auto add_texture_set = [&](uint32_t layout_index, ShadowResourceArray& array)
                {
                    orendergraph::ResourceSetDesc resource_set{};
                    resource_set.resource_set_layout_idx = layout_index;
                    resource_set.variable_descriptor_count = array.capacity;

                    array.resource_set_index = resource_set_idx++;

                    desc.resource_sets_desc.push_back(resource_set);
                };

                add_texture_set(5, frame.directional_shadow_resources);
                add_texture_set(6, frame.spot_shadow_resources);
                add_texture_set(7, frame.point_shadow_resources);
            }

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

    auto& ctx = m_pbr_node->context();
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

    m_pbr_node->set_pre_execute
    (
        [this](orendergraph::RenderGraph& rg, ogfx::CommandBuffer& cmd,
            orendergraph::RenderGraphNode<orendergraph::RasterContext>& node)
        {
            const uint32_t frame_index = m_desc.graphics.current_frame();
            FrameResources& frame = m_frame_resources[frame_index];

            auto has_structural_changes = [](const auto& events)
            {
                return std::any_of(
                    events.begin(), events.end(), [](const auto& event)
                    { return event.type == ComponentEventType::Added || event.type == ComponentEventType::Removed; });
            };

            bool directional_lights_changed = false;
            bool point_lights_changed = false;
            bool spot_lights_changed = false;

            // Update light buffers
            {
                m_directional_light_events.capture(m_desc.events);
                m_point_light_events.capture(m_desc.events);
                m_spot_light_events.capture(m_desc.events);

                directional_lights_changed = has_structural_changes(m_directional_light_events.get_events(frame_index));
                point_lights_changed = has_structural_changes(m_point_light_events.get_events(frame_index));
                spot_lights_changed = has_structural_changes(m_spot_light_events.get_events(frame_index));

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

            // Update shadow buffers
            {
                const ShadowProjectionSettings& settings = m_desc.shadows.projection_settings();

                // Update directional light when camera moves
                {
                    m_transform_events.capture(m_desc.events);

                    const auto& transform_events = m_transform_events.get_events(frame_index);

                    const bool camera_moved = std::any_of(
                        transform_events.begin(), transform_events.end(), [this](const ComponentEvent<Transform>& event)
                        { return event.entity == m_desc.camera && event.type == ComponentEventType::Modified; });

                    if (camera_moved)
                    {
                        auto& storage = m_desc.components.get_storage<DirectionalShadow>();
                        const std::vector<Entity> entities = m_desc.components.entities_with<DirectionalShadow>();
                        for (Entity entity : entities)
                        {
                            frame.directional_shadows.write_component(storage, entity, rg, m_desc.transforms,
                                                                      m_desc.camera, settings);
                        }
                    }

                    m_transform_events.clear(frame_index);
                }

                m_directional_shadow_events.capture(m_desc.events);
                m_point_shadow_events.capture(m_desc.events);
                m_spot_shadow_events.capture(m_desc.events);

                auto directional_shadow_events = m_directional_shadow_events.get_events(frame_index);
                auto point_shadow_events = m_point_shadow_events.get_events(frame_index);
                auto spot_shadow_events = m_spot_shadow_events.get_events(frame_index);

                if (frame.directional_shadows.process_frame(cmd, rg, directional_shadow_events,
                                                            m_desc.transforms, m_desc.camera, settings))
                {
                    frame.directional_shadow_buffer_write.write_info.buffers[0].range = frame.directional_shadows.get_buffer().get_capacity();
                    node.context().update_resource_set(rg, frame.directional_shadow_buffer_write);
                }

                if (frame.point_shadows.process_frame(cmd, rg, point_shadow_events, 
                                                     m_desc.components, m_desc.transforms, settings))
                {
                    frame.point_shadow_buffer_write.write_info.buffers[0].range = frame.point_shadows.get_buffer().get_capacity();
                    node.context().update_resource_set(rg, frame.point_shadow_buffer_write);
                }

                if (frame.spot_shadows.process_frame(cmd, rg, spot_shadow_events,
                                                     m_desc.components, m_desc.transforms, settings))
                {
                    frame.spot_shadow_buffer_write.write_info.buffers[0].range = frame.spot_shadows.get_buffer().get_capacity();
                    node.context().update_resource_set(rg, frame.spot_shadow_buffer_write);
                }

                auto sync_array = [&](const auto& events, ShadowTextureTable& table, ShadowResourceArray& resources,
                                      ShadowMapType type, uint32_t layout_index, uint32_t maximum)
                {
                    ShadowTextureSyncResult changes = sync_shadow_texture_table(m_desc.shadows, events, table, type, resources.capacity, maximum);

                    if (changes.capacity_changed)
                    {
                        const uint32_t new_capacity = grow_shadow_capacity(resources.capacity, changes.required_capacity, maximum);
                        resize_shadow_texture_set(rg, node.context(), resources, layout_index, new_capacity);
                        populate_shadow_texture_set(rg, node.context(), table, resources.resource_set_index);
                    }
                    else
                    {
                        write_shadow_texture_changes(rg, node.context(), resources.resource_set_index, changes);
                    }

                    return changes.lookup_changed;
                };

                const bool directional_slots_changed = sync_array(
                    directional_shadow_events,
                    frame.directional_shadow_textures,
                    frame.directional_shadow_resources,
                    ShadowMapType::Directional, 5,
                    m_max_directional_shadow_descriptors
                );
                
                const bool spot_slots_changed = sync_array(
                    spot_shadow_events,
                    frame.spot_shadow_textures,
                    frame.spot_shadow_resources,
                    ShadowMapType::Spot, 6,
                    m_max_spot_shadow_descriptors
                );
                
                const bool point_slots_changed = sync_array(
                    point_shadow_events,
                    frame.point_shadow_textures,
                    frame.point_shadow_resources,
                    ShadowMapType::Point, 7,
                    m_max_point_shadow_descriptors
                );

                if (directional_slots_changed || directional_lights_changed)
                {
                    const bool resized = update_shadow_lookup<DirectionalLight, DirectionalShadow>(rg, m_desc.components, frame.directional_shadow_lookup, [&](Entity entity) { return frame.directional_shadow_textures.find(entity); });
                    if (resized)
                    {
                        frame.directional_shadow_lookup_write.write_info.buffers[0].range = frame.directional_shadow_lookup.get_capacity();
                        node.context().update_resource_set(rg, frame.directional_shadow_lookup_write);
                    }
                }

                if (spot_slots_changed || spot_lights_changed)
                {
                    const bool resized = update_shadow_lookup<SpotLight, SpotShadow>(rg, m_desc.components, frame.spot_shadow_lookup, [&](Entity entity) { return frame.spot_shadow_textures.find(entity); });
                    if (resized)
                    {
                        frame.spot_shadow_lookup_write.write_info.buffers[0].range = frame.spot_shadow_lookup.get_capacity();
                        node.context().update_resource_set(rg, frame.spot_shadow_lookup_write);
                    }
                }

                if (point_slots_changed || point_lights_changed)
                {
                    const bool resized = update_shadow_lookup<PointLight, PointShadow>(rg, m_desc.components, frame.point_shadow_lookup,[&](Entity entity) { return frame.point_shadow_textures.find(entity); });
                    if (resized)
                    {
                        frame.point_shadow_lookup_write.write_info.buffers[0].range = frame.point_shadow_lookup.get_capacity();
                        node.context().update_resource_set(rg, frame.point_shadow_lookup_write);
                    }
                }

                m_directional_shadow_events.clear(frame_index);
                m_point_shadow_events.clear(frame_index);
                m_spot_shadow_events.clear(frame_index);
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

    m_pbr_node->set_execute
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

            // Light resources
            {
                ogfx::ResourceSetBindInfo resource_set_info{};
                resource_set_info.layout = ctx.pipeline_layout;
                resource_set_info.pipeline_type = ogfx::PipelineType::Graphics;
                resource_set_info.set_index = 0;
                resource_set_info.resource_set = ctx.resource_sets[m_desc.graphics.current_frame()];

                cmd.bind_resource_set(resource_set_info);
            }

            // Environment resources
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

            // Shadow data and lookup buffers (set 4)
            {
                ogfx::ResourceSetBindInfo resource_set_info{};
                resource_set_info.layout = ctx.pipeline_layout;
                resource_set_info.pipeline_type = ogfx::PipelineType::Graphics;
                resource_set_info.set_index = 4;
                resource_set_info.resource_set = ctx.resource_sets[frame.shadow_data_resource_set_index];

                cmd.bind_resource_set(resource_set_info);
            }

            // Directional shadow textures (set 5)
            {
                ogfx::ResourceSetBindInfo resource_set_info{};
                resource_set_info.layout = ctx.pipeline_layout;
                resource_set_info.pipeline_type = ogfx::PipelineType::Graphics;
                resource_set_info.set_index = 5;
                resource_set_info.resource_set =
                    ctx.resource_sets[frame.directional_shadow_resources.resource_set_index];

                cmd.bind_resource_set(resource_set_info);
            }

            // Spot shadow textures (set 6)
            {
                ogfx::ResourceSetBindInfo resource_set_info{};
                resource_set_info.layout = ctx.pipeline_layout;
                resource_set_info.pipeline_type = ogfx::PipelineType::Graphics;
                resource_set_info.set_index = 6;
                resource_set_info.resource_set = ctx.resource_sets[frame.spot_shadow_resources.resource_set_index];

                cmd.bind_resource_set(resource_set_info);
            }

            // Point shadow textures (set 7)
            {
                ogfx::ResourceSetBindInfo resource_set_info{};
                resource_set_info.layout = ctx.pipeline_layout;
                resource_set_info.pipeline_type = ogfx::PipelineType::Graphics;
                resource_set_info.set_index = 7;
                resource_set_info.resource_set = ctx.resource_sets[frame.point_shadow_resources.resource_set_index];

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