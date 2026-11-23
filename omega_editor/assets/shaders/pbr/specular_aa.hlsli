#ifndef OMEGA_PBR_SPECULAR_AA_HLSLI
#define OMEGA_PBR_SPECULAR_AA_HLSLI

float geometric_normal_variance(float3 normal)
{
    float3 N = normalize(normal);

    float3 dNdx = ddx(N);
    float3 dNdy = ddy(N);

    return 0.5f * (dot(dNdx, dNdx) + dot(dNdy, dNdy));
}

float normal_map_variance(float3 sampled_normal)
{
    float3 N = sampled_normal * 2.0f - 1.0f;
    float L = saturate(length(N));
    return (1.0f - L) / max(L, 0.001f);
}

float filter_specular_roughness(float roughness, float variance)
{
    float alpha = roughness * roughness;
    return clamp(sqrt(alpha + variance), 0.04f, 1.0f);
}

#endif // OMEGA_PBR_SPECULAR_AA_HLSLI