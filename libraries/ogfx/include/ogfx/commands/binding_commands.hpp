#ifndef OGFX_BINDING_COMMANDS_HPP
#define OGFX_BINDING_COMMANDS_HPP

#include <ogfx/pipeline_type.hpp>
#include <ogfx/index_type.hpp>

#include <cstdint>

namespace ogfx
{

class Pipeline;
class Buffer;
class ResourceSet;
class PipelineLayout;

struct PipelineBindInfo
{
    const Pipeline* pipeline = nullptr;
};

struct VertexBufferBindInfo
{
    const Buffer* buffer = nullptr;

    uint32_t binding = 0;
    uint64_t offset = 0;
};

struct IndexBufferBindInfo
{
    const Buffer* buffer = nullptr;

    uint64_t offset = 0;
    IndexType index_type = IndexType::UInt32;
};

struct ResourceSetBindInfo
{
    const ResourceSet* resource_set = nullptr;

    const PipelineLayout* layout = nullptr;
    PipelineType pipeline_type = PipelineType::Graphics;

    uint32_t set_index = 0;
};

} // namespace ogfx

#endif // OGFX_BINDING_COMMANDS_HPP