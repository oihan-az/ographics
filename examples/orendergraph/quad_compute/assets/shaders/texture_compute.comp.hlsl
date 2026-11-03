struct ComputeConstants
{
    uint width;
    uint height;
    float t;
};

[[vk::binding(0, 0)]]
[[vk::image_format("rgba8")]]
RWTexture2D<float4> output_texture;

[[vk::push_constant]]
ConstantBuffer<ComputeConstants> constants;

float2 hash22(float2 p)
{
    float2 result = float2(
        dot(p, float2(127.1, 311.7)),
        dot(p, float2(269.5, 183.3)));

    return frac(sin(result) * 43758.5453);
}

float2 feature_point(float2 cell, float time)
{
    float2 seed = hash22(cell);
    
    float t1 = time * 0.5 + seed.x * 6.28318;
    float t2 = time * 0.85 + seed.y * 6.28318;
    float t3 = time * 1.43 + (seed.x + seed.y) * 3.14159;

    float2 motion;
    motion.x = sin(t1) * 0.6 + cos(t2 * 1.5) * 0.3 + sin(t3 * 2.1) * 0.1;
    motion.y = cos(t2) * 0.6 + sin(t1 * 1.3) * 0.3 + cos(t3 * 1.9) * 0.1;

    return 0.5 + 0.35 * motion;
}

[numthreads(8, 8, 1)]
void CSMain(uint3 dispatch_id : SV_DispatchThreadID)
{
    if (dispatch_id.x >= constants.width ||
        dispatch_id.y >= constants.height)
    {
        return;
    }

    float time = constants.t;
    float2 uv = (float2(dispatch_id.xy) + 0.5) /
                float2(constants.width, constants.height);

    float2 position = uv * 10.0;
    float2 base_cell = floor(position);
    float2 local_position = frac(position);

    float falloff = 32.0;
    float accum_caustic = 0.0;
    for (int y = -2; y <= 2; ++y)
    {
        for (int x = -2; x <= 2; ++x)
        {
            float2 neighbor = float2((float) x, (float) y);
            float2 cell = base_cell + neighbor;
            float2 p = neighbor + feature_point(cell, time);
            
            float distance_to_point = length(p - local_position);
            
            accum_caustic += exp(-falloff * distance_to_point);
        }
    }

    float caustic = log(accum_caustic) / falloff;
    caustic = saturate(exp(-caustic * 1.8) * 0.2);

    float3 deep_color = float3(0.01, 0.06, 0.12);
    float3 light_color = float3(0.35, 0.85, 1.0);
    float3 color = lerp(deep_color, light_color, caustic);
    output_texture[dispatch_id.xy] = float4(color, 1.0);
}