#ifndef OGFX_DRAW_COMMANDS_HPP
#define OGFX_DRAW_COMMANDS_HPP

#include <cstdint>

namespace ogfx
{

class Buffer;

struct DrawInfo
{
    uint32_t vertex_count = 0;
    uint32_t instance_count = 1;
    uint32_t first_vertex = 0;
    uint32_t first_instance = 0;
};

struct DrawIndexedInfo
{
    uint32_t index_count = 0;
    uint32_t instance_count = 1;
    uint32_t first_index = 0;
    int32_t vertex_offset = 0;
    uint32_t first_instance = 0;
};

struct DrawIndexedIndirectInfo
{
    const Buffer* buffer = nullptr;

    uint64_t offset = 0;
    uint32_t draw_count = 1;
    uint32_t stride = 0;
};

} // namespace ogfx

#endif // OGFX_DRAW_COMMANDS_HPP