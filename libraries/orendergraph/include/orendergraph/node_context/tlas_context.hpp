#ifndef ORENDERGRAPH_CONTEXT_TLAS_HPP
#define ORENDERGRAPH_CONTEXT_TLAS_HPP

#include <orendergraph/resource_descriptors.hpp>

#include <ogfx/shader.hpp>

namespace ogfx
{

class TLAS;
class Buffer;

} // namespace ogfx

namespace orendergraph
{

class RenderGraph;

struct TLASContext
{
    struct Desc
    {
        std::vector<AccelerationStructureInstanceDesc> instances;
        ogfx::AccelerationStructureBuildPreference build_preference;
    };

    void create(RenderGraph& graph, const Desc& desc);
    void clear();

    ResourceHandle<ogfx::TLAS> tlas_handle;
    ogfx::TLAS* tlas = nullptr;
    ogfx::Buffer* instance_buffer = nullptr;
};

} // namespace orendergraph

#endif // ORENDERGRAPH_CONTEXT_TLAS_HPP
