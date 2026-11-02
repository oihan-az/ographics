#ifndef ORENDERGRAPH_CONTEXT_BLAS_HPP
#define ORENDERGRAPH_CONTEXT_BLAS_HPP

#include <orendergraph/resource_descriptors.hpp>

#include <ogfx/shader.hpp>

namespace ogfx
{

class BLAS;

} // namespace ogfx

namespace orendergraph
{

class RenderGraph;

struct BLASContext
{
    struct Desc
    {
        struct GeometryDesc
        {
            std::vector<AccelerationStructureTriangleGeometry> triangle_geometries;
            std::vector<AccelerationStructureAABBGeometry> aabb_geometries;
            ogfx::AccelerationStructureBuildPreference build_preference;
        };

        std::vector<GeometryDesc> geometries_desc;
    };

    void create(RenderGraph& graph, const Desc& desc);
    void clear();

    std::vector<ResourceHandle<ogfx::BLAS>> blas_handles;
    std::vector<ogfx::BLAS*> blases;
};

} // namespace orendergraph

#endif // ORENDERGRAPH_CONTEXT_BLAS_HPP
