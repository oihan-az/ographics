#ifndef ORENDERGRAPH_CONTEXT_PIPELINE_HPP
#define ORENDERGRAPH_CONTEXT_PIPELINE_HPP

#include <orendergraph/resource_descriptors.hpp>

#include <ogfx/resource_set_layout.hpp>
#include <ogfx/inline_constant_range.hpp>

#include <vector>

namespace ogfx
{

class PipelineLayout;
class ResourceSet;

}

namespace orendergraph
{

class RenderGraph;

struct PipelineContext
{
    struct Desc
    {
        std::vector<ogfx::ResourceSetLayoutDesc> resource_set_layouts_desc;
        std::vector<ogfx::InlineConstantRange> inline_constants_ranges;

        std::vector<ResourceSetDesc> resource_sets_desc;
        std::vector<ResourceWriteDesc> resource_writes_desc;
    };

    void create(RenderGraph& graph, const Desc& desc);

    void create_pipeline_layout(RenderGraph& graph, const Desc& desc);
    void create_resource_sets(RenderGraph& graph, const Desc& desc);

    void release_pipeline_layout(RenderGraph& graph);
    void release_resource_sets(RenderGraph& graph);

    void update_resource_set(RenderGraph& graph, const ResourceWriteDesc& desc);

    ogfx::PipelineLayout* pipeline_layout = nullptr;
    std::vector<const ogfx::ResourceSetLayout*> resource_set_layouts;
    std::vector<ogfx::ResourceSet*> resource_sets;
};

} // namespace orendergraph

#endif // ORENDERGRAPH_CONTEXT_PIPELINE_HPP
