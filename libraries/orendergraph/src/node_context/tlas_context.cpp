#include <orendergraph/node_context/tlas_context.hpp>

#include <orendergraph/render_graph_core.hpp>

namespace orendergraph
{

void TLASContext::create(RenderGraph& graph, const Desc& desc)
{
    clear();

    // Instances
    std::vector<ogfx::AccelerationStructureInstance> instances;
    instances.reserve(desc.instances.size());
    for (const auto& instance_desc : desc.instances)
    {
        ogfx::AccelerationStructureInstance instance{};

        instance.acceleration_structure = graph.get_resource(instance_desc.blas);
        std::memcpy(instance.transform, instance_desc.transform, sizeof(instance.transform));
        instance.custom_index = instance_desc.custom_index;
        instance.mask = instance_desc.mask;
        instance.shader_binding_table_record_offset = instance_desc.shader_binding_table_record_offset;
        instance.flags = instance_desc.flags;

        instances.push_back(instance);
    }

    // Instance buffer
    ogfx::BufferDesc buffer_desc{};
    buffer_desc.size = ogfx::TLAS::instance_size() * instances.size();
    buffer_desc.usage = ogfx::BufferUsage::AccelerationStructureBuildInput | ogfx::BufferUsage::ShaderDeviceAddress;
    buffer_desc.memory_properties = ogfx::MemoryProperty::HostVisible | ogfx::MemoryProperty::HostCoherent;
    auto instance_buffer_handle = graph.create_resource<ogfx::Buffer>(buffer_desc);
    instance_buffer = graph.get_resource(instance_buffer_handle);

    ogfx::TLAS::write_instances_to_buffer(instances, *instance_buffer);

    // TLAS
    ogfx::TLASDesc tlas_desc{};

    ogfx::AccelerationStructureInstanceGeometry instance_geometry{};
    instance_geometry.instance_count = static_cast<uint32_t>(instances.size());
    instance_geometry.instance_buffer = instance_buffer;
    instance_geometry.instance_offset = 0;
    tlas_desc.instance_geometries.push_back(instance_geometry);

    tlas_handle = graph.create_resource<ogfx::TLAS>(tlas_desc);
    tlas = graph.get_resource(tlas_handle);

    // Set as write resource
    graph.write(tlas_handle);
}

void TLASContext::clear()
{
    tlas = nullptr;
    tlas_handle = {};
}

} // namespace orendergraph
