#ifndef OMEGA_NORMAL_MAPPING_HLSLI
#define OMEGA_NORMAL_MAPPING_HLSLI

float3 normal_mapping(
    float3 normal,
    float3 tangent,
    float3 sampled_normal,
    float normal_scale)
{
    float3 N = normalize(normal);

    // Reorthogonalize the interpolated tangent
    float3 T = normalize(tangent - N * dot(N, tangent));
    float3 B = normalize(cross(N, T));

    // Decode normal texture from [0, 1] to [-1, 1]
    float3 tangent_normal = sampled_normal * 2.0f - 1.0f;

    // Apply the normal scale
    tangent_normal.xy *= normal_scale;
    tangent_normal = normalize(tangent_normal);

    // Transform from tangent-space normal
    return normalize(tangent_normal.x * T + tangent_normal.y * B + tangent_normal.z * N);
}

#endif // OMEGA_NORMAL_MAPPING_HLSLI