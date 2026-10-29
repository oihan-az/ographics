struct RayPayload
{
    float3 color;
    float3 hitPosition;
    float3 hitNormal;
    bool hitValue;
};

[shader("miss")]
void MSMain(inout RayPayload payload)
{
    float t = 0.5f * (WorldRayDirection().y + 1.0f);
    float3 skyColor = (1.0f - t) * float3(1.0f, 1.0f, 1.0f) + t * float3(0.3f, 0.5f, 0.9f);
    
    payload.color = skyColor;
    payload.hitValue = false;
}