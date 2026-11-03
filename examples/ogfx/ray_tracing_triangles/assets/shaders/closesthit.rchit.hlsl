struct RayPayload
{
    float3 color;
    float3 hitPosition;
    float3 hitNormal;
    bool hitValue;
};

struct Vertex
{
    float4 position;
    float4 normal;
};

struct MeshCluster
{
    float4 color;
    uint first_index;
    uint index_count;
};

[[vk::binding(2, 0)]] StructuredBuffer<Vertex> vertices;
[[vk::binding(3, 0)]] StructuredBuffer<uint> indices;
[[vk::binding(4, 0)]] StructuredBuffer<MeshCluster> clusters;

[shader("closesthit")]
void CHMain(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    uint clusterId = InstanceID();
    MeshCluster cluster = clusters[clusterId];
    uint globalIndexOffset = cluster.first_index + (PrimitiveIndex() * 3);

    uint index0 = indices[globalIndexOffset + 0];
    uint index1 = indices[globalIndexOffset + 1];
    uint index2 = indices[globalIndexOffset + 2];
    
    float3 n0 = vertices[index0].normal.xyz;
    float3 n1 = vertices[index1].normal.xyz;
    float3 n2 = vertices[index2].normal.xyz;
    
    float3 barycentrics;
    barycentrics.y = attribs.barycentrics.x;
    barycentrics.z = attribs.barycentrics.y;
    barycentrics.x = 1.0f - barycentrics.y - barycentrics.z;
    
    float3 interpolatedNormal = n0 * barycentrics.x + n1 * barycentrics.y + n2 * barycentrics.z;
    float3 worldNormal = normalize(mul((float3x3) ObjectToWorld3x4(), interpolatedNormal));
    
    payload.hitPosition = WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
    payload.hitNormal = worldNormal;
    payload.hitValue = true;
    payload.color = cluster.color.rgb;
}