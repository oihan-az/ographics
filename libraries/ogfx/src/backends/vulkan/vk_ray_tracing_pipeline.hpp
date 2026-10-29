#ifndef OGFX_VK_RAY_TRACING_HPP
#define OGFX_VK_RAY_TRACING_HPP

#include <ogfx/ray_tracing_pipeline.hpp>

#include <vulkan/vulkan.h>

namespace ogfx
{

struct RayTracingPipeline::Impl
{
    uint32_t m_raw_handle_size = 0;
    uint32_t m_handle_size = 0;

    uint32_t m_miss_count = 0;
    uint32_t m_hit_count = 0;
};

} // namespace ogfx

#endif // OGFX_VK_RAY_TRACING_HPP