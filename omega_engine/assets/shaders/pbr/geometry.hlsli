#ifndef OMEGA_PBR_GEOMETRY_HLSLI
#define OMEGA_PBR_GEOMETRY_HLSLI

float geometry_schlick_ggx(float NdotX, float roughness)
{
    NdotX = saturate(NdotX);
    roughness = clamp(roughness, 0.04f, 1.0f);
    
    float r = roughness + 1.0f;
    float k = (r * r) / 8.0f;

    return NdotX / (NdotX * (1.0f - k) + k);
}

float geometry_smith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = saturate(dot(N, V));
    float NdotL = saturate(dot(N, L));

    float ggx_v = geometry_schlick_ggx(NdotV, roughness);
    float ggx_l = geometry_schlick_ggx(NdotL, roughness);

    return ggx_v * ggx_l;
}

#endif // OMEGA_PBR_GEOMETRY_HLSLI