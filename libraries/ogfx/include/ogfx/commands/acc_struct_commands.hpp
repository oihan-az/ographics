#ifndef OGFX_ACCELERATION_STRUCTURE_COMMANDS_HPP
#define OGFX_ACCELERATION_STRUCTURE_COMMANDS_HPP

#include <ogfx/acc_struct_geometry.hpp>
#include <ogfx/acc_struct_build_preference.hpp>
#include <ogfx/acc_struct_instance.hpp>

#include <cstdint>
#include <vector>

namespace ogfx
{

class AccelerationStructure;
class Buffer;

struct AccelerationStructureBuildInfo
{
    AccelerationStructure* destination = nullptr;
    bool allow_update = false;
};

} // namespace ogfx

#endif // OGFX_ACCELERATION_STRUCTURE_COMMANDS_HPP