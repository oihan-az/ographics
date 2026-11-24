#ifndef OMEGA_PBR_IMPORTANCE_SAMPLING_HLSLI
#define OMEGA_PBR_IMPORTANCE_SAMPLING_HLSLI

static const float PI = 3.14159265359f;

// Van der Corput radical inverse, base 2
float radical_inverse_vdc(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) |
           ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) |
           ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) |
           ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) |
           ((bits & 0xFF00FF00u) >> 8u);

    return float(bits) * 2.3283064365386963e-10f;
}

// Low-discrepancy sample in [0, 1]^2
float2 hammersley(uint i, uint sample_count)
{
    return float2(float(i) / float(sample_count), radical_inverse_vdc(i));
}

// Importance-sample a GGX half-vector around normal N
float3 importance_sample_ggx(float2 xi, float3 N, float roughness)
{
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;

    float phi = 2.0f * PI * xi.x;
    float cos_theta = sqrt((1.0f - xi.y) / (1.0f + (alpha2 - 1.0f) * xi.y));
    float sin_theta = sqrt(max(1.0f - cos_theta * cos_theta, 0.0f));

    // Half-vector in tangent space
    float3 H = float3(cos(phi) * sin_theta, sin(phi) * sin_theta, cos_theta);

    // Construct an orthonormal basis around N
    float3 up = abs(N.z) < 0.999f ? float3(0.0f, 0.0f, 1.0f) : float3(1.0f, 0.0f, 0.0f);
    float3 tangent = normalize(cross(up, N));
    float3 bitangent = cross(N, tangent);

    return normalize(tangent * H.x + bitangent * H.y + N * H.z);
}

float ggx_sample_lod(float3 N, float3 H, float3 V, float roughness, uint sample_count, float source_resolution)
{
    float NdotH = saturate(dot(N, H));
    float HdotV = max(dot(H, V), 1e-5f);

    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;

    float denominator = NdotH * NdotH * (alpha2 - 1.0f) + 1.0f;

    float D = alpha2 / max(PI * denominator * denominator, 1e-7f);

    // Convert half-vector sampling PDF into light-direction PDF
    float pdf = max(D * NdotH / (4.0f * HdotV), 1e-6f);

    // Approximate solid angle of one source cubemap texel
    float texel_solid_angle = 4.0f * PI / (6.0f * source_resolution * source_resolution);

    // Solid angle represented by an importance sample
    float sample_solid_angle = 1.0f / (float(sample_count) * pdf);

    return roughness <= 0.0f ? 0.0f : max(0.5f * log2(sample_solid_angle / texel_solid_angle), 0.0f);
}

#endif // OMEGA_PBR_IMPORTANCE_SAMPLING_HLSLI