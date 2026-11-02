#ifndef ORENDERGRAPH_OBJECT_TRAITS_COMPUTE_PIPELINE_HPP
#define ORENDERGRAPH_OBJECT_TRAITS_COMPUTE_PIPELINE_HPP

#include <orendergraph/object_traits.hpp>

#include <ogfx/physical_device.hpp>
#include <ogfx/device.hpp>
#include <ogfx/compute_pipeline.hpp>

#include <memory>

namespace orendergraph
{

template <> 
struct ObjectTraits<ogfx::ComputePipeline>
{
    using DescType = ogfx::ComputePipelineDesc;

    static std::unique_ptr<ogfx::ComputePipeline> create(const ogfx::PhysicalDevice&, const ogfx::Device& device, 
                                                         const DescType& desc)
    {
        return std::make_unique<ogfx::ComputePipeline>(device, desc);
    }

    static bool matches(const DescType& lhs, const DescType& rhs)
    {
        return lhs.layout == rhs.layout &&
               lhs.shader == rhs.shader;
    }
};

} // namespace orendergraph

#endif // ORENDERGRAPH_OBJECT_TRAITS_COMPUTE_PIPELINE_HPP