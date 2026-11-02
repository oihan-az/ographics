#ifndef ORENDERGRAPH_OBJECT_TRAITS_RESOURCE_SET_LAYOUT_HPP
#define ORENDERGRAPH_OBJECT_TRAITS_RESOURCE_SET_LAYOUT_HPP

#include <orendergraph/object_traits.hpp>

#include <ogfx/physical_device.hpp>
#include <ogfx/device.hpp>
#include <ogfx/resource_set_layout.hpp>

#include <memory>

namespace orendergraph
{

template <> struct ObjectTraits<ogfx::ResourceSetLayout>
{
    using DescType = ogfx::ResourceSetLayoutDesc;

    static std::unique_ptr<ogfx::ResourceSetLayout> create(const ogfx::PhysicalDevice&, const ogfx::Device& device, 
                                                           const DescType& desc)
    {
        return std::make_unique<ogfx::ResourceSetLayout>(device, desc);
    }

    static bool matches(const DescType& lhs, const DescType& rhs)
    {
        if (lhs.bindings.size() != rhs.bindings.size())
            return false;
    
        for (size_t i = 0; i < lhs.bindings.size(); ++i)
        {
            const auto& a = lhs.bindings[i];
            const auto& b = rhs.bindings[i];
    
            if (a.binding != b.binding ||
                a.type != b.type ||
                a.array_count != b.array_count ||
                a.stages != b.stages)
                return false;
        }
    
        return true;
    }
};

} // namespace orendergraph

#endif // ORENDERGRAPH_OBJECT_TRAITS_RESOURCE_SET_LAYOUT_HPP