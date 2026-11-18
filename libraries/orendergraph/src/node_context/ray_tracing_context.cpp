#include <orendergraph/node_context/ray_tracing_context.hpp>

#include <orendergraph/render_graph_core.hpp>

namespace orendergraph
{

void RayTracingContext::create(RenderGraph& graph, const Desc& desc)
{
    PipelineContext::create(graph, desc);
    create_shaders(graph, desc);
    create_pipeline(graph, desc);
    create_stb_buffer(graph, desc);
}

void RayTracingContext::create_pipeline(RenderGraph& graph, const Desc& desc)
{
    release_pipeline(graph);

    ogfx::RayTracingPipelineDesc pipeline_desc{};
    pipeline_desc.layout = pipeline_layout;
    pipeline_desc.max_recursion_depth = desc.max_recursion_depth;

    pipeline_desc.raygen_shader = raygen_shader;
    pipeline_desc.miss_shaders = miss_shaders;
    pipeline_desc.hit_shaders.reserve(hit_shaders.size());
    for (ogfx::RayTracingHitShaders& hit_shaders : hit_shaders)
    {
        pipeline_desc.hit_shaders.push_back(hit_shaders);
    }

    pipeline = graph.get_or_create_object<ogfx::RayTracingPipeline>(pipeline_desc);
}

void RayTracingContext::create_shaders(RenderGraph& graph, const Desc& desc)
{
    release_shaders(graph);

    // Ray generation shader
    raygen_shader = desc.raygen_shader_desc.bytecode.empty()
                                  ? nullptr
                                  : graph.get_or_create_object<ogfx::Shader>(desc.raygen_shader_desc);

    // Miss shaders
    miss_shaders.reserve(desc.miss_shader_descs.size());
    for (const auto& shader_desc : desc.miss_shader_descs)
    {
        if (shader_desc.bytecode.empty())
            continue;

        miss_shaders.push_back(graph.get_or_create_object<ogfx::Shader>(shader_desc));
    }

    // Hit shaders
    hit_shaders.reserve(desc.hit_shader_descs.size());
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

        hit_shaders.push_back(hit_group);
    }
}

void RayTracingContext::create_stb_buffer(RenderGraph& graph, const Desc& desc)
{
    release_stb_buffer(graph);

    const uint32_t group_count = pipeline->group_count();

    ogfx::BufferDesc sbt_desc{};
    sbt_desc.size = pipeline->sbt_size(group_count);
    sbt_desc.usage = ogfx::BufferUsage::ShaderBindingTable | ogfx::BufferUsage::ShaderDeviceAddress;
    sbt_desc.memory_properties = ogfx::MemoryProperty::HostVisible | ogfx::MemoryProperty::HostCoherent;

    sbt_buffer_handle = graph.create_resource<ogfx::Buffer>(sbt_desc);
    sbt_buffer = graph.get_resource(sbt_buffer_handle);

    pipeline->write_handles_to_buffer(0, group_count, *sbt_buffer);
}

void RayTracingContext::release_shaders(RenderGraph& graph)
{
    if (raygen_shader)
        graph.release_object(raygen_shader);
    raygen_shader = nullptr;

    if (!miss_shaders.empty())
    {
        for (const ogfx::Shader* shader : miss_shaders)
            graph.release_object(shader);
    }
    miss_shaders.clear();

    if (!hit_shaders.empty())
    {
        for (ogfx::RayTracingHitShaders& hit_shaders : hit_shaders)
        {
            if (hit_shaders.closest_hit_shader)
                graph.release_object(hit_shaders.closest_hit_shader);

            if (hit_shaders.any_hit_shader)
                graph.release_object(hit_shaders.any_hit_shader);

            if (hit_shaders.intersection_shader)
                graph.release_object(hit_shaders.intersection_shader);
        }
    }
    hit_shaders.clear();
}

void RayTracingContext::release_pipeline(RenderGraph& graph)
{
    if (pipeline)
        graph.release_object(pipeline);
    pipeline = nullptr;
}

void RayTracingContext::release_stb_buffer(RenderGraph& graph)
{
    if (sbt_buffer)
        graph.release_resource<ogfx::Buffer>(sbt_buffer_handle);
    sbt_buffer = nullptr;
    sbt_buffer_handle = {};
}

} // namespace orendergraph
