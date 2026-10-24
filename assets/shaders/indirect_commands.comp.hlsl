struct MeshCluster
{
    float4 color;
    uint first_index;
    uint index_count;
};

struct DrawIndexedIndirectCommand
{
    uint index_count;
    uint instance_count;
    uint first_index;
    int vertex_offset;
    uint first_instance;
};

[[vk::binding(0, 0)]]
StructuredBuffer<MeshCluster> clusters;

[[vk::binding(1, 0)]]
RWStructuredBuffer<DrawIndexedIndirectCommand> indirect_commands;

[numthreads(1, 1, 1)]
void CSMain(uint3 dispatch_id : SV_DispatchThreadID)
{
    uint cluster_index = dispatch_id.x;

    MeshCluster cluster = clusters[cluster_index];

    DrawIndexedIndirectCommand command;
    command.index_count = cluster.index_count;
    command.instance_count = 1;
    command.first_index = cluster.first_index;
    command.vertex_offset = 0;
    command.first_instance = cluster_index;

    indirect_commands[cluster_index] = command;
}