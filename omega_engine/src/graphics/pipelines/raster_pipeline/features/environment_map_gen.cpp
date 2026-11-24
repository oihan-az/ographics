#include <omega/graphics/pipelines/raster_pipeline/features/environment_map_gen.hpp>

#include <orenderer/render_feature_context.hpp>
#include <orendergraph/render_graph.hpp>

#include <ogfx/command_buffer.hpp>
#include <ogfx/image.hpp>
#include <ogfx/image_view.hpp>
#include <ogfx/commands/barrier_commands.hpp>

#include <omega/core/component/component_system.hpp>
#include <omega/core/math/vector4.hpp>
#include <omega/graphics/graphics_system.hpp>
#include <omega/loader/loader.hpp>
#include <omega/transform/transform.hpp>
#include <omega/transform/transform_system.hpp>
#include <omega/lighting/directional_light.hpp>
#include <omega/graphics/resources/gpu_light_data.hpp>
#include <omega/atmosphere/atmosphere.hpp>
#include <omega/graphics/resources/gpu_atmosphere_data.hpp>

#include <filesystem>
#include <vector>
#include <bit>

namespace omega
{

namespace
{

constexpr uint32_t environment_resolution = 1024;
constexpr uint32_t workgroup_size = 8;
constexpr uint32_t cubemap_face_count = 6;

} // namespace

EnvironmentMapGen::EnvironmentMapGen(const EnvironmentMapGenDesc& desc) : m_desc(desc) {}

void EnvironmentMapGen::build(orenderer::RenderFeatureContext& ctx)
{
    orendergraph::RenderGraph& rg = ctx.render_graph();

    create_resources(rg);
    add_node(rg);
}

void EnvironmentMapGen::set_camera(Entity camera)
{
    m_desc.camera = camera;
}

void EnvironmentMapGen::set_atmosphere(Entity atmosphere)
{
    m_desc.atmosphere = atmosphere;
}

void EnvironmentMapGen::create_resources(orendergraph::RenderGraph& rg)
{
    ogfx::ImageDesc image_desc{};
    image_desc.width = environment_resolution;
    image_desc.height = environment_resolution;
    image_desc.format = ogfx::Format::RGBA16_FLOAT;
    image_desc.usage = ogfx::ImageUsage::Storage | ogfx::ImageUsage::Sampled | 
                       ogfx::ImageUsage::TransferSrc |ogfx::ImageUsage::TransferDst;
    image_desc.mip_levels = std::bit_width(environment_resolution);
    image_desc.array_layers = cubemap_face_count;
    image_desc.type = ogfx::ImageType::Cube;
    m_environment_map = rg.create_resource<ogfx::Image>(image_desc);

    // Storage view: compute writes to six 2D array layers
    ogfx::ImageViewDesc storage_view_desc{};
    storage_view_desc.image = rg.get_resource(m_environment_map);
    storage_view_desc.type = ogfx::ImageViewType::View2DArray;
    storage_view_desc.aspect = ogfx::ImageAspect::Color;
    storage_view_desc.base_array_layer = 0;
    storage_view_desc.array_layer_count = cubemap_face_count;
    storage_view_desc.base_mip_level = 0;
    storage_view_desc.mip_level_count = 1;
    m_environment_map_storage_view = rg.create_resource<ogfx::ImageView>(storage_view_desc);

    // Cube view: other shaders sample the environment by direction
    ogfx::ImageViewDesc cube_view_desc{};
    cube_view_desc.image = rg.get_resource(m_environment_map);
    cube_view_desc.type = ogfx::ImageViewType::Cube;
    cube_view_desc.aspect = ogfx::ImageAspect::Color;
    cube_view_desc.base_array_layer = 0;
    cube_view_desc.array_layer_count = cubemap_face_count;
    cube_view_desc.base_mip_level = 0;
    cube_view_desc.mip_level_count = image_desc.mip_levels;
    m_environment_map_view = rg.create_resource<ogfx::ImageView>(cube_view_desc);
}

void EnvironmentMapGen::add_node(orendergraph::RenderGraph& rg)
{
    m_environment_node = rg.add_node<orendergraph::ComputeContext>
    (
        "Environment Map Generation",
        [this](orendergraph::RenderGraph& rg, orendergraph::ComputeContext::Desc& desc)
        {
            rg.write(m_environment_map, ogfx::ImageState::Common, {}, ogfx::ImageState::ShaderRead, true);

            const std::filesystem::path shader_path = std::filesystem::path("shaders/") / ("environment_map/environment_map_gen.comp." OGRAPHICS_SHADER_FORMAT);

            ogfx::ShaderDesc shader_desc{};
            shader_desc.stage = ogfx::ShaderStage::Compute;
            shader_desc.entry_point = "CSMain";
            shader_desc.bytecode = load_shader(shader_path);

            desc.compute_shader_desc = shader_desc;

            // Set 0, binding 0
            ogfx::ResourceBindingDesc image_binding{};
            image_binding.binding = 0;
            image_binding.type = ogfx::ResourceType::StorageTexture;
            image_binding.array_count = 1;
            image_binding.stages = ogfx::ShaderStageFlags::Compute;

            ogfx::ResourceSetLayoutDesc set_layout_desc{};
            set_layout_desc.bindings = {image_binding};

            desc.resource_set_layouts_desc.push_back(set_layout_desc);

            // Push constants
            ogfx::InlineConstantRange constants_range{};
            constants_range.stages = ogfx::ShaderStageFlags::Compute;
            constants_range.offset = 0;
            constants_range.size = sizeof(GpuComponentData<Atmosphere>);

            desc.inline_constants_ranges.push_back(constants_range);

            // Resource set
            orendergraph::ResourceSetDesc resource_set_desc{};
            resource_set_desc.resource_set_layout_idx = 0;

            desc.resource_sets_desc.push_back(resource_set_desc);

            // Storage image descriptor write
            orendergraph::ImageBinding storage_binding{};
            storage_binding.binding = 0;
            storage_binding.image_view = m_environment_map_storage_view;
            storage_binding.image_state = ogfx::ImageState::Common;

            orendergraph::ResourceWriteDesc write_desc{};
            write_desc.resource_set_idx = 0;
            write_desc.write_info.textures = {storage_binding};

            desc.resource_writes_desc.push_back(write_desc);

            desc.group_count_x = (environment_resolution + workgroup_size - 1) / workgroup_size;
            desc.group_count_y = (environment_resolution + workgroup_size - 1) / workgroup_size;
            desc.group_count_z = cubemap_face_count;
        });

    m_environment_node->set_execute
    (
        [this](orendergraph::RenderGraph& rg, ogfx::CommandBuffer& cmd,
               orendergraph::RenderGraphNode<orendergraph::ComputeContext>& node)
        {
            orendergraph::ComputeContext& ctx = node.context();
            const auto& desc = node.descriptor();

            // Bind storage image
            ogfx::ResourceSetBindInfo bind_info{};
            bind_info.layout = ctx.pipeline_layout;
            bind_info.resource_set = ctx.resource_sets[0];
            bind_info.set_index = 0;
            bind_info.pipeline_type = ogfx::PipelineType::Compute;

            cmd.bind_resource_set(bind_info);

            // Get sun data
            const Atmosphere* atmosphere = m_desc.components.get<Atmosphere>(m_desc.atmosphere);
            const DirectionalLight* sun = m_desc.components.get<DirectionalLight>(m_desc.atmosphere);
            if (!atmosphere || !sun)
                return;

            // Convert component data into GPU constants
            const GpuComponentData<Atmosphere> constants = gpu_component_data(*atmosphere, *sun, m_desc.atmosphere, m_desc.camera, m_desc.transforms);

            ogfx::InlineConstantsInfo constants_info{};
            constants_info.layout = ctx.pipeline_layout;
            constants_info.stages = ogfx::ShaderStageFlags::Compute;
            constants_info.offset = 0;
            constants_info.size = sizeof(GpuComponentData<Atmosphere>);
            constants_info.data = &constants;

            cmd.set_inline_constants(constants_info);

            cmd.dispatch({desc.group_count_x, desc.group_count_y, desc.group_count_z});

            // Generate mipmaps
            {
                ogfx::Image* image = rg.get_resource(m_environment_map);

                ogfx::ImageTransitionInfo source_transition{};
                source_transition.image = image;
                source_transition.old_state = ogfx::ImageState::Common;
                source_transition.new_state = ogfx::ImageState::TransferDst;
                source_transition.base_mip_level = 0;
                source_transition.mip_level_count = image->mip_levels();
                source_transition.base_array_layer = 0;
                source_transition.array_layer_count = image->array_layers();  
                cmd.transition_image(source_transition);

                ogfx::GenerateMipmapsInfo mipmap_info{};
                mipmap_info.image = rg.get_resource(m_environment_map);
                cmd.generate_mipmaps(mipmap_info);
            }
        });
}

} // namespace omega