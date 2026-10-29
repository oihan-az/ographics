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

[shader("intersection")]
void ISMain()
{
    float3 center = sphereData.centerRadius.xyz;
    float radius = sphereData.centerRadius.w;

    float3 origin = ObjectRayOrigin() - center;
    float3 rayDirection = ObjectRayDirection();

    float a = dot(rayDirection, rayDirection);
    float b = 2.0f * dot(origin, rayDirection);
    float c = dot(origin, origin) - radius * radius;

    float discriminant = b * b - 4.0f * a * c;

    if (discriminant < 0.0f || a <= 0.0f)
        return;

    float sqrtDiscriminant = sqrt(discriminant);

    float t0 = (-b - sqrtDiscriminant) / (2.0f * a);
    float t1 = (-b + sqrtDiscriminant) / (2.0f * a);

    float t = t0;

    if (t < RayTMin())
        t = t1;

    if (t < RayTMin() || t > RayTCurrent())
        return;

    float3 hitPosition = origin + t * rayDirection;
    float3 normal = normalize(hitPosition);

    SphereAttributes attributes;
    attributes.normal = normal;

    ReportHit(t, 0, attributes);
}
