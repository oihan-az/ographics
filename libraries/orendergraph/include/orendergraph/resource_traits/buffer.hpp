#ifndef ORENDERGRAPH_RESOURCE_TRAITS_BUFFER_HPP
#define ORENDERGRAPH_RESOURCE_TRAITS_BUFFER_HPP

#include <orendergraph/resource_traits.hpp>

#include <ogfx/physical_device.hpp>
#include <ogfx/device.hpp>
#include <ogfx/buffer.hpp>
#include <ogfx/pipeline_stage.hpp>
#include <ogfx/access_flags.hpp>
#include <ogfx/command_buffer.hpp>
#include <ogfx/commands/barrier_commands.hpp>

#include <cstdint>

namespace orendergraph
{

template <> 
struct ResourceTraits<ogfx::Buffer> : DefaultResourceTraits<ogfx::Buffer>
{
    using DescType = ogfx::BufferDesc;

    struct StateType
    {
        ogfx::AccessFlags access = ogfx::AccessFlags::None;
        ogfx::PipelineStage stage = ogfx::PipelineStage::TopOfPipe;
    };

    struct DataType
    {
        uint64_t offset = 0;
        uint64_t size = 0;
    };

    static ogfx::Buffer* create(const ogfx::PhysicalDevice& physical_device, const ogfx::Device& device, DescType desc)
    {
        return new ogfx::Buffer(physical_device, device, desc);
    }

    static void allocate(ogfx::Buffer* resource)
    {
        resource->allocate_memory();
        resource->bind_memory();
    }

    static void transition(ogfx::CommandBuffer& cmd, 
                           ogfx::Buffer* resource, 
                           StateType old_state, StateType new_state,
                           DataType data)
    {
        ogfx::BufferBarrierInfo barrier{};
        barrier.buffer = resource;
        barrier.memory_barrier_info.src_stages = old_state.stage;
        barrier.memory_barrier_info.dst_stages = new_state.stage;
        barrier.memory_barrier_info.src_access = old_state.access;
        barrier.memory_barrier_info.dst_access = new_state.access;
        barrier.offset = data.offset;
        barrier.size = data.size == 0 ? resource->size() - data.offset : data.size;

        cmd.buffer_barrier(barrier);
    }
};

} // namespace orendergraph

#endif // ORENDERGRAPH_RESOURCE_TRAITS_BUFFER_HPP
