#include <omega/graphics/pipelines/geometry_pipeline/geometry_pipeline.hpp>

namespace omega
{

void GeometryRenderPipeline::setup(GeometryRenderPipelineDesc& desc)
{
    //begin_setup();
    //
    //ogfx::BufferDesc desc;
    //desc.size = sizeof(Vertex) * geometry.vertices.size();
    //desc.usage = ogfx::BufferUsage::Vertex;
    //desc.memory_properties = ogfx::MemoryProperty::HostVisible | ogfx::MemoryProperty::HostCoherent;
    //
    //vertex_buffer_handle = registry.create_resource<ogfx::Buffer>(physical_device, device, desc);
    //ogfx::Buffer* buffer = registry.get_resource(vertex_buffer_handle);
    //
    //ogfx::BufferWriteInfo write_info;
    //write_info.data = geometry.vertices.data();
    //write_info.size = desc.size;
    //buffer->write(write_info);
    //
    //GeometryRenderFeatureDesc feature_desc;
    //
    //add_feature<GeometryRenderFeature>(feature_desc);
}

}