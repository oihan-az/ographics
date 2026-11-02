#ifndef ORENDERGRAPH_RESOURCE_TRAITS_TLAS_HPP
#define ORENDERGRAPH_RESOURCE_TRAITS_TLAS_HPP

#include <orendergraph/resource_traits.hpp>

#include <ogfx/physical_device.hpp>
#include <ogfx/device.hpp>
#include <ogfx/acc_struct.hpp>
#include <ogfx/command_buffer.hpp>

namespace orendergraph
{

template <> 
struct ResourceTraits<ogfx::TLAS> : DefaultResourceTraits<ogfx::TLAS>
{
    using DescType = ogfx::TLASDesc;

    static ogfx::TLAS* create(const ogfx::PhysicalDevice& physical_devic, const ogfx::Device& devic, 
                              DescType desc)
    {
        return new ogfx::TLAS(physical_devic, devic, desc);
    }
};

} // namespace orendergraph

#endif // ORENDERGRAPH_RESOURCE_TRAITS_TLAS_HPP
