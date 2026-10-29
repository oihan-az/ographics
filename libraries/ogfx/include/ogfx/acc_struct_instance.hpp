#ifndef OGFX_ACCELERATION_STRUCTURE_INSTANCE_HPP
#define OGFX_ACCELERATION_STRUCTURE_INSTANCE_HPP

#include <ogfx/acc_struct_type.hpp>
#include <ogfx/acc_struct_geometry.hpp>
#include <ogfx/acc_struct_build_preference.hpp>

#include <cstdint>
#include <memory>
#include <vector>

namespace ogfx
{

class AccelerationStructure;

struct AccelerationStructureInstance
{
    const AccelerationStructure* acceleration_structure = nullptr;

    float transform[3][4] = {{1.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}};

    uint32_t custom_index = 0;
    uint8_t mask = 0xFF;
    uint32_t shader_binding_table_record_offset = 0;
    uint8_t flags = 0;
};

} // namespace ogfx

#endif // OGFX_ACCELERATION_STRUCTURE_INSTANCE_HPP