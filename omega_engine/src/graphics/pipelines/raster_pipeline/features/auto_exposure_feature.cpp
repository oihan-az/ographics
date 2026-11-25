
#include <omega/graphics/pipelines/raster_pipeline/features/auto_exposure_feature.hpp>

#include <orenderer/render_feature_context.hpp>
#include <orendergraph/render_graph.hpp>

#include <ogfx/access_flags.hpp>
#include <ogfx/buffer.hpp>
#include <ogfx/command_buffer.hpp>
#include <ogfx/commands/barrier_commands.hpp>
#include <ogfx/image.hpp>
#include <ogfx/pipeline_stage.hpp>

#include <omega/loader/loader.hpp>

#include <algorithm>
#include <cstdint>
#include <filesystem>

namespace omega
{

namespace
{

constexpr uint32_t histogram_bin_count = 256;
constexpr uint32_t histogram_workgroup_size = 16;
constexpr uint32_t sample_stride = 4;

constexpr uint64_t histogram_buffer_size = histogram_bin_count * sizeof(uint32_t);

struct HistogramConstants
{
    float min_log_luminance;
    float max_log_luminance;
    uint32_t sample_stride;
    uint32_t clear;
};

struct ExposureConstants
{
    float min_log_luminance;
    float max_log_luminance;
    float low_percentile;
    float high_percentile;

    float highlight_weight;

    float middle_gray;
    float min_exposure;
    float max_exposure;
    float delta_time;

