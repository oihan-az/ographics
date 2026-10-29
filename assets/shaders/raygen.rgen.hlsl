struct RayPayload
{
    float3 color;
    float3 hitPosition;
    float3 hitNormal;
    bool hitValue;
};

[[vk::binding(0, 0)]] RaytracingAccelerationStructure TLAS;
[[vk::binding(1, 0)]] RWTexture2D<float4> outputImage;

[shader("raygeneration")]
void RGMain()
{
    uint2 launchId = DispatchRaysIndex().xy;
    uint2 launchSize = DispatchRaysDimensions().xy;

    // Calculate Normalized Device Coordinates [-1, 1]
    float2 ndc = (float2(launchId) / float2(launchSize)) * 2.0f - 1.0f;
    ndc.y = -ndc.y;

    // Setup a simple orthographic/perspective ray direction
    float3 cameraPos = float3(0.0f, 0.0f, -3.0f);
    float3 rayDir = normalize(float3(ndc.x * 1.77f, ndc.y, 1.0f));

    RayDesc ray;
    ray.Origin = cameraPos;
    ray.Direction = rayDir;
    ray.TMin = 0.001f;
    ray.TMax = 1000.0f;

    RayPayload payload;
    payload.color = float3(0.0f, 0.0f, 0.0f);
    payload.hitPosition = float3(0.0f, 0.0f, 0.0f);
    payload.hitNormal = float3(0.0f, 0.0f, 0.0f);
    payload.hitValue = false;

    // Trace the ray through the Top-Level Acceleration Structure
    TraceRay(TLAS, 0, 0xFF, 0, 1, 0, ray, payload);

    if (payload.hitValue)
    {
        float3 lightPos = float3(5.0f, 5.0f, -5.0f);
        float3 lightDir = normalize(lightPos - payload.hitPosition);
        
        float diffuse = max(dot(payload.hitNormal, lightDir), 0.0f);
        float3 ambient = float3(0.1f, 0.1f, 0.15f);

        float3 finalColor = ambient + (payload.color * diffuse);

        outputImage[launchId] = float4(finalColor, 1.0f);
    }
    else
    {
        outputImage[launchId] = float4(payload.color, 1.0f);
    }
}