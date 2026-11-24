#include <omega/graphics/pipelines/raster_pipeline/features/environment_prefilter_feature.hpp>

#include <orenderer/render_feature_context.hpp>
#include <orendergraph/render_graph.hpp>

#include <ogfx/command_buffer.hpp>
#include <ogfx/image.hpp>
#include <ogfx/image_view.hpp>
#include <ogfx/resource_allocator.hpp>

#include <omega/graphics/graphics_system.hpp>
#include <omega/loader/loader.hpp>

#include <algorithm>
#include <bit>
#include <filesystem>
#include <stdexcept>

namespace omega
{

namespace
{

constexpr uint32_t workgroup_size = 8;
constexpr uint32_t cubemap_face_count = 6;

struct PrefilterConstants
{
    float roughness = 0.0f;
    uint32_t sample_count = 128;
    uint32_t padding0 = 0;
    uint32_t padding1 = 0;
};

} // namespace

EnvironmentPrefilterFeature::EnvironmentPrefilterFeature(const EnvironmentPrefilterFeatureDesc& desc) : m_desc(desc) {}

void EnvironmentPrefilterFeature::build(orenderer::RenderFeatureContext& ctx)
{
    orendergraph::RenderGraph& rg = ctx.render_graph();

    create_resources(rg);

    ogfx::DescriptorPoolDesc pool_desc{};
    pool_desc.max_sets = m_mip_levels;
    pool_desc.sizes = {{ogfx::ResourceType::SampledTexture, m_mip_levels},
                       {ogfx::ResourceType::StorageTexture, m_mip_levels},
                       {ogfx::ResourceType::Sampler, m_mip_levels}};

    rg.get_resource_allocator().create_pool(pool_desc);

    add_node(rg);
}

void EnvironmentPrefilterFeature::create_resources(orendergraph::RenderGraph& rg)
{
    const ogfx::Image* source = rg.get_resource(m_desc.environment_map);

    if (!source)
        throw std::runtime_error("Invalid environment map");

    m_resolution = source->width();
    m_mip_levels = std::bit_width(m_resolution);

    // GGX-prefiltered cubemap
    ogfx::ImageDesc image_desc{};
    image_desc.width = m_resolution;
    image_desc.height = m_resolution;
    image_desc.format = ogfx::Format::RGBA16_FLOAT;
    image_desc.mip_levels = m_mip_levels;
    image_desc.array_layers = cubemap_face_count;
    image_desc.type = ogfx::ImageType::Cube;

    image_desc.usage = ogfx::ImageUsage::Storage | ogfx::ImageUsage::Sampled;

    m_out_image = rg.create_resource<ogfx::Image>(image_desc);

    // Cubemap view spanning the entire mip chain
    ogfx::ImageViewDesc cube_view_desc{};
    cube_view_desc.image = rg.get_resource(m_out_image);
    cube_view_desc.type = ogfx::ImageViewType::Cube;
    cube_view_desc.aspect = ogfx::ImageAspect::Color;
    cube_view_desc.base_mip_level = 0;
    cube_view_desc.mip_level_count = m_mip_levels;
    cube_view_desc.base_array_layer = 0;
    cube_view_desc.array_layer_count = cubemap_face_count;

    m_out_view = rg.create_resource<ogfx::ImageView>(cube_view_desc);

    // One writable array view for each mip level
    m_storage_views.reserve(m_mip_levels);

    for (uint32_t mip = 0; mip < m_mip_levels; ++mip)
    {
        ogfx::ImageViewDesc view_desc{};
        view_desc.image = rg.get_resource(m_out_image);
        view_desc.type = ogfx::ImageViewType::View2DArray;
        view_desc.aspect = ogfx::ImageAspect::Color;
        view_desc.base_mip_level = mip;
        view_desc.mip_level_count = 1;
        view_desc.base_array_layer = 0;
        view_desc.array_layer_count = cubemap_face_count;

        m_storage_views.push_back(rg.create_resource<ogfx::ImageView>(view_desc));
    }

    // Source cubemap sampler
    ogfx::SamplerDesc sampler_desc{};
    sampler_desc.min_filter = ogfx::Filter::Linear;
    sampler_desc.mag_filter = ogfx::Filter::Linear;
    sampler_desc.mip_filter = ogfx::MipmapFilter::Linear;
    sampler_desc.address_u = ogfx::AddressMode::ClampToEdge;
    sampler_desc.address_v = ogfx::AddressMode::ClampToEdge;
    sampler_desc.address_w = ogfx::AddressMode::ClampToEdge;

    m_sampler = rg.create_resource<ogfx::Sampler>(sampler_desc);
}

void EnvironmentPrefilterFeature::add_node(orendergraph::RenderGraph& rg)
{
    m_prefilter_node = rg.add_node<orendergraph::ComputeContext>
    (
        "Environment Prefilter",
        [this](orendergraph::RenderGraph& rg, orendergraph::ComputeContext::Desc& desc)
        {
            rg.read(m_desc.environment_map, ogfx::ImageState::ShaderRead, {ogfx::ImageAspect::Color});
            rg.write(m_out_image, ogfx::ImageState::Common, {ogfx::ImageAspect::Color});

            // Shader
            const std::filesystem::path shader_path = std::filesystem::path("shaders/") / ("pbr/environment_prefilter.comp." OGRAPHICS_SHADER_FORMAT);

            ogfx::ShaderDesc shader_desc{};
            shader_desc.stage = ogfx::ShaderStage::Compute;
            shader_desc.entry_point = "CSMain";
            shader_desc.bytecode = load_shader(shader_path);

            desc.compute_shader_desc = shader_desc;

            // Set 0, binding 0: input cubemap
            ogfx::ResourceBindingDesc input_binding{};
            input_binding.binding = 0;
            input_binding.type = ogfx::ResourceType::SampledTexture;
            input_binding.stages = ogfx::ShaderStageFlags::Compute;

            // Set 0, binding 1: output mip
            ogfx::ResourceBindingDesc output_binding{};
            output_binding.binding = 1;
            output_binding.type = ogfx::ResourceType::StorageTexture;
            output_binding.stages = ogfx::ShaderStageFlags::Compute;

            // Set 0, binding 2: sampler
            ogfx::ResourceBindingDesc sampler_binding{};
            sampler_binding.binding = 2;
            sampler_binding.type = ogfx::ResourceType::Sampler;
            sampler_binding.stages = ogfx::ShaderStageFlags::Compute;

            ogfx::ResourceSetLayoutDesc set_layout_desc{};
            set_layout_desc.bindings = {input_binding, output_binding, sampler_binding};

            desc.resource_set_layouts_desc.push_back(set_layout_desc);

            // Push constants
            ogfx::InlineConstantRange constants_range{};
            constants_range.stages = ogfx::ShaderStageFlags::Compute;
            constants_range.offset = 0;
            constants_range.size = sizeof(PrefilterConstants);

            desc.inline_constants_ranges.push_back(constants_range);

            // Resource set per mip
            for (uint32_t mip = 0; mip < m_mip_levels; ++mip)
            {
                orendergraph::ResourceSetDesc resource_set_desc{};
                resource_set_desc.resource_set_layout_idx = 0;

                desc.resource_sets_desc.push_back(resource_set_desc);

                orendergraph::ImageBinding input{};
                input.binding = 0;
                input.image_view = m_desc.environment_map_view;
                input.image_state = ogfx::ImageState::ShaderRead;

                orendergraph::ImageBinding output{};
                output.binding = 1;
                output.image_view = m_storage_views[mip];
                output.image_state = ogfx::ImageState::Common;

                orendergraph::SamplerBinding sampler{};
                sampler.binding = 2;
                sampler.sampler = m_sampler;

                orendergraph::ResourceWriteDesc write_desc{};
                write_desc.resource_set_idx = mip;
                write_desc.write_info.textures = {input, output};
                write_desc.write_info.samplers = {sampler};

                desc.resource_writes_desc.push_back(write_desc);
            }

            desc.group_count_x = (m_resolution + workgroup_size - 1) / workgroup_size;
            desc.group_count_y = desc.group_count_x;
            desc.group_count_z = cubemap_face_count;
        });

    m_prefilter_node->set_execute
    (
        [this](orendergraph::RenderGraph& rg, ogfx::CommandBuffer& cmd,
               orendergraph::RenderGraphNode<orendergraph::ComputeContext>& node)
        {
            orendergraph::ComputeContext& ctx = node.context();

            for (uint32_t mip = 0; mip < m_mip_levels; ++mip)
            {
                // Bind the source cubemap and destination mip
                ogfx::ResourceSetBindInfo bind_info{};
                bind_info.layout = ctx.pipeline_layout;
                bind_info.resource_set = ctx.resource_sets[mip];
                bind_info.set_index = 0;
                bind_info.pipeline_type = ogfx::PipelineType::Compute;

                cmd.bind_resource_set(bind_info);

                // Associate this mip with its GGX roughness
                PrefilterConstants constants{};
                constants.roughness = static_cast<float>(mip) / static_cast<float>(m_mip_levels - 1);

                constants.sample_count = m_desc.sample_count;

                ogfx::InlineConstantsInfo constants_info{};
                constants_info.layout = ctx.pipeline_layout;
                constants_info.stages = ogfx::ShaderStageFlags::Compute;
                constants_info.offset = 0;
                constants_info.size = sizeof(PrefilterConstants);
                constants_info.data = &constants;

                cmd.set_inline_constants(constants_info);

                const uint32_t mip_resolution = std::max(1u, m_resolution >> mip);
                const uint32_t group_count = (mip_resolution + workgroup_size - 1) / workgroup_size;

                cmd.dispatch({group_count, group_count, cubemap_face_count});
            }
        });
}

} // namespace omega
