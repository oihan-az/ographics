#ifndef ORENDERGRAPH_RESOURCE_TRAITS_HPP
#define ORENDERGRAPH_RESOURCE_TRAITS_HPP

namespace ogfx
{

class PhysicalDevice;
class Device;
class CommandBuffer;

}

namespace orendergraph
{

struct EmptyResourceDesc {};
struct EmptyResourceState {};
struct EmptyResourceData {};

template <typename T> 
struct DefaultResourceTraits
{
    using DescType = EmptyResourceDesc;
    using DataType = EmptyResourceState;
    using StateType = EmptyResourceData;

    static T* create(const ogfx::PhysicalDevice& physical_device, const ogfx::Device& device, 
                           DescType desc)
    {
        return nullptr;
    }

    static void allocate(T* resource)
    {
        // Empty
    }

    static void transition(ogfx::CommandBuffer& cmd, 
                           T* resource, 
                           StateType old_state, StateType new_state,
                           DataType data)
    {
        // Empty
    }
};

template <typename T>
struct ResourceTraits : DefaultResourceTraits<T> {};

} // namespace orendergraph

#endif // ORENDERGRAPH_RESOURCE_TRAITS_HPP