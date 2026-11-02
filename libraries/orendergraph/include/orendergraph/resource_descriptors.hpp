#ifndef ORENDERGRAPH_RESOURCE_DESCRIPTORS_HPP
#define ORENDERGRAPH_RESOURCE_DESCRIPTORS_HPP

#include <orendergraph/resource_handle.hpp>

#include <ogfx/buffer.hpp>
#include <ogfx/image.hpp>
#include <ogfx/image_view.hpp>
#include <ogfx/image_state.hpp>
#include <ogfx/sampler.hpp>
#include <ogfx/acc_struct.hpp>
#include <ogfx/shader.hpp>

#include <cstdint>

namespace orendergraph
{

struct ImageBinding
{
    uint32_t binding = 0;
    ResourceHandle<ogfx::ImageView> image_view;
    ogfx::ImageState image_state = ogfx::ImageState::Undefined;
};

struct SamplerBinding
{
    uint32_t binding = 0;
    ResourceHandle<ogfx::Sampler> sampler;
};

struct BufferBinding
{
    uint32_t binding = 0;
    ResourceHandle<ogfx::Buffer> buffer;
    size_t offset = 0;
    size_t range = 0;
};

struct BLASBinding
{
    uint32_t binding = 0;
    ResourceHandle<ogfx::BLAS> blas;
};

struct TLASBinding
{
    uint32_t binding = 0;
    ResourceHandle<ogfx::TLAS> tlas;
};

struct ResourceSetWriteInfo
{
    std::vector<ImageBinding> textures;
    std::vector<SamplerBinding> samplers;
    std::vector<BufferBinding> buffers;
    std::vector<BLASBinding> blases;
    std::vector<TLASBinding> tlases;
};

struct ResourceSetDesc
{
    uint32_t resource_set_layout_idx = 0;
};

struct ResourceWriteDesc
{
    uint32_t resource_set_idx = 0;
    ResourceSetWriteInfo write_info;
};

struct RayTracingHitShadersDesc
{
    ogfx::ShaderDesc closest_hit_shader_desc;
    ogfx::ShaderDesc any_hit_shader_desc;
    ogfx::ShaderDesc intersection_shader_desc;
};

struct AccelerationStructureTriangleGeometry
{
    ResourceHandle<ogfx::Buffer> vertex_buffer;
    uint64_t vertex_offset = 0;
    uint64_t vertex_stride = 0;
    uint32_t vertex_count = 0;
    ogfx::Format vertex_format = ogfx::Format::Undefined;

    ResourceHandle<ogfx::Buffer> index_buffer;
    uint64_t index_offset = 0;
    uint32_t index_count = 0;
    ogfx::IndexType index_type = ogfx::IndexType::Undefined;

    uint32_t primitive_count = 0;
};

struct AccelerationStructureAABBGeometry
{
    ResourceHandle<ogfx::Buffer> buffer;
    uint64_t offset = 0;
    uint64_t stride = sizeof(float) * 6;
    uint32_t primitive_count = 1;
};

struct  AccelerationStructureInstanceDesc    
{
    ResourceHandle<ogfx::BLAS> blas;

    float transform[3][4] = {{1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}};

    uint32_t custom_index = 0;
    uint8_t mask = 0xFF;
    uint32_t shader_binding_table_record_offset = 0;
    uint8_t flags = 0;
};

} // namespace orendergraph

#endif // ORENDERGRAPH_RESOURCE_DESCRIPTORS_HPP
