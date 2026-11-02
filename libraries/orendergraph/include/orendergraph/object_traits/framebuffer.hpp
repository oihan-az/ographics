#ifndef ORENDERGRAPH_OBJECT_TRAITS_FRAMEBUFFER_HPP
#define ORENDERGRAPH_OBJECT_TRAITS_FRAMEBUFFER_HPP

#include <orendergraph/object_traits.hpp>

#include <ogfx/physical_device.hpp>
#include <ogfx/device.hpp>
#include <ogfx/framebuffer.hpp>

#include <memory>

namespace orendergraph
{

template <> 
struct ObjectTraits<ogfx::Framebuffer>
{
    using DescType = ogfx::FramebufferDesc;

    static std::unique_ptr<ogfx::Framebuffer> create(const ogfx::PhysicalDevice&, const ogfx::Device& device, 
                                                     const DescType& desc)
    {
        return std::make_unique<ogfx::Framebuffer>(device, desc);
    }

    static bool matches(const DescType& lhs, const DescType& rhs)
    {
        return lhs.render_pass == rhs.render_pass &&
               lhs.attachments == rhs.attachments &&
               lhs.width == rhs.width &&
               lhs.height == rhs.height &&
               lhs.layers == rhs.layers;
    }
};

} // namespace orendergraph

#endif // ORENDERGRAPH_OBJECT_TRAITS_FRAMEBUFFER_HPP