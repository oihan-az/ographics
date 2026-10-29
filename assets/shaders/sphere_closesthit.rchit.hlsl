struct RayPayload
{
    float3 color;
    float3 hitPosition;
    float3 hitNormal;
    bool hitValue;
};

struct SphereAttributes
{
    float3 normal;
};

struct SphereData
{
    float4 centerRadius;
    float4 color;
};

[[vk::binding(2, 0)]]
ConstantBuffer<SphereData> sphereData;

[shader("closesthit")]
void CHMain(inout RayPayload payload,
            in SphereAttributes attributes)
{
    float3 objectNormal = normalize(attributes.normal);

    payload.color = sphereData.color.rgb;
    payload.hitPosition = WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
    payload.hitNormal = normalize(mul(objectNormal, (float3x3) ObjectToWorld3x4()));
    payload.hitValue = true;
}