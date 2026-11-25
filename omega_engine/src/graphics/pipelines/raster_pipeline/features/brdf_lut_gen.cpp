#include <omega/graphics/pipelines/raster_pipeline/features/brdf_lut_gen.hpp>

#include <orenderer/render_feature_context.hpp>
#include <orendergraph/render_graph.hpp>

#include <ogfx/command_buffer.hpp>
#include <ogfx/image.hpp>
#include <ogfx/image_view.hpp>

#include <omega/loader/loader.hpp>

#include <cstdint>
#include <filesystem>

namespace omega
{

namespace
{

constexpr uint32_t lut_resolution = 256;
constexpr uint32_t workgroup_size = 8;

} // namespace

BrdfLutGen::BrdfLutGen(const BrdfLutGenDesc& desc) {}

void BrdfLutGen::build(orenderer::RenderFeatureContext& ctx)
{
    orendergraph::RenderGraph& rg = ctx.render_graph();

    create_resources(rg);
    add_node(rg);
}

void BrdfLutGen::create_resources(orendergraph::RenderGraph& rg)
{
    ogfx::ImageDesc image_desc{};
    image_desc.width = lut_resolution;
    image_desc.height = lut_resolution;
    image_desc.format = ogfx::Format::RG16_FLOAT;
    image_desc.usage = ogfx::ImageUsage::Storage | ogfx::ImageUsage::Sampled;
    image_desc.mip_levels = 1;
    image_desc.array_layers = 1;
    image_desc.type = ogfx::ImageType::Image2D;

    m_brdf_lut = rg.create_resource<ogfx::Image>(image_desc);

    ogfx::ImageViewDesc view_desc{};
    view_desc.image = rg.get_resource(m_brdf_lut);
    view_desc.type = ogfx::ImageViewType::View2D;
    view_desc.aspect = ogfx::ImageAspect::Color;
    view_desc.base_array_layer = 0;
    view_desc.array_layer_count = 1;
    view_desc.base_mip_level = 0;
    view_desc.mip_level_count = 1;

    m_brdf_lut_view = rg.create_resource<ogfx::ImageView>(view_desc);
}

void BrdfLutGen::add_node(orendergraph::RenderGraph& rg)
{
    m_brdf_lut_node = rg.add_node<orendergraph::ComputeContext>
    (
        "BRDF LUT Generation",
        [this](orendergraph::RenderGraph& rg, orendergraph::ComputeContext::Desc& desc)
        {
            rg.write(m_brdf_lut, ogfx::ImageState::Common, {ogfx::ImageAspect::Color});

            const std::filesystem::path shader_path = std::filesystem::path("shaders/") / ("pbr/brdf_lut_gen.comp." OGRAPHICS_SHADER_FORMAT);

            ogfx::ShaderDesc shader_desc{};
            shader_desc.stage = ogfx::ShaderStage::Compute;
            shader_desc.entry_point = "CSMain";
            shader_desc.bytecode = load_shader(shader_path);

            desc.compute_shader_desc = shader_desc;

            // Set 0, binding 0: output LUT
            ogfx::ResourceBindingDesc image_binding{};
            image_binding.binding = 0;
            image_binding.type = ogfx::ResourceType::StorageTexture;
            image_binding.array_count = 1;
            image_binding.stages = ogfx::ShaderStageFlags::Compute;

            ogfx::ResourceSetLayoutDesc set_layout_desc{};
            set_layout_desc.bindings = {image_binding};

            desc.resource_set_layouts_desc.push_back(set_layout_desc);

            // Resource set
            orendergraph::ResourceSetDesc resource_set_desc{};
            resource_set_desc.resource_set_layout_idx = 0;

            desc.resource_sets_desc.push_back(resource_set_desc);

            // Storage image descriptor write
            orendergraph::ImageBinding storage_binding{};
            storage_binding.binding = 0;
            storage_binding.image_view = m_brdf_lut_view;
            storage_binding.image_state = ogfx::ImageState::Common;

            orendergraph::ResourceWriteDesc write_desc{};
            write_desc.resource_set_idx = 0;
            write_desc.write_info.textures = {storage_binding};

            desc.resource_writes_desc.push_back(write_desc);

            desc.group_count_x = (lut_resolution + workgroup_size - 1) / workgroup_size;
            desc.group_count_y = (lut_resolution + workgroup_size - 1) / workgroup_size;
            desc.group_count_z = 1;
        }
    );

    m_brdf_lut_node->set_execution_frequency(orendergraph::ExecutionFrequency::Once);

    m_brdf_lut_node->set_execute
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

            cmd.dispatch({desc.group_count_x, desc.group_count_y, desc.group_count_z});
        });
}

} // namespace omega