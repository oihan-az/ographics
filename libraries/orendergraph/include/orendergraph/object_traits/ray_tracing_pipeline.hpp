#ifndef ORENDERGRAPH_OBJECT_TRAITS_RAY_TRACING_PIPELINE_HPP
#define ORENDERGRAPH_OBJECT_TRAITS_RAY_TRACING_PIPELINE_HPP

#include <orendergraph/object_traits.hpp>

#include <ogfx/physical_device.hpp>
#include <ogfx/device.hpp>
#include <ogfx/ray_tracing_pipeline.hpp>

#include <memory>

namespace orendergraph
{

template <> 
struct ObjectTraits<ogfx::RayTracingPipeline>
{
    using DescType = ogfx::RayTracingPipelineDesc;

    static std::unique_ptr<ogfx::RayTracingPipeline> create(const ogfx::PhysicalDevice& physical_device, const ogfx::Device& device, 
                                                            const DescType& desc)
    {
        return std::make_unique<ogfx::RayTracingPipeline>(physical_device, device, desc);
    }

    static bool matches(const DescType& lhs, const DescType& rhs)
    {
        if (lhs.layout != rhs.layout ||
            lhs.raygen_shader != rhs.raygen_shader ||
            lhs.miss_shaders != rhs.miss_shaders ||
            lhs.max_recursion_depth != rhs.max_recursion_depth ||
            lhs.hit_shaders.size() != rhs.hit_shaders.size())
            return false;
    
        for (size_t i = 0; i < lhs.hit_shaders.size(); ++i)
        {
            const auto& a = lhs.hit_shaders[i];
            const auto& b = rhs.hit_shaders[i];
    
            if (a.closest_hit_shader != b.closest_hit_shader ||
                a.any_hit_shader != b.any_hit_shader ||
                a.intersection_shader != b.intersection_shader)
                return false;
        }
    
        return true;
    }
};

} // namespace orendergraph

#endif // ORENDERGRAPH_OBJECT_TRAITS_RAY_TRACING_PIPELINE_HPP