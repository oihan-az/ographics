#ifndef OGFX_ACCELERATION_STRUCTURE_BUILD_PREFERENCE_HPP
#define OGFX_ACCELERATION_STRUCTURE_BUILD_PREFERENCE_HPP

#include <cstdint>

namespace ogfx
{

enum class AccelerationStructureBuildPreference : uint8_t
{
    Default = 0,
    PreferFastTrace,
    PreferFastBuild
};

} // namespace ogfx

#endif // OGFX_ACCELERATION_STRUCTURE_BUILD_PREFERENCE_HPP