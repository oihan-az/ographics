#include <orendergraph/node_context/ray_tracing_context.hpp>

#include <orendergraph/render_graph_core.hpp>

namespace orendergraph
{

void RayTracingContext::create(RenderGraph& graph, const Desc& desc)
{
    // Clear
    clear();

    // Pipeline context
    PipelineContext::create(graph, desc);

    // Pipeline
    ogfx::RayTracingPipelineDesc pipeline_desc{};
    pipeline_desc.layout = pipeline_layout;
    pipeline_desc.max_recursion_depth = desc.max_recursion_depth;

    // Ray generation shader
    pipeline_desc.raygen_shader = desc.raygen_shader_desc.bytecode.empty()
                                  ? nullptr
                                  : graph.get_or_create_object<ogfx::Shader>(desc.raygen_shader_desc);

    // Miss shaders
    pipeline_desc.miss_shaders.reserve(desc.miss_shader_descs.size());
    for (const auto& shader_desc : desc.miss_shader_descs)
    {
        if (shader_desc.bytecode.empty())
            continue;

        pipeline_desc.miss_shaders.push_back(graph.get_or_create_object<ogfx::Shader>(shader_desc));
    }

    // Hit shaders
    pipeline_desc.hit_shaders.reserve(desc.hit_shader_descs.size());
    for (const auto& shader_desc : desc.hit_shader_descs)
    {
        ogfx::RayTracingHitShaders hit_group{};

        hit_group.closest_hit_shader = shader_desc.closest_hit_shader_desc.bytecode.empty()
                                       ? nullptr
                                       : graph.get_or_create_object<ogfx::Shader>(shader_desc.closest_hit_shader_desc);

        hit_group.any_hit_shader = shader_desc.any_hit_shader_desc.bytecode.empty()
                                   ? nullptr
                                   : graph.get_or_create_object<ogfx::Shader>(shader_desc.any_hit_shader_desc);

        hit_group.intersection_shader = shader_desc.intersection_shader_desc.bytecode.empty()
                                        ? nullptr
                                        : graph.get_or_create_object<ogfx::Shader>(shader_desc.intersection_shader_desc);

        pipeline_desc.hit_shaders.push_back(hit_group);
    }

    // Pipeline
    pipeline = graph.get_or_create_object<ogfx::RayTracingPipeline>(pipeline_desc);

    // Shader binding table
    const uint32_t group_count = pipeline->group_count();

    ogfx::BufferDesc sbt_desc{};
    sbt_desc.size = pipeline->sbt_size(group_count);
    sbt_desc.usage = ogfx::BufferUsage::ShaderBindingTable | ogfx::BufferUsage::ShaderDeviceAddress;
    sbt_desc.memory_properties = ogfx::MemoryProperty::HostVisible | ogfx::MemoryProperty::HostCoherent;

    ResourceHandle<ogfx::Buffer> sbt_buffer_handle = graph.create_resource<ogfx::Buffer>(sbt_desc);
    sbt_buffer = graph.get_resource(sbt_buffer_handle);

    pipeline->write_handles_to_buffer(0, group_count, *sbt_buffer);
}

void RayTracingContext::clear() 
{
    PipelineContext::clear();
    pipeline = nullptr;
    sbt_buffer = nullptr;
}

} // namespace orendergraph
