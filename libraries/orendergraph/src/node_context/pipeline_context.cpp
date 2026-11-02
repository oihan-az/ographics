#include <orendergraph/node_context/pipeline_context.hpp>

#include <orendergraph/render_graph_core.hpp>

#include <ogfx/pipeline_layout.hpp>
#include <ogfx/resource_set.hpp>

namespace orendergraph
{

void PipelineContext::create(RenderGraph& graph, const Desc& desc)
{
    // Pipeline layout
    ogfx::PipelineLayoutDesc pipeline_layout_desc;
    pipeline_layout_desc.set_layouts.reserve(desc.resource_set_layouts_desc.size());
    for (const ogfx::ResourceSetLayoutDesc& set_layout_desc : desc.resource_set_layouts_desc)
    {
        pipeline_layout_desc.set_layouts.push_back(graph.get_or_create_object<ogfx::ResourceSetLayout>(set_layout_desc));
    }
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
        ogfx::ResourceSetWriteInfo result;

        for (const auto& binding : resource_write_desc.write_info.textures)
        {
            ogfx::ImageBinding resolved;
            resolved.binding = binding.binding;
            resolved.image_view = graph.get_resource(binding.image_view);
            resolved.image_state = binding.image_state;

            result.textures.push_back(resolved);
        }

        for (const auto& binding : resource_write_desc.write_info.samplers)
        {
            ogfx::SamplerBinding resolved;
            resolved.binding = binding.binding;
            resolved.sampler = graph.get_resource(binding.sampler);

            result.samplers.push_back(resolved);
        }

        for (const auto& binding : resource_write_desc.write_info.buffers)
        {
            ogfx::BufferBinding resolved;
            resolved.binding = binding.binding;
            resolved.buffer = graph.get_resource(binding.buffer);
            resolved.offset = binding.offset;
            resolved.size = binding.range;

            result.buffers.push_back(resolved);
        }

        for (const auto& binding : resource_write_desc.write_info.blases)
        {
            ogfx::AccelerationStructureBinding resolved;
            resolved.binding = binding.binding;
            resolved.acceleration_structure = graph.get_resource(binding.blas);

            result.acceleration_structures.push_back(resolved);
        }

        for (const auto& binding : resource_write_desc.write_info.tlases)
        {
            ogfx::AccelerationStructureBinding resolved;
            resolved.binding = binding.binding;
            resolved.acceleration_structure = graph.get_resource(binding.tlas);

            result.acceleration_structures.push_back(resolved);
        }

        resource_sets[resource_write_desc.resource_set_idx]->write(result);
    }
}

void PipelineContext::clear() 
{
    pipeline_layout = nullptr;
    resource_sets.clear();
}

} // namespace orendergraph
