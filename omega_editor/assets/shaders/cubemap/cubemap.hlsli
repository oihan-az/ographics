#ifndef OMEGA_CUBEMAP_HLSLI
#define OMEGA_CUBEMAP_HLSLI

// uv is in [-1, 1], with y increasing downward
float3 cubemap_direction(uint face, float2 uv)
{
    float x = uv.x;
    float y = uv.y;

    switch (face)
    {
        case 0:
            return normalize(float3(1.0f, -y, -x)); // +X
        case 1:
            return normalize(float3(-1.0f, -y, x)); // -X
        case 2:
            return normalize(float3(x, 1.0f, y)); // +Y
        case 3:
            return normalize(float3(x, -1.0f, -y)); // -Y
        case 4:
            return normalize(float3(x, -y, 1.0f)); // +Z
        default:
            return normalize(float3(-x, -y, -1.0f)); // -Z
    }
}

#endif // OMEGA_CUBEMAP_HLSLI
