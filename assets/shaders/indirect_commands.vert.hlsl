struct MeshCluster
{
    float4 color;
    uint first_index;
    uint index_count;
};

[[vk::binding(0, 0)]]
StructuredBuffer<MeshCluster> clusters;

[[vk::binding(1, 0)]]
cbuffer TransformBuffer
{
    float4x4 model2w;
};

struct VertexInput
{
    [[vk::location(0)]] float3 position : POSITION;
    [[vk::location(1)]] float3 normal : NORMAL;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float3 normal : NORMAL;
    float3 color : COLOR0;
};

VertexOutput VSMain(VertexInput input, uint instance_id : SV_InstanceID)
{
    VertexOutput output;

    output.position = mul(model2w, float4(input.position, 1.0));
    output.normal = normalize(mul((float3x3) model2w, input.normal));
    output.color = clusters[instance_id].color.rgb;

    return output;
}