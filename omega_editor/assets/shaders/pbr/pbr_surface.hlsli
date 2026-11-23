#ifndef OMEGA_PBR_SURFACE_HLSLI
#define OMEGA_PBR_SURFACE_HLSLI

struct PbrSurface
{
    float3 position;
    float3 normal;

    float3 base_color;

    float metallic;
    float roughness;

    float3 view_direction;
};

#endif // OMEGA_PBR_SURFACE_HLSLI