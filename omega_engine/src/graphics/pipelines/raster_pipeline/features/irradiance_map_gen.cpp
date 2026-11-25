#include <omega/graphics/pipelines/raster_pipeline/features/irradiance_map_gen.hpp>

#include <orenderer/render_feature_context.hpp>
#include <orendergraph/render_graph.hpp>

#include <ogfx/command_buffer.hpp>
#include <ogfx/image.hpp>
#include <ogfx/image_view.hpp>
#include <ogfx/sampler.hpp>

#include <omega/loader/loader.hpp>

#include <cstdint>
#include <filesystem>

namespace omega
{

namespace
{

constexpr uint32_t irradiance_resolution = 32;
constexpr uint32_t workgroup_size = 8;
constexpr uint32_t cubemap_face_count = 6;

} // namespace

IrradianceMapGen::IrradianceMapGen(const IrradianceMapGenDesc& desc) : m_desc(desc) {}

void IrradianceMapGen::build(orenderer::RenderFeatureContext& ctx)
{
    orendergraph::RenderGraph& rg = ctx.render_graph();

    create_resources(rg);
    add_node(rg);
}

void IrradianceMapGen::create_resources(orendergraph::RenderGraph& rg)
{
    // Irradiance cubemap
    ogfx::ImageDesc image_desc{};
    image_desc.width = irradiance_resolution;
    image_desc.height = irradiance_resolution;
    image_desc.format = ogfx::Format::RGBA16_FLOAT;
    image_desc.usage = ogfx::ImageUsage::Storage | ogfx::ImageUsage::Sampled;
    image_desc.mip_levels = 1;
    image_desc.array_layers = cubemap_face_count;
    image_desc.type = ogfx::ImageType::Cube;

    m_irradiance_map = rg.create_resource<ogfx::Image>(image_desc);

    // Storage view: six 2D array layers
    ogfx::ImageViewDesc storage_view_desc{};
    storage_view_desc.image = rg.get_resource(m_irradiance_map);
    storage_view_desc.type = ogfx::ImageViewType::View2DArray;
    storage_view_desc.aspect = ogfx::ImageAspect::Color;
    storage_view_desc.base_array_layer = 0;
    storage_view_desc.array_layer_count = cubemap_face_count;
    storage_view_desc.base_mip_level = 0;
    storage_view_desc.mip_level_count = 1;

    m_irradiance_storage_view = rg.create_resource<ogfx::ImageView>(storage_view_desc);

    // Sampled cubemap view
    ogfx::ImageViewDesc cube_view_desc{};
    cube_view_desc.image = rg.get_resource(m_irradiance_map);
    cube_view_desc.type = ogfx::ImageViewType::Cube;
    cube_view_desc.aspect = ogfx::ImageAspect::Color;
    cube_view_desc.base_array_layer = 0;
    cube_view_desc.array_layer_count = cubemap_face_count;
    cube_view_desc.base_mip_level = 0;
    cube_view_desc.mip_level_count = 1;

    m_irradiance_map_view = rg.create_resource<ogfx::ImageView>(cube_view_desc);

    // Source environment sampler
    ogfx::SamplerDesc sampler_desc{};
    sampler_desc.min_filter = ogfx::Filter::Linear;
    sampler_desc.mag_filter = ogfx::Filter::Linear;
    sampler_desc.mip_filter = ogfx::MipmapFilter::Linear;
    sampler_desc.address_u = ogfx::AddressMode::ClampToEdge;
    sampler_desc.address_v = ogfx::AddressMode::ClampToEdge;
    sampler_desc.address_w = ogfx::AddressMode::ClampToEdge;
    sampler_desc.max_lod = 1000.0f;

    m_environment_sampler = rg.create_resource<ogfx::Sampler>(sampler_desc);
}

void IrradianceMapGen::add_node(orendergraph::RenderGraph& rg)
{
    m_irradiance_node = rg.add_node<orendergraph::ComputeContext>
    (
        "Irradiance Map Generation",
        [this](orendergraph::RenderGraph& rg, orendergraph::ComputeContext::Desc& desc)
        {
            rg.read(m_desc.environment_map, ogfx::ImageState::ShaderRead, {ogfx::ImageAspect::Color});
            rg.write(m_irradiance_map, ogfx::ImageState::Common, {ogfx::ImageAspect::Color});

            const std::filesystem::path shader_path = std::filesystem::path("shaders/") / ("pbr/irradiance_map_gen.comp." OGRAPHICS_SHADER_FORMAT);

            ogfx::ShaderDesc shader_desc{};
            shader_desc.stage = ogfx::ShaderStage::Compute;
            shader_desc.entry_point = "CSMain";
            shader_desc.bytecode = load_shader(shader_path);

            desc.compute_shader_desc = shader_desc;

            // Set 0, binding 0: source environment cubemap
            ogfx::ResourceBindingDesc environment_binding{};
            environment_binding.binding = 0;
            environment_binding.type = ogfx::ResourceType::SampledTexture;
            environment_binding.array_count = 1;
            environment_binding.stages = ogfx::ShaderStageFlags::Compute;

            // Set 0, binding 1: destination irradiance cubemap
            ogfx::ResourceBindingDesc irradiance_binding{};
            irradiance_binding.binding = 1;
            irradiance_binding.type = ogfx::ResourceType::StorageTexture;
            irradiance_binding.array_count = 1;
            irradiance_binding.stages = ogfx::ShaderStageFlags::Compute;

            // Set 0, binding 2: source environment sampler
            ogfx::ResourceBindingDesc sampler_binding{};
            sampler_binding.binding = 2;
            sampler_binding.type = ogfx::ResourceType::Sampler;
            sampler_binding.array_count = 1;
            sampler_binding.stages = ogfx::ShaderStageFlags::Compute;

            ogfx::ResourceSetLayoutDesc set_layout_desc{};
            set_layout_desc.bindings = {environment_binding, irradiance_binding, sampler_binding};

            desc.resource_set_layouts_desc.push_back(set_layout_desc);

            // Resource set
            orendergraph::ResourceSetDesc resource_set_desc{};
            resource_set_desc.resource_set_layout_idx = 0;

            desc.resource_sets_desc.push_back(resource_set_desc);

            // Source cubemap descriptor
            orendergraph::ImageBinding environment_image_binding{};
            environment_image_binding.binding = 0;
            environment_image_binding.image_view = m_desc.environment_map_view;
            environment_image_binding.image_state = ogfx::ImageState::ShaderRead;

            // Destination cubemap descriptor
            orendergraph::ImageBinding irradiance_image_binding{};
            irradiance_image_binding.binding = 1;
            irradiance_image_binding.image_view = m_irradiance_storage_view;
            irradiance_image_binding.image_state = ogfx::ImageState::Common;

            // Sampler descriptor
            orendergraph::SamplerBinding environment_sampler_binding{};
            environment_sampler_binding.binding = 2;
            environment_sampler_binding.sampler = m_environment_sampler;

            orendergraph::ResourceWriteDesc write_desc{};
            write_desc.resource_set_idx = 0;
            write_desc.write_info.textures = {environment_image_binding, irradiance_image_binding};
            write_desc.write_info.samplers = {environment_sampler_binding};

            desc.resource_writes_desc.push_back(write_desc);

            desc.group_count_x = (irradiance_resolution + workgroup_size - 1) / workgroup_size;
            desc.group_count_y = (irradiance_resolution + workgroup_size - 1) / workgroup_size;
            desc.group_count_z = cubemap_face_count;
        });

    m_irradiance_node->set_execute(
        [](orendergraph::RenderGraph& rg, ogfx::CommandBuffer& cmd,
           orendergraph::RenderGraphNode<orendergraph::ComputeContext>& node)
        {
            orendergraph::ComputeContext& ctx = node.context();
            const auto& desc = node.descriptor();

            ogfx::ResourceSetBindInfo bind_info{};
            bind_info.layout = ctx.pipeline_layout;
            bind_info.resource_set = ctx.resource_sets[0];
            bind_info.set_index = 0;
            bind_info.pipeline_type = ogfx::PipelineType::Compute;

            cmd.bind_resource_set(bind_info);

            cmd.dispatch({desc.group_count_x, desc.group_count_y, desc.group_count_z});
        });
}

} // namespace omega