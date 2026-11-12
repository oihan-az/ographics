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
    void update_resource_set(RenderGraph& graph, const ResourceWriteDesc& desc);
    void clear();

    ogfx::PipelineLayout* pipeline_layout = nullptr;
    std::vector<ogfx::ResourceSet*> resource_sets;
};

} // namespace orendergraph

#endif // ORENDERGRAPH_CONTEXT_PIPELINE_HPP
