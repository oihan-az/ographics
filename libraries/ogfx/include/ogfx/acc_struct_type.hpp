#ifndef OGFX_ACCELERATION_STRUCTURE_TYPE_HPP
#define OGFX_ACCELERATION_STRUCTURE_TYPE_HPP

#include <cstdint>

namespace ogfx
{

enum class AccelerationStructureType : uint8_t
{
    Undefined = 0,
    BottomLevel,
    TopLevel
};

} // namespace ogfx

#endif // OGFX_ACCELERATION_STRUCTURE_TYPE_HPP