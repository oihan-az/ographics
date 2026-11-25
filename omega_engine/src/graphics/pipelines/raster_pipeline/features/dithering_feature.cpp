#include <omega/graphics/pipelines/raster_pipeline/features/dithering_feature.hpp>

#include <orenderer/render_feature_context.hpp>
#include <orendergraph/render_graph.hpp>

#include <ogfx/command_buffer.hpp>
#include <ogfx/image.hpp>
#include <ogfx/image_view.hpp>
#include <ogfx/resource_allocator.hpp>
#include <ogfx/swapchain.hpp>

#include <omega/graphics/graphics_system.hpp>
#include <omega/loader/loader.hpp>

#include <filesystem>
#include <stdexcept>

namespace omega
{

namespace
{

constexpr uint32_t workgroup_size = 8;

} // namespace

DitheringFeature::DitheringFeature(const DitheringFeatureDesc& desc) : m_desc(desc) {}

void DitheringFeature::build(orenderer::RenderFeatureContext& ctx)
{
    orendergraph::RenderGraph& rg = ctx.render_graph();

    ogfx::DescriptorPoolDesc pool_desc{};
    pool_desc.max_sets = 1;
    pool_desc.sizes = {{ogfx::ResourceType::SampledTexture, 1}, {ogfx::ResourceType::StorageTexture, 1}};

    rg.get_resource_allocator().create_pool(pool_desc);

    create_resources(rg);
    add_node(rg);
}

void DitheringFeature::create_resources(orendergraph::RenderGraph& rg)
{
    const ogfx::Swapchain& swapchain = m_desc.graphics.swapchain();

    // Output image
    {
        ogfx::ImageDesc image_desc{};
        image_desc.width = swapchain.width();
        image_desc.height = swapchain.height();
        image_desc.format = ogfx::Format::RGBA8_UNORM;
        image_desc.usage = ogfx::ImageUsage::Storage | ogfx::ImageUsage::TransferSrc;

        if (!m_out_image.is_valid())
        {
            m_out_image = rg.create_resource<ogfx::Image>(image_desc);
        }
        else if (!rg.replace_resource<ogfx::Image>(m_out_image, image_desc))
        {
            throw std::runtime_error("Failed to recreate dithering output image");
        }
        else
        {
            rg.reset_resource_state(m_out_image);
        }
    }

    // Output storage image view
    {
        ogfx::ImageViewDesc view_desc{};
        view_desc.image = rg.get_resource(m_out_image);
        view_desc.aspect = ogfx::ImageAspect::Color;

        if (!m_out_view.is_valid())
        {
            m_out_view = rg.create_resource<ogfx::ImageView>(view_desc);
        }
        else if (!rg.replace_resource<ogfx::ImageView>(m_out_view, view_desc))
        {
            throw std::runtime_error("Failed to recreate dithering output view");
        }
        else
        {
            rg.reset_resource_state(m_out_view);
        }
    }
}

void DitheringFeature::add_node(orendergraph::RenderGraph& rg)
{
    m_dithering_node = rg.add_node<orendergraph::ComputeContext>
    (
        "Dithering",
        [this](orendergraph::RenderGraph& rg, orendergraph::ComputeContext::Desc& desc)
        {
            rg.read(m_desc.input_image, ogfx::ImageState::ShaderRead, {ogfx::ImageAspect::Color});
            rg.write(m_out_image, ogfx::ImageState::Common, {ogfx::ImageAspect::Color});

            // Compute shader
            const std::filesystem::path shader_path = std::filesystem::path("shaders/") / ("post_processing/dithering/dithering.comp." OGRAPHICS_SHADER_FORMAT);

            ogfx::ShaderDesc shader_desc{};
            shader_desc.stage = ogfx::ShaderStage::Compute;
            shader_desc.entry_point = "CSMain";
            shader_desc.bytecode = load_shader(shader_path);

            desc.compute_shader_desc = shader_desc;

            // Set 0, binding 0: HDR input texture
            ogfx::ResourceBindingDesc input_binding{};
            input_binding.binding = 0;
            input_binding.type = ogfx::ResourceType::SampledTexture;
            input_binding.stages = ogfx::ShaderStageFlags::Compute;

            // Set 0, binding 1: output storage texture
            ogfx::ResourceBindingDesc output_binding{};
            output_binding.binding = 1;
            output_binding.type = ogfx::ResourceType::StorageTexture;
            output_binding.stages = ogfx::ShaderStageFlags::Compute;

            ogfx::ResourceSetLayoutDesc set_layout_desc{};
            set_layout_desc.bindings = {input_binding, output_binding};

            desc.resource_set_layouts_desc.push_back(set_layout_desc);

            // Resource set
            orendergraph::ResourceSetDesc set_desc{};
            set_desc.resource_set_layout_idx = 0;

            desc.resource_sets_desc.push_back(set_desc);

            // Input image descriptor
            orendergraph::ImageBinding image_input{};
            image_input.binding = 0;
            image_input.image_view = m_desc.input_image_view;
            image_input.image_state = ogfx::ImageState::ShaderRead;

            // Output image descriptor
            orendergraph::ImageBinding image_output{};
            image_output.binding = 1;
            image_output.image_view = m_out_view;
            image_output.image_state = ogfx::ImageState::Common;

            orendergraph::ResourceWriteDesc write_desc{};
            write_desc.resource_set_idx = 0;
            write_desc.write_info.textures = {image_input, image_output};

            desc.resource_writes_desc.push_back(write_desc);

            // Dispatch size
            const ogfx::Swapchain& swapchain = m_desc.graphics.swapchain();

            desc.group_count_x = (swapchain.width() + workgroup_size - 1) / workgroup_size;
            desc.group_count_y = (swapchain.height() + workgroup_size - 1) / workgroup_size;
            desc.group_count_z = 1;
        });

    m_dithering_node->set_execute
    (
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

void DitheringFeature::on_swapchain_recreated(orenderer::RenderFeatureContext& ctx)
{
    orendergraph::RenderGraph& rg = ctx.render_graph();

    create_resources(rg);

    auto& compute_ctx = m_dithering_node->context();
    auto& compute_desc = m_dithering_node->descriptor();

    // Update descriptors to reference the recreated views
    orendergraph::ImageBinding image_input{};
    image_input.binding = 0;
    image_input.image_view = m_desc.input_image_view;
    image_input.image_state = ogfx::ImageState::ShaderRead;

    orendergraph::ImageBinding image_output{};
    image_output.binding = 1;
    image_output.image_view = m_out_view;
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
