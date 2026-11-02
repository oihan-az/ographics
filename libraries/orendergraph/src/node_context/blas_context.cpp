#include <orendergraph/node_context/blas_context.hpp>

#include <orendergraph/render_graph_core.hpp>

namespace orendergraph
{

void BLASContext::create(RenderGraph& graph, const Desc& desc)
{
    clear();

    blases.reserve(desc.geometries_desc.size());
    for (const BLASContext::Desc::GeometryDesc& geometry_desc : desc.geometries_desc)
    {
        ogfx::BLASDesc blas_desc{};
  
        for (const AccelerationStructureTriangleGeometry& geom : geometry_desc.triangle_geometries)
        {
            ogfx::AccelerationStructureTriangleGeometry triangle_geometry{};
            triangle_geometry.vertex_buffer = graph.get_resource(geom.vertex_buffer);
            triangle_geometry.vertex_offset = geom.vertex_offset;
            triangle_geometry.vertex_stride = geom.vertex_stride;
            triangle_geometry.vertex_count = geom.vertex_count;
            triangle_geometry.vertex_format = geom.vertex_format;

            triangle_geometry.index_buffer = graph.get_resource(geom.index_buffer);
            triangle_geometry.index_offset = geom.index_offset;
            triangle_geometry.index_count = geom.index_count;
            triangle_geometry.index_type = geom.index_type;
            triangle_geometry.primitive_count = geom.primitive_count;

            blas_desc.triangle_geometries.push_back(triangle_geometry);
        }

        for (const AccelerationStructureAABBGeometry& geom : geometry_desc.aabb_geometries)
        {
            ogfx::AccelerationStructureAABBGeometry aabb_geometry{};
            aabb_geometry.buffer = graph.get_resource(geom.buffer);
            aabb_geometry.offset = geom.offset;
            aabb_geometry.stride = geom.stride;
            aabb_geometry.primitive_count = geom.primitive_count;

            blas_desc.aabb_geometries.push_back(aabb_geometry);
        }

        blas_desc.build_preference = geometry_desc.build_preference;

        ResourceHandle<ogfx::BLAS> blas_handle = graph.create_resource<ogfx::BLAS>(blas_desc);
        blas_handles.push_back(blas_handle);

        blases.push_back(graph.get_resource(blas_handle));

        // Set as write resource
        graph.write(blas_handle);
    }
}

void BLASContext::clear()
{
    blases.clear();
    blas_handles.clear();
}

} // namespace orendergraph
