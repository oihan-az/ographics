#ifndef ORENDERGRAPH_RESOURCE_TRAITS_IMAGE_VIEW_HPP
#define ORENDERGRAPH_RESOURCE_TRAITS_IMAGE_VIEW_HPP

#include <orendergraph/resource_traits.hpp>

#include <ogfx/physical_device.hpp>
#include <ogfx/device.hpp>
#include <ogfx/image_view.hpp>
#include <ogfx/command_buffer.hpp>

namespace orendergraph
{

template <> 
struct ResourceTraits<ogfx::ImageView> : DefaultResourceTraits<ogfx::ImageView>
{
    using DescType = ogfx::ImageViewDesc;

    static ogfx::ImageView* create(const ogfx::PhysicalDevice& physical_devic, const ogfx::Device& devic, 
                                   DescType desc)
    {
        return new ogfx::ImageView(desc);
    }
};

} // namespace orendergraph

#endif // ORENDERGRAPH_RESOURCE_TRAITS_IMAGE_VIEW_HPP
