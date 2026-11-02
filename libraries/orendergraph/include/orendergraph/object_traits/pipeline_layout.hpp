#ifndef ORENDERGRAPH_OBJECT_TRAITS_PIPELINE_LAYOUT_HPP
#define ORENDERGRAPH_OBJECT_TRAITS_PIPELINE_LAYOUT_HPP

#include <orendergraph/object_traits.hpp>

#include <ogfx/physical_device.hpp>
#include <ogfx/device.hpp>
#include <ogfx/pipeline_layout.hpp>

#include <memory>

namespace orendergraph
{

template <> 
struct ObjectTraits<ogfx::PipelineLayout>
{
    using DescType = ogfx::PipelineLayoutDesc;

    static std::unique_ptr<ogfx::PipelineLayout> create(const ogfx::PhysicalDevice&, const ogfx::Device& device, 
                                                        const DescType& desc)
    {
        return std::make_unique<ogfx::PipelineLayout>(device, desc);
    }

    static bool matches(const DescType& lhs, const DescType& rhs)
    {
        if (lhs.set_layouts != rhs.set_layouts ||
            lhs.inline_constant_ranges.size() != rhs.inline_constant_ranges.size())
            return false;
    
        for (size_t i = 0; i < lhs.inline_constant_ranges.size(); ++i)
        {
            const auto& a = lhs.inline_constant_ranges[i];
            const auto& b = rhs.inline_constant_ranges[i];
    
            if (a.stages != b.stages ||
                a.offset != b.offset ||
                a.size != b.size)
                return false;
        }
    
        return true;
    }
};

} // namespace orendergraph

#endif // ORENDERGRAPH_OBJECT_TRAITS_PIPELINE_LAYOUT_HPP