#ifndef ORENDERGRAPH_RESOURCE_TRAITS_BLAS_HPP
#define ORENDERGRAPH_RESOURCE_TRAITS_BLAS_HPP

#include <orendergraph/resource_traits.hpp>

#include <ogfx/physical_device.hpp>
#include <ogfx/device.hpp>
#include <ogfx/acc_struct.hpp>
#include <ogfx/command_buffer.hpp>

namespace orendergraph
{

template <> 
struct ResourceTraits<ogfx::BLAS> : DefaultResourceTraits<ogfx::BLAS>
{
    using DescType = ogfx::BLASDesc;

    static ogfx::BLAS* create(const ogfx::PhysicalDevice& physical_devic, const ogfx::Device& devic, 
                              DescType desc)
    {
        return new ogfx::BLAS(physical_devic, devic, desc);
    }
};

} // namespace orendergraph

#endif // ORENDERGRAPH_RESOURCE_TRAITS_BLAS_HPP
