#ifndef OGFX_RAY_TRACING_PIPELINE_HPP
#define OGFX_RAY_TRACING_PIPELINE_HPP

#include <ogfx/pipeline.hpp>

#include <cstdint>
#include <vector>
#include <memory>

namespace ogfx
{

class Device;
class PhysicalDevice;
class Shader;
class PipelineLayout;
class Buffer;

struct RayTracingHitShaders
{
    const Shader* closest_hit_shader = nullptr;
    const Shader* any_hit_shader = nullptr;
    const Shader* intersection_shader = nullptr;
};

struct RayTracingPipelineDesc
{
    const PipelineLayout* layout = nullptr;

    const Shader* raygen_shader = nullptr;
    std::vector<const Shader*> miss_shaders;
    std::vector<RayTracingHitShaders> hit_shaders;

    uint32_t max_recursion_depth = 1;
};

class RayTracingPipeline : public Pipeline
{
  public:
    RayTracingPipeline(const PhysicalDevice& physical_device, const Device& device, const RayTracingPipelineDesc& desc);

    ~RayTracingPipeline();

    RayTracingPipeline(const RayTracingPipeline&) = delete;
    RayTracingPipeline& operator=(const RayTracingPipeline&) = delete;

    RayTracingPipeline(RayTracingPipeline&&) noexcept;
    RayTracingPipeline& operator=(RayTracingPipeline&&) noexcept;

    uint32_t group_count() const;
    uint32_t sbt_size(uint32_t group_count) const;
    void write_handles_to_buffer(uint32_t first_group, uint32_t group_count, Buffer& output_buffer) const;

    PipelineType type() const noexcept override
    {
        return PipelineType::RayTracing;
    }

private:
    friend class CommandBuffer;

    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace ogfx

#endif // OGFX_RAY_TRACING_PIPELINE_HPP