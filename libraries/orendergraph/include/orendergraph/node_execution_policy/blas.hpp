#ifndef ORENDERGRAPH_NODE_EXECUTE_BLAS_HPP
#define ORENDERGRAPH_NODE_EXECUTE_BLAS_HPP

#include <orendergraph/node_execution_policy.hpp>
#include <orendergraph/node.hpp>
#include <orendergraph/node_context/blas_context.hpp>

#include <ogfx/command_buffer.hpp>
#include <ogfx/commands/acc_struct_commands.hpp>
#include <ogfx/commands/barrier_commands.hpp>

namespace orendergraph
{

template <> 
struct NodeExecute<BLASContext>
{
    static void execute(RenderGraph&, ogfx::CommandBuffer& cmd, RenderGraphNode<BLASContext>& node) 
    {
        BLASContext& ctx = node.context();

        for (ogfx::BLAS* blas : ctx.blases)
        {
            ogfx::AccelerationStructureBuildInfo build_info;
            build_info.destination = blas;
            cmd.build_acceleration_structure(build_info);
        }
    }
};

template <> 
struct NodePostExecute<BLASContext>
{
    static void execute(RenderGraph&, ogfx::CommandBuffer& cmd, RenderGraphNode<BLASContext>& node)
    {
        ogfx::MemoryBarrierInfo barrier_info;
        barrier_info.src_stages = ogfx::PipelineStage::AccelerationStructureBuild;
        barrier_info.dst_stages = ogfx::PipelineStage::AccelerationStructureBuild;
        barrier_info.src_access = ogfx::AccessFlags::AccelerationStructureWrite;
        barrier_info.dst_access = ogfx::AccessFlags::AccelerationStructureRead;

        cmd.memory_barrier(barrier_info);
    }
};

} // namespace orendergraph

#endif // ORENDERGRAPH_NODE_EXECUTE_BLAS_HPP