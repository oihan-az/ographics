#ifndef ORENDERGRAPH_RESOURCE_TRAITS_IMAGE_HPP
#define ORENDERGRAPH_RESOURCE_TRAITS_IMAGE_HPP

#include <orendergraph/resource_traits.hpp>

#include <ogfx/physical_device.hpp>
#include <ogfx/device.hpp>
#include <ogfx/image.hpp>
#include <ogfx/image_state.hpp>
#include <ogfx/command_buffer.hpp>
#include <ogfx/commands/barrier_commands.hpp>

namespace orendergraph
{

template <> 
struct ResourceTraits<ogfx::Image> : DefaultResourceTraits<ogfx::Image>
{
    using DescType = ogfx::ImageDesc;
    using StateType = ogfx::ImageState;
    struct DataType {};

    static ogfx::Image* create(const ogfx::PhysicalDevice& physical_devic, const ogfx::Device& devic, 
                               DescType desc)
    {
        return new ogfx::Image(physical_devic, devic, desc);
    }

    static void allocate(ogfx::Image* resource)
    {
        resource->allocate_memory();
        resource->bind_memory();
    }

    static void transition(ogfx::CommandBuffer& cmd, 
                           ogfx::Image* resource, 
                           StateType old_state, StateType new_state,
                           DataType data)
    {
        ogfx::ImageTransitionInfo transition_info{};
        transition_info.image = resource;
        transition_info.old_state = old_state;
        transition_info.new_state = new_state;

        cmd.transition_image(transition_info);
    }
};

} // namespace orendergraph

#endif // ORENDERGRAPH_RESOURCE_TRAITS_IMAGE_HPP
