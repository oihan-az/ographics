#ifndef OMEGA_ENVIRONMENT_MAP_COMP_HLSL
#define OMEGA_ENVIRONMENT_MAP_COMP_HLSL

struct AtmosphereConstants
{
    float4 camera_position;
    float4 sun_direction;
    float4 sun_radiance;
    float4 planet;
    float4 scattering;
    float4 settings;
};

[[vk::push_constant]]
ConstantBuffer<AtmosphereConstants> constants;

[[vk::binding(0, 0)]]
RWTexture2DArray<float4> environment_map;

static const float PI = 3.14159265359f;

static const uint VIEW_SAMPLES = 12;
static const uint SUN_SAMPLES = 6;

// Convert a cubemap face texel to a world-space direction
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

// Ray/sphere intersection for a sphere centered at the origin
bool intersect_sphere(
    float3 origin,
    float3 direction,
    float radius,
    out float t_near,
    out float t_far)
{
    float b = dot(origin, direction);
    float c = dot(origin, origin) - radius * radius;
    float discriminant = b * b - c;

    if (discriminant < 0.0f)
    {
        t_near = 0.0f;
        t_far = 0.0f;
        return false;
    }

    float root = sqrt(discriminant);

    t_near = -b - root;
    t_far = -b + root;

    return true;
}

// Returns Rayleigh and Mie density relative to sea level
float2 atmosphere_density(float3 position)
{
    float altitude = max(length(position) - constants.planet.x, 0.0f);

    float rayleigh_density = exp(-altitude / constants.planet.z);
    float mie_density = exp(-altitude / constants.planet.w);
    return float2(rayleigh_density, mie_density);
}

// Optical depth from a point toward the sun
float2 sun_optical_depth(float3 position, float3 sun_direction)
{
    float t_near, t_far;

    if (!intersect_sphere(
        position,
        sun_direction,
        constants.planet.y,
        t_near,
        t_far))
    {
        return float2(0.0f, 0.0f);
    }

    float step_size = t_far / float(SUN_SAMPLES);
    float2 optical_depth = 0.0f;

    for (uint i = 0; i < SUN_SAMPLES; ++i)
    {
        float distance = (float(i) + 0.5f) * step_size;
        float3 sample_position = position + sun_direction * distance;
        optical_depth += atmosphere_density(sample_position) * step_size;
    }

    return optical_depth;
}

float rayleigh_phase(float cos_theta)
{
    return 3.0f / (16.0f * PI) * (1.0f + cos_theta * cos_theta);
}

float mie_phase(float cos_theta, float g)
{
    float g2 = g * g;
    float denominator = max(1.0f + g2 - 2.0f * g * cos_theta, 1e-4f);
    return (1.0f - g2) / (4.0f * PI * denominator * sqrt(denominator));
}

float3 evaluate_atmosphere(float3 origin, float3 direction)
{
    float atmosphere_near, atmosphere_far;
    if (!intersect_sphere(
        origin,
        direction,
        constants.planet.y,
        atmosphere_near,
        atmosphere_far))
    {
        return 0.0f;
    }

    float start_distance = max(atmosphere_near, 0.0f);
    float end_distance = atmosphere_far;

    // Stop the view ray when it hits the planet
    float ground_near, ground_far;
    if (intersect_sphere(
        origin,
        direction,
        constants.planet.x,
        ground_near,
        ground_far))
    {
        if (ground_far > 0.0f)
            end_distance = min(end_distance, max(ground_near, 0.0f));
    }

    if (end_distance <= start_distance)
        return 0.0f;

    float3 sun_direction = normalize(constants.sun_direction.xyz);

    float3 beta_rayleigh = constants.scattering.xyz;
    float beta_mie = constants.scattering.w;

    float mu = dot(direction, sun_direction);

    float phase_rayleigh = rayleigh_phase(mu);
    float phase_mie = mie_phase(mu, constants.settings.x);

    float step_size = (end_distance - start_distance) / float(VIEW_SAMPLES);

    float2 view_depth = 0.0f;
    float3 radiance = 0.0f;

    for (uint i = 0; i < VIEW_SAMPLES; ++i)
    {
        float distance = start_distance + (float(i) + 0.5f) * step_size;
        float3 position = origin + direction * distance;
        float2 density = atmosphere_density(position);
        view_depth += density * step_size;

        // The planet may obstruct the sun
        float ground_t_near, ground_t_far;
        if (intersect_sphere(
            position,
            sun_direction,
            constants.planet.x,
            ground_t_near,
            ground_t_far))
        {
            if (ground_t_far > 0.0f && ground_t_near >= 0.0f)
                continue;
        }

        float2 sun_depth = sun_optical_depth(position, sun_direction);
        float2 total_depth = view_depth + sun_depth;
        float3 extinction = exp(-(beta_rayleigh * total_depth.x + beta_mie * total_depth.y));
        float3 scattering = beta_rayleigh * density.x * phase_rayleigh + beta_mie * density.y * phase_mie;
        radiance += extinction * scattering * step_size;
    }

    return radiance * constants.sun_radiance.rgb;
}

[numthreads(8, 8, 1)]
void CSMain(uint3 dispatch_id : SV_DispatchThreadID)
{
    uint width, height, layers;
    environment_map.GetDimensions(width, height, layers);

    if (dispatch_id.x >= width || dispatch_id.y >= height || dispatch_id.z >= 6)
        return;

    float2 uv = (float2(dispatch_id.xy) + 0.5f) / float2(width, height);
    uv = uv * 2.0f - 1.0f;

    float3 direction = cubemap_direction(dispatch_id.z, uv);
    float3 radiance = evaluate_atmosphere(constants.camera_position.xyz, direction);

    environment_map[dispatch_id] = float4(radiance, 1.0f);
}

#endif // OMEGA_ENVIRONMENT_MAP_COMP_HLSL