    float speed_up;
    float speed_down;
    uint32_t has_history;
};

} // namespace

AutoExposureFeature::AutoExposureFeature(const AutoExposureFeatureDesc& desc)
    : m_desc(desc)
{
}

void AutoExposureFeature::build(orenderer::RenderFeatureContext& ctx)
{
    orendergraph::RenderGraph& rg = ctx.render_graph();

    create_resources(rg);
    add_histogram_node(rg);
    add_exposure_node(rg);
}

void AutoExposureFeature::set_delta_time(float delta_time)
{
    m_delta_time = std::clamp(delta_time, 0.0f, 0.25f);
}

void AutoExposureFeature::create_resources(orendergraph::RenderGraph& rg)
{
    // Global 256-bin histogram
    ogfx::BufferDesc histogram_desc{};
    histogram_desc.size = histogram_buffer_size;
    histogram_desc.usage = ogfx::BufferUsage::Storage;
    histogram_desc.memory_properties = ogfx::MemoryProperty::DeviceLocal;

    m_histogram_buffer = rg.create_resource<ogfx::Buffer>(histogram_desc);

    // Persistent exposure value
    ogfx::BufferDesc exposure_desc{};
    exposure_desc.size = sizeof(float);
    exposure_desc.usage = ogfx::BufferUsage::Storage;
    exposure_desc.memory_properties = ogfx::MemoryProperty::DeviceLocal;

    m_exposure_buffer = rg.create_resource<ogfx::Buffer>(exposure_desc);

    m_has_exposure_history = false;
}

void AutoExposureFeature::add_histogram_node(orendergraph::RenderGraph& rg)
{
    m_histogram_node = rg.add_node<orendergraph::ComputeContext>
    (
        "Luminance Histogram",
        [this](orendergraph::RenderGraph& rg,
               orendergraph::ComputeContext::Desc& desc)
        {
            rg.read(m_desc.input_image, ogfx::ImageState::ShaderRead, {ogfx::ImageAspect::Color});
            rg.write(m_histogram_buffer, {ogfx::AccessFlags::ShaderWrite, ogfx::PipelineStage::ComputeShader});

            const std::filesystem::path shader_path = std::filesystem::path("shaders/") / ("post_processing/tone_mapping/luminance_histogram.comp." OGRAPHICS_SHADER_FORMAT);

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

            // Set 0, binding 1: global histogram
            ogfx::ResourceBindingDesc histogram_binding{};
            histogram_binding.binding = 1;
            histogram_binding.type = ogfx::ResourceType::StorageBuffer;
            histogram_binding.array_count = 1;
            histogram_binding.stages = ogfx::ShaderStageFlags::Compute;

            ogfx::ResourceSetLayoutDesc layout_desc{};
            layout_desc.bindings = {input_binding, histogram_binding};

            desc.resource_set_layouts_desc.push_back(layout_desc);

            // Push constants
            ogfx::InlineConstantRange constants_range{};
            constants_range.stages = ogfx::ShaderStageFlags::Compute;
            constants_range.offset = 0;
            constants_range.size = sizeof(HistogramConstants);

            desc.inline_constants_ranges.push_back(constants_range);

            // Resource set
            orendergraph::ResourceSetDesc resource_set_desc{};
            resource_set_desc.resource_set_layout_idx = 0;

            desc.resource_sets_desc.push_back(resource_set_desc);

            // HDR texture binding
            orendergraph::ImageBinding input_image_binding{};
            input_image_binding.binding = 0;
            input_image_binding.image_view = m_desc.input_image_view;
            input_image_binding.image_state = ogfx::ImageState::ShaderRead;

            // Histogram buffer binding
            orendergraph::BufferBinding histogram_buffer_binding{};
            histogram_buffer_binding.binding = 1;
            histogram_buffer_binding.buffer = m_histogram_buffer;
            histogram_buffer_binding.offset = 0;
            histogram_buffer_binding.range = histogram_buffer_size;

            orendergraph::ResourceWriteDesc write_desc{};
            write_desc.resource_set_idx = 0;
            write_desc.write_info.textures = {input_image_binding};
            write_desc.write_info.buffers = {histogram_buffer_binding};

            desc.resource_writes_desc.push_back(write_desc);

            const ogfx::Image* input = rg.get_resource(m_desc.input_image);
            constexpr uint32_t tile_size = histogram_workgroup_size * sample_stride;
            desc.group_count_x = (input->width() + tile_size - 1) / tile_size;
            desc.group_count_y = (input->height() + tile_size - 1) / tile_size;
            desc.group_count_z = 1;
        }
    );

    m_histogram_node->set_execute
    (
        [this](orendergraph::RenderGraph& rg,
               ogfx::CommandBuffer& cmd,
               orendergraph::RenderGraphNode<orendergraph::ComputeContext>& node)
        {
            auto& ctx = node.context();
            const auto& desc = node.descriptor();

            ogfx::ResourceSetBindInfo bind_info{};
            bind_info.layout = ctx.pipeline_layout;
            bind_info.resource_set = ctx.resource_sets[0];
            bind_info.set_index = 0;
            bind_info.pipeline_type = ogfx::PipelineType::Compute;

            cmd.bind_resource_set(bind_info);

            ogfx::InlineConstantsInfo constants_info{};
            constants_info.layout = ctx.pipeline_layout;
            constants_info.stages = ogfx::ShaderStageFlags::Compute;
            constants_info.offset = 0;
            constants_info.size = sizeof(HistogramConstants);

            HistogramConstants constants{};
            constants.min_log_luminance = m_desc.min_log_luminance;
            constants.max_log_luminance = m_desc.max_log_luminance;
            constants.sample_stride = sample_stride;

            // Clear the histogram using the same compute shader
            constants.clear = 1;
            constants_info.data = &constants;

            cmd.set_inline_constants(constants_info);
            cmd.dispatch({1, 1, 1});

            // Ensure clearing completes before accumulation starts
            ogfx::BufferBarrierInfo barrier{};
            barrier.buffer = rg.get_resource(m_histogram_buffer);
            barrier.memory_barrier_info.src_stages = ogfx::PipelineStage::ComputeShader;
            barrier.memory_barrier_info.dst_stages = ogfx::PipelineStage::ComputeShader;
            barrier.memory_barrier_info.src_access = ogfx::AccessFlags::ShaderWrite;
            barrier.memory_barrier_info.dst_access = ogfx::AccessFlags::ShaderRead | ogfx::AccessFlags::ShaderWrite;
            barrier.offset = 0;
            barrier.size = histogram_buffer_size;

            cmd.buffer_barrier(barrier);

            // Accumulate the histogram from the HDR scene
            constants.clear = 0;

            cmd.set_inline_constants(constants_info);

            cmd.dispatch({desc.group_count_x, desc.group_count_y, desc.group_count_z});
        }
    );
}

void AutoExposureFeature::add_exposure_node(orendergraph::RenderGraph& rg)
{
    m_exposure_node = rg.add_node<orendergraph::ComputeContext>
    (
        "Auto Exposure",
        [this](orendergraph::RenderGraph& rg,
               orendergraph::ComputeContext::Desc& desc)
        {
            rg.read(m_histogram_buffer, {ogfx::AccessFlags::ShaderRead, ogfx::PipelineStage::ComputeShader});
            rg.write(m_exposure_buffer, {ogfx::AccessFlags::ShaderRead | ogfx::AccessFlags::ShaderWrite, ogfx::PipelineStage::ComputeShader});

            const std::filesystem::path shader_path = std::filesystem::path("shaders/") / ("post_processing/tone_mapping/auto_exposure.comp." OGRAPHICS_SHADER_FORMAT);

            ogfx::ShaderDesc shader_desc{};
            shader_desc.stage = ogfx::ShaderStage::Compute;
            shader_desc.entry_point = "CSMain";
            shader_desc.bytecode = load_shader(shader_path);

            desc.compute_shader_desc = shader_desc;

            // Set 0, binding 0: histogram
            ogfx::ResourceBindingDesc histogram_binding{};
            histogram_binding.binding = 0;
            histogram_binding.type = ogfx::ResourceType::StorageBuffer;
            histogram_binding.array_count = 1;
            histogram_binding.stages = ogfx::ShaderStageFlags::Compute;

            // Set 0, binding 1: persistent exposure
            ogfx::ResourceBindingDesc exposure_binding{};
            exposure_binding.binding = 1;
            exposure_binding.type = ogfx::ResourceType::StorageBuffer;
            exposure_binding.array_count = 1;
            exposure_binding.stages = ogfx::ShaderStageFlags::Compute;

            ogfx::ResourceSetLayoutDesc layout_desc{};
            layout_desc.bindings =
            {
                histogram_binding,
                exposure_binding
            };

            desc.resource_set_layouts_desc.push_back(layout_desc);

            // Push constants
            ogfx::InlineConstantRange constants_range{};
            constants_range.stages = ogfx::ShaderStageFlags::Compute;
            constants_range.offset = 0;
            constants_range.size = sizeof(ExposureConstants);

            desc.inline_constants_ranges.push_back(constants_range);

            // Resource set
            orendergraph::ResourceSetDesc resource_set_desc{};
            resource_set_desc.resource_set_layout_idx = 0;

            desc.resource_sets_desc.push_back(resource_set_desc);

            // Histogram buffer binding
            orendergraph::BufferBinding histogram_buffer_binding{};
            histogram_buffer_binding.binding = 0;
            histogram_buffer_binding.buffer = m_histogram_buffer;
            histogram_buffer_binding.offset = 0;
            histogram_buffer_binding.range = histogram_buffer_size;

            // Exposure buffer binding
            orendergraph::BufferBinding exposure_buffer_binding{};
            exposure_buffer_binding.binding = 1;
            exposure_buffer_binding.buffer = m_exposure_buffer;
            exposure_buffer_binding.offset = 0;
            exposure_buffer_binding.range = sizeof(float);

            orendergraph::ResourceWriteDesc write_desc{};
            write_desc.resource_set_idx = 0;
            write_desc.write_info.buffers =
            {
                histogram_buffer_binding,
                exposure_buffer_binding
            };

            desc.resource_writes_desc.push_back(write_desc);

            desc.group_count_x = 1;
            desc.group_count_y = 1;
            desc.group_count_z = 1;
        }
    );

    m_exposure_node->set_execute
    (
        [this](orendergraph::RenderGraph& rg,
               ogfx::CommandBuffer& cmd,
               orendergraph::RenderGraphNode<orendergraph::ComputeContext>& node)
        {
            auto& ctx = node.context();
            const auto& desc = node.descriptor();

            ogfx::ResourceSetBindInfo bind_info{};
            bind_info.layout = ctx.pipeline_layout;
            bind_info.resource_set = ctx.resource_sets[0];
            bind_info.set_index = 0;
            bind_info.pipeline_type = ogfx::PipelineType::Compute;

            cmd.bind_resource_set(bind_info);

            const ExposureConstants constants
            {
                m_desc.min_log_luminance,
                m_desc.max_log_luminance,
                m_desc.low_percentile,
                m_desc.high_percentile,

                m_desc.highlight_weight,

                m_desc.middle_gray,
                m_desc.min_exposure,
                m_desc.max_exposure,
                m_delta_time,

                m_desc.speed_up,
                m_desc.speed_down,
                m_has_exposure_history ? 1u : 0u
            };

            ogfx::InlineConstantsInfo constants_info{};
            constants_info.layout = ctx.pipeline_layout;
            constants_info.stages = ogfx::ShaderStageFlags::Compute;
            constants_info.offset = 0;
            constants_info.size = sizeof(ExposureConstants);
            constants_info.data = &constants;

            cmd.set_inline_constants(constants_info);

            cmd.dispatch({desc.group_count_x, desc.group_count_y, desc.group_count_z});

            m_has_exposure_history = true;
        }
    );
}

void AutoExposureFeature::on_swapchain_recreated(orenderer::RenderFeatureContext& ctx)
{
    orendergraph::RenderGraph& rg = ctx.render_graph();

    auto& compute_ctx = m_histogram_node->context();
    auto& compute_desc = m_histogram_node->descriptor();

    // Update the HDR input image descriptor
    orendergraph::ImageBinding image_input{};
    image_input.binding = 0;
    image_input.image_view = m_desc.input_image_view;
    image_input.image_state = ogfx::ImageState::ShaderRead;

    orendergraph::ResourceWriteDesc write_desc{};
    write_desc.resource_set_idx = 0;
    write_desc.write_info.textures = {image_input};

    compute_desc.resource_writes_desc[0] = write_desc;
    compute_ctx.update_resource_set(rg, write_desc);

    // Update histogram dispatch dimensions
    const ogfx::Image* input = rg.get_resource(m_desc.input_image);
    constexpr uint32_t tile_size = histogram_workgroup_size * sample_stride;
    compute_desc.group_count_x = (input->width() + tile_size - 1) / tile_size;
    compute_desc.group_count_y = (input->height() + tile_size - 1) / tile_size;
    compute_desc.group_count_z = 1;
}

} // namespace omega