#include <omega/graphics/pipelines/raster_pipeline/features/tone_mapping_feature.hpp>

#include <orenderer/render_feature_context.hpp>
#include <orendergraph/render_graph.hpp>

#include <ogfx/command_buffer.hpp>
#include <ogfx/image.hpp>
#include <ogfx/image_view.hpp>
#include <ogfx/sampler.hpp>

#include <omega/loader/loader.hpp>
#include <omega/graphics/graphics_system.hpp>

#include <algorithm>
#include <cstdint>
#include <filesystem>

namespace omega
{

namespace
{

constexpr uint32_t workgroup_size = 8;

struct ToneMappingConstants
{
    float exposure;
    float padding[3];
};

} // namespace

ToneMappingFeature::ToneMappingFeature(const ToneMappingFeatureDesc& desc) : m_desc(desc) {}

void ToneMappingFeature::build(orenderer::RenderFeatureContext& ctx)
{
    orendergraph::RenderGraph& rg = ctx.render_graph();

    create_resources(rg);
    add_node(rg);
}

void ToneMappingFeature::set_exposure(float exposure)
{
    m_desc.exposure = std::max(exposure, 0.0f);
}

void ToneMappingFeature::create_out_resources(orendergraph::RenderGraph& rg)
{
    const ogfx::Image* input = rg.get_resource(m_desc.input_image);

    // Tone-mapped output
    ogfx::ImageDesc image_desc{};
    image_desc.width = input->width();
    image_desc.height = input->height();
    image_desc.format = ogfx::Format::RGBA16_FLOAT;
    image_desc.usage = ogfx::ImageUsage::Storage | ogfx::ImageUsage::Sampled;
    image_desc.mip_levels = 1;
    image_desc.array_layers = 1;
    image_desc.type = ogfx::ImageType::Image2D;

    if (!m_out_image.is_valid())
    {
        m_out_image = rg.create_resource<ogfx::Image>(image_desc);
    }
    else if (!rg.replace_resource<ogfx::Image>(m_out_image, image_desc))
    {
        throw std::runtime_error("Failed to recreate tone mapping output image");
    }
    else
    {
        rg.reset_resource_state(m_out_image);
    }

    // Output storage/sample view
    ogfx::ImageViewDesc view_desc{};
    view_desc.image = rg.get_resource(m_out_image);
    view_desc.type = ogfx::ImageViewType::View2D;
    view_desc.aspect = ogfx::ImageAspect::Color;
    view_desc.base_array_layer = 0;
    view_desc.array_layer_count = 1;
    view_desc.base_mip_level = 0;
    view_desc.mip_level_count = 1;

    if (!m_out_image_view.is_valid())
    {
        m_out_image_view = rg.create_resource<ogfx::ImageView>(view_desc);
    }
    else if (!rg.replace_resource<ogfx::ImageView>(m_out_image_view, view_desc))
    {
        throw std::runtime_error("Failed to recreate tone mapping output view");
    }
    else
    {
        rg.reset_resource_state(m_out_image_view);
    }
}

void ToneMappingFeature::create_resources(orendergraph::RenderGraph& rg)
{
    create_out_resources(rg);

    // Input sampler
    ogfx::SamplerDesc sampler_desc{};
    sampler_desc.min_filter = ogfx::Filter::Linear;
    sampler_desc.mag_filter = ogfx::Filter::Linear;
    sampler_desc.mip_filter = ogfx::MipmapFilter::Linear;

    sampler_desc.address_u = ogfx::AddressMode::ClampToEdge;
    sampler_desc.address_v = ogfx::AddressMode::ClampToEdge;
    sampler_desc.address_w = ogfx::AddressMode::ClampToEdge;

    sampler_desc.max_lod = 0.0f;

    m_sampler = rg.create_resource<ogfx::Sampler>(sampler_desc);
}

void ToneMappingFeature::add_node(orendergraph::RenderGraph& rg)
{
    m_tone_mapping_node = rg.add_node<orendergraph::ComputeContext>
    (
        "Tone Mapping",
        [this](orendergraph::RenderGraph& rg, orendergraph::ComputeContext::Desc& desc)
        {
            rg.read(m_desc.input_image, ogfx::ImageState::ShaderRead, {ogfx::ImageAspect::Color});
            rg.write(m_out_image, ogfx::ImageState::Common, {ogfx::ImageAspect::Color});

            const std::filesystem::path shader_path = std::filesystem::path("shaders/") / ("post_processing/tone_mapping/tone_mapping.comp." OGRAPHICS_SHADER_FORMAT);

            ogfx::ShaderDesc shader_desc{};
            shader_desc.stage = ogfx::ShaderStage::Compute;
            shader_desc.entry_point = "CSMain";
            shader_desc.bytecode = load_shader(shader_path);

            desc.compute_shader_desc = shader_desc;

            // Set 0, binding 0: HDR input
            ogfx::ResourceBindingDesc input_binding{};
            input_binding.binding = 0;
            input_binding.type = ogfx::ResourceType::SampledTexture;
            input_binding.array_count = 1;
            input_binding.stages = ogfx::ShaderStageFlags::Compute;

            // Set 0, binding 1: tone-mapped output
            ogfx::ResourceBindingDesc output_binding{};
            output_binding.binding = 1;
            output_binding.type = ogfx::ResourceType::StorageTexture;
            output_binding.array_count = 1;
            output_binding.stages = ogfx::ShaderStageFlags::Compute;

            // Set 0, binding 2: sampler
            ogfx::ResourceBindingDesc sampler_binding{};
            sampler_binding.binding = 2;
            sampler_binding.type = ogfx::ResourceType::Sampler;
            sampler_binding.array_count = 1;
            sampler_binding.stages = ogfx::ShaderStageFlags::Compute;

            ogfx::ResourceSetLayoutDesc set_layout_desc{};
            set_layout_desc.bindings = {input_binding, output_binding, sampler_binding};

            desc.resource_set_layouts_desc.push_back(set_layout_desc);

            // Push constants
            ogfx::InlineConstantRange constants_range{};
            constants_range.stages = ogfx::ShaderStageFlags::Compute;
            constants_range.offset = 0;
            constants_range.size = sizeof(ToneMappingConstants);

            desc.inline_constants_ranges.push_back(constants_range);

            // Resource set
            orendergraph::ResourceSetDesc resource_set_desc{};
            resource_set_desc.resource_set_layout_idx = 0;

            desc.resource_sets_desc.push_back(resource_set_desc);

            // Input image descriptor
            orendergraph::ImageBinding input_image_binding{};
            input_image_binding.binding = 0;
            input_image_binding.image_view = m_desc.input_image_view;
            input_image_binding.image_state = ogfx::ImageState::ShaderRead;

            // Output image descriptor
            orendergraph::ImageBinding output_image_binding{};
            output_image_binding.binding = 1;
            output_image_binding.image_view = m_out_image_view;
            output_image_binding.image_state = ogfx::ImageState::Common;

            // Sampler descriptor
            orendergraph::SamplerBinding sampler_binding_write{};
            sampler_binding_write.binding = 2;
            sampler_binding_write.sampler = m_sampler;

            orendergraph::ResourceWriteDesc write_desc{};
            write_desc.resource_set_idx = 0;
            write_desc.write_info.textures = {input_image_binding, output_image_binding};
            write_desc.write_info.samplers = {sampler_binding_write};

            desc.resource_writes_desc.push_back(write_desc);

            const ogfx::Image* input = rg.get_resource(m_desc.input_image);

            desc.group_count_x = (input->width() + workgroup_size - 1) / workgroup_size;
            desc.group_count_y = (input->height() + workgroup_size - 1) / workgroup_size;
            desc.group_count_z = 1;
        }
    );

    m_tone_mapping_node->set_execute
    (
        [this](orendergraph::RenderGraph& rg, ogfx::CommandBuffer& cmd,
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

            const ToneMappingConstants constants{m_desc.exposure, {0.0f, 0.0f, 0.0f}};

            ogfx::InlineConstantsInfo constants_info{};
            constants_info.layout = ctx.pipeline_layout;
            constants_info.stages = ogfx::ShaderStageFlags::Compute;
            constants_info.offset = 0;
            constants_info.size = sizeof(ToneMappingConstants);
            constants_info.data = &constants;

            cmd.set_inline_constants(constants_info);

            cmd.dispatch({desc.group_count_x, desc.group_count_y, desc.group_count_z});
        }
    );
}

void ToneMappingFeature::on_swapchain_recreated(orenderer::RenderFeatureContext& ctx)
{
    orendergraph::RenderGraph& rg = ctx.render_graph();

    create_out_resources(rg);

    auto& compute_ctx = m_tone_mapping_node->context();
    auto& compute_desc = m_tone_mapping_node->descriptor();

    // Update descriptors to reference the recreated views
    orendergraph::ImageBinding image_input{};
    image_input.binding = 0;
    image_input.image_view = m_desc.input_image_view;
    image_input.image_state = ogfx::ImageState::ShaderRead;

    orendergraph::ImageBinding image_output{};
    image_output.binding = 1;
    image_output.image_view = m_out_image_view;
    image_output.image_state = ogfx::ImageState::Common;

    orendergraph::ResourceWriteDesc write_desc{};
    write_desc.resource_set_idx = 0;
    write_desc.write_info.textures = {image_input, image_output};

    compute_desc.resource_writes_desc[0] = write_desc;
    compute_ctx.update_resource_set(rg, write_desc);

    // Update dispatch dimensions.
    const ogfx::Swapchain& swapchain = m_desc.graphics.swapchain();
    compute_desc.group_count_x = (swapchain.width() + workgroup_size - 1) / workgroup_size;
    compute_desc.group_count_y = (swapchain.height() + workgroup_size - 1) / workgroup_size;
    compute_desc.group_count_z = 1;
}

} // namespace omega