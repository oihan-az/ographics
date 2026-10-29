#ifndef OGFX_ACCELERATION_STRUCTURE_GEOMETRY_HPP
#define OGFX_ACCELERATION_STRUCTURE_GEOMETRY_HPP

#include <ogfx/format.hpp>
#include <ogfx/index_type.hpp>

#include <cstdint>

namespace ogfx
{

class Buffer;

struct AccelerationStructureTriangleGeometry
{
    const Buffer* vertex_buffer = nullptr;
    uint64_t vertex_offset = 0;
    uint64_t vertex_stride = 0;
    uint32_t vertex_count = 0;
    Format vertex_format = Format::Undefined;

    const Buffer* index_buffer = nullptr;
    uint64_t index_offset = 0;
    uint32_t index_count = 0;
    IndexType index_type = IndexType::Undefined;

    uint32_t primitive_count = 0;
};

struct AccelerationStructureAABBGeometry
{
    Buffer* buffer = nullptr;
    uint64_t offset = 0;
    uint64_t stride = sizeof(float) * 6;
    uint32_t primitive_count = 1;
};

struct AccelerationStructureInstanceGeometry
{
    const Buffer* instance_buffer = nullptr;
    uint64_t instance_offset = 0;
    uint32_t instance_count = 0;
};

} // namespace ogfx

#endif // OGFX_ACCELERATION_STRUCTURE_GEOMETRY_HPP