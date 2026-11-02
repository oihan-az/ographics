#ifndef ORENDERGRAPH_RESOURCE_TRAITS_SAMPLER_HPP
#define ORENDERGRAPH_RESOURCE_TRAITS_SAMPLER_HPP

#include <orendergraph/resource_traits.hpp>

#include <ogfx/physical_device.hpp>
#include <ogfx/device.hpp>
#include <ogfx/sampler.hpp>
#include <ogfx/command_buffer.hpp>

namespace orendergraph
{

template <> 
struct ResourceTraits<ogfx::Sampler> : DefaultResourceTraits<ogfx::Sampler>
{
    using DescType = ogfx::SamplerDesc;

    static ogfx::Sampler* create(const ogfx::PhysicalDevice& physical_devic, const ogfx::Device& devic, 
                                 DescType desc)
    {
        return new ogfx::Sampler(devic, desc);
    }
};

} // namespace orendergraph

#endif // ORENDERGRAPH_RESOURCE_TRAITS_SAMPLER_HPP
