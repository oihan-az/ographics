#ifndef OMEGA_PBR_SPECULAR_AA_HLSLI
#define OMEGA_PBR_SPECULAR_AA_HLSLI

float filter_specular_roughness(float3 normal, float roughness)
{
    float3 N = normalize(normal);

    float3 dNdx = ddx(N);
    float3 dNdy = ddy(N);

    float variance = 0.5f * (dot(dNdx, dNdx) + dot(dNdy, dNdy));

    // Increase roughness where the normal varies rapidly
    // across the pixel footprint
    float filtered_roughness = sqrt(roughness * roughness + min(variance, 0.1f));

    return clamp(filtered_roughness, 0.04f, 1.0f);
}

#endif // OMEGA_PBR_SPECULAR_AA_HLSLI