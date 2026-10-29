#ifndef OGFX_RESOURCE_TYPE_HPP
#define OGFX_RESOURCE_TYPE_HPP

#include <cstdint>

namespace ogfx
{

enum class ResourceType : uint32_t
{
    Undefined,
    UniformBuffer,
    StorageBuffer,
    SampledTexture,
    StorageTexture,
    Sampler,
    AccelerationStructure
};

} // namespace ogfx

#endif // OGFX_RESOURCE_TYPE_HPP
