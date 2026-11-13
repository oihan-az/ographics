#include <orendergraph/node_context/pipeline_context.hpp>

#include <orendergraph/render_graph_core.hpp>

#include <ogfx/pipeline_layout.hpp>
#include <ogfx/resource_set.hpp>

namespace orendergraph
{

void PipelineContext::create(RenderGraph& graph, const Desc& desc)
{
    // Pipeline layout
    resource_set_layouts.clear();
    resource_set_layouts.reserve(desc.resource_set_layouts_desc.size());
    for (const ogfx::ResourceSetLayoutDesc& set_layout_desc : desc.resource_set_layouts_desc)
    {
        resource_set_layouts.push_back(graph.get_or_create_object<ogfx::ResourceSetLayout>(set_layout_desc));
    }

    ogfx::PipelineLayoutDesc pipeline_layout_desc;
    pipeline_layout_desc.set_layouts = resource_set_layouts;
    pipeline_layout_desc.inline_constant_ranges = desc.inline_constants_ranges;

    pipeline_layout = graph.get_or_create_object<ogfx::PipelineLayout>(pipeline_layout_desc);

    // Resource sets
    resource_sets.reserve(desc.resource_sets_desc.size());
    for (const ResourceSetDesc& resource_set_desc : desc.resource_sets_desc)
    {
        ogfx::ResourceSetDesc set_desc;
        set_desc.layout = pipeline_layout_desc.set_layouts[resource_set_desc.resource_set_layout_idx];

        ogfx::ResourceSet* resource_set = graph.create_object<ogfx::ResourceSet>(set_desc);
        resource_set->allocate(graph.get_resource_allocator());

        resource_sets.push_back(resource_set);
    }

    // Resource set writes
    for (const ResourceWriteDesc& resource_write_desc : desc.resource_writes_desc)
    {
        update_resource_set(graph, resource_write_desc);
    }
}

void PipelineContext::update_resource_set(RenderGraph& graph, const ResourceWriteDesc& desc)
{
    ogfx::ResourceSetWriteInfo result;

    for (const auto& binding : desc.write_info.textures)
    {
        result.textures.push_back({binding.binding, graph.get_resource(binding.image_view), binding.image_state});
    }

    for (const auto& binding : desc.write_info.samplers)
    {
        result.samplers.push_back({binding.binding, graph.get_resource(binding.sampler)});
    }

    for (const auto& binding : desc.write_info.buffers)
    {
        result.buffers.push_back({binding.binding, graph.get_resource(binding.buffer), binding.offset, binding.range});
    }

    for (const auto& binding : desc.write_info.blases)
    {
        result.acceleration_structures.push_back({binding.binding, graph.get_resource(binding.blas)});
    }

    for (const auto& binding : desc.write_info.tlases)
    {
        result.acceleration_structures.push_back({binding.binding, graph.get_resource(binding.tlas)});
    }

    resource_sets[desc.resource_set_idx]->write(result);
}

void PipelineContext::clear() 
{
    pipeline_layout = nullptr;
    resource_set_layouts.clear();
    resource_sets.clear();
}

} // namespace orendergraph
