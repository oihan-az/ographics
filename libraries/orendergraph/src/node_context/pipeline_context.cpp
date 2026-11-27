#include <orendergraph/node_context/pipeline_context.hpp>

#include <orendergraph/render_graph_core.hpp>

#include <ogfx/pipeline_layout.hpp>
#include <ogfx/resource_set.hpp>

namespace orendergraph
{

void PipelineContext::create(RenderGraph& graph, const Desc& desc)
{
    create_pipeline_layout(graph, desc);
    create_resource_sets(graph, desc);

    for (const ResourceWriteDesc& resource_write_desc : desc.resource_writes_desc)
    {
        update_resource_set(graph, resource_write_desc);
    }
}

void PipelineContext::create_pipeline_layout(RenderGraph& graph, const Desc& desc) 
{
    release_pipeline_layout(graph);

    resource_set_layouts.reserve(desc.resource_set_layouts_desc.size());
    for (const ogfx::ResourceSetLayoutDesc& set_layout_desc : desc.resource_set_layouts_desc)
    {
        resource_set_layouts.push_back(graph.get_or_create_object<ogfx::ResourceSetLayout>(set_layout_desc));
    }

    ogfx::PipelineLayoutDesc pipeline_layout_desc;
    pipeline_layout_desc.set_layouts = resource_set_layouts;
    pipeline_layout_desc.inline_constant_ranges = desc.inline_constants_ranges;

    pipeline_layout = graph.get_or_create_object<ogfx::PipelineLayout>(pipeline_layout_desc);
}

void PipelineContext::create_resource_sets(RenderGraph& graph, const Desc& desc)
{
    release_resource_sets(graph);

    resource_sets.reserve(desc.resource_sets_desc.size());
    for (const ResourceSetDesc& resource_set_desc : desc.resource_sets_desc)
    {
        ogfx::ResourceSetDesc set_desc;
        set_desc.layout = resource_set_layouts[resource_set_desc.resource_set_layout_idx];
        set_desc.variable_descriptor_count = resource_set_desc.variable_descriptor_count;

        ogfx::ResourceSet* resource_set = graph.create_object<ogfx::ResourceSet>(set_desc);
        resource_set->allocate(graph.get_resource_allocator());

        resource_sets.push_back(resource_set);
    }
}

void PipelineContext::release_pipeline_layout(RenderGraph& graph)
{
    if (pipeline_layout)
        graph.release_object(pipeline_layout);
    pipeline_layout = nullptr;

    for (const ogfx::ResourceSetLayout* resource_set_layout : resource_set_layouts)
        graph.release_object(resource_set_layout);
    resource_set_layouts.clear();
}

void PipelineContext::release_resource_sets(RenderGraph& graph) 
{
    for (const ogfx::ResourceSet* resource_set : resource_sets)
        graph.release_object(resource_set);
    resource_sets.clear();
}

void PipelineContext::replace_resource_set(RenderGraph& graph, const ResourceSetDesc& desc, uint32_t resource_set_index)
{
    // Create a new set using the existing layout
    ogfx::ResourceSetDesc set_desc{};
    set_desc.layout = resource_set_layouts[desc.resource_set_layout_idx];
    set_desc.variable_descriptor_count = desc.variable_descriptor_count;

    ogfx::ResourceSet* new_set = graph.create_object<ogfx::ResourceSet>(set_desc);
    new_set->allocate(graph.get_resource_allocator());

    // Replace only the resource set at the requested index
    ogfx::ResourceSet* old_set = resource_sets[resource_set_index];
    resource_sets[resource_set_index] = new_set;

    if (old_set)
    {
        old_set->free(graph.get_resource_allocator());
        graph.release_object(old_set);
    }
}

void PipelineContext::update_resource_set(RenderGraph& graph, const ResourceWriteDesc& desc)
{
    ogfx::ResourceSetWriteInfo result;

    for (const auto& binding : desc.write_info.textures)
    {
        result.textures.push_back({binding.binding, binding.array_element, graph.get_resource(binding.image_view), binding.image_state});
    }

    for (const auto& binding : desc.write_info.samplers)
    {
        result.samplers.push_back({binding.binding, binding.array_element, graph.get_resource(binding.sampler)});
    }

    for (const auto& binding : desc.write_info.buffers)
    {
        result.buffers.push_back({binding.binding, binding.array_element, graph.get_resource(binding.buffer), binding.offset, binding.range});
    }

    for (const auto& binding : desc.write_info.blases)
    {
        result.acceleration_structures.push_back({binding.binding, binding.array_element, graph.get_resource(binding.blas)});
    }

    for (const auto& binding : desc.write_info.tlases)
    {
        result.acceleration_structures.push_back({binding.binding, binding.array_element, graph.get_resource(binding.tlas)});
    }

    resource_sets[desc.resource_set_idx]->write(result);
}

} // namespace orendergraph
