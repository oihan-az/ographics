#ifndef ORENDERGRAPH_NODE_EXECUTE_TLAS_HPP
#define ORENDERGRAPH_NODE_EXECUTE_TLAS_HPP

#include <orendergraph/node_execution_policy.hpp>
#include <orendergraph/node.hpp>
#include <orendergraph/node_context/tlas_context.hpp>

#include <ogfx/command_buffer.hpp>
#include <ogfx/commands/acc_struct_commands.hpp>

namespace orendergraph
{

template <> 
struct NodeExecute<TLASContext>
{
    static void execute(RenderGraph&, ogfx::CommandBuffer& cmd, RenderGraphNode<TLASContext>& node)
    {
        TLASContext& ctx = node.context();

        ogfx::AccelerationStructureBuildInfo build_info;
        build_info.destination = ctx.tlas;
        cmd.build_acceleration_structure(build_info);
    }
};

template <> 
struct NodePostExecute<TLASContext>
{
    static void execute(RenderGraph&, ogfx::CommandBuffer& cmd, RenderGraphNode<TLASContext>& node)
    {
        ogfx::MemoryBarrierInfo barrier_info;
        barrier_info.src_stages = ogfx::PipelineStage::AccelerationStructureBuild;
        barrier_info.dst_stages = ogfx::PipelineStage::RayTracingShader;
        barrier_info.src_access = ogfx::AccessFlags::AccelerationStructureWrite;
        barrier_info.dst_access = ogfx::AccessFlags::AccelerationStructureRead;

        cmd.memory_barrier(barrier_info);
    }
};

} // namespace orendergraph

#endif // ORENDERGRAPH_NODE_EXECUTE_TLAS_HPP