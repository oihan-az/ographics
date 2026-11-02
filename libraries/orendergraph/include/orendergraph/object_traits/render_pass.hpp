#ifndef ORENDERGRAPH_OBJECT_TRAITS_RENDER_PASS_HPP
#define ORENDERGRAPH_OBJECT_TRAITS_RENDER_PASS_HPP

#include <orendergraph/object_traits.hpp>

#include <ogfx/physical_device.hpp>
#include <ogfx/device.hpp>
#include <ogfx/render_pass.hpp>

#include <memory>

namespace orendergraph
{

template <> 
struct ObjectTraits<ogfx::RenderPass>
{
    using DescType = ogfx::RenderPassDesc;

    static std::unique_ptr<ogfx::RenderPass> create(const ogfx::PhysicalDevice&, const ogfx::Device& device, 
                                                    const DescType& desc)
    {
        return std::make_unique<ogfx::RenderPass>(device, desc);
    }

    static bool matches(const DescType& lhs, const DescType& rhs)
    {
        if (lhs.attachments.size() != rhs.attachments.size())
            return false;
    
        for (size_t i = 0; i < lhs.attachments.size(); ++i)
        {
            const auto& a = lhs.attachments[i];
            const auto& b = rhs.attachments[i];
    
            if (a.format != b.format ||
                a.load_op != b.load_op ||
                a.store_op != b.store_op ||
                a.initial_state != b.initial_state ||
                a.final_state != b.final_state)
                return false;
        }
    
        return true;
    }
};

} // namespace orendergraph

#endif // ORENDERGRAPH_OBJECT_TRAITS_RENDER_PASS_HPP