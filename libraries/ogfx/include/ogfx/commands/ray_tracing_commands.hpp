#ifndef OGFX_RAY_TRACING_COMMANDS_HPP
#define OGFX_RAY_TRACING_COMMANDS_HPP

#include <cstdint>

namespace ogfx
{

class RayTracingPipeline;
class Buffer;

struct TraceRaysInfo
{
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t depth = 1;

    RayTracingPipeline* pipeline = nullptr;
    Buffer* sbt_buffer = nullptr;
};

} // namespace ogfx

#endif // OGFX_RAY_TRACING_COMMANDS_HPP