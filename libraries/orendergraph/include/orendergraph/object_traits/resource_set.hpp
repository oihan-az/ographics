#ifndef ORENDERGRAPH_OBJECT_TRAITS_RESOURCE_SET_HPP
#define ORENDERGRAPH_OBJECT_TRAITS_RESOURCE_SET_HPP

#include <orendergraph/object_traits.hpp>

#include <ogfx/physical_device.hpp>
#include <ogfx/device.hpp>
#include <ogfx/resource_set.hpp>

#include <memory>

namespace orendergraph
{

template <> struct ObjectTraits<ogfx::ResourceSet>
{
    using DescType = ogfx::ResourceSetDesc;

    static std::unique_ptr<ogfx::ResourceSet> create(const ogfx::PhysicalDevice&, const ogfx::Device& device, 
                                                     const DescType& desc)
    {
        return std::make_unique<ogfx::ResourceSet>(device, desc);
    }

    static bool matches(const DescType& lhs, const DescType& rhs)
    {
        return lhs.layout == rhs.layout;
    }
};

} // namespace orendergraph

#endif // ORENDERGRAPH_OBJECT_TRAITS_RESOURCE_SET_HPP