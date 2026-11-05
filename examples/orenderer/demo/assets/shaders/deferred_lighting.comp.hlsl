struct LightingConstants
{
    uint width;
    uint height;
    float ambient_strength;
    float padding;
    float4 light_direction;
    float4 light_color;
};

[[vk::binding(0, 0)]]
Texture2D<float4> albedo_texture;

[[vk::binding(1, 0)]]
Texture2D<float4> normal_texture;

[[vk::binding(2, 0)]]
Texture2D<float> depth_texture;

[[vk::binding(3, 0)]]
[[vk::image_format("rgba16f")]]
RWTexture2D<float4> output_texture;

[[vk::push_constant]]
ConstantBuffer<LightingConstants> constants;

[numthreads(8, 8, 1)]
void CSMain(uint3 dispatch_id : SV_DispatchThreadID)
{
    uint2 pixel = dispatch_id.xy;

    float depth = depth_texture.Load(int3(pixel, 0));
    if (depth >= 1.0)
    {
        output_texture[pixel] = float4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    float3 albedo = albedo_texture.Load(int3(pixel, 0)).rgb;
    float3 normal = normalize(normal_texture.Load(int3(pixel, 0)).xyz);

    float3 light_direction = normalize(-constants.light_direction.xyz);

    float diffuse = max(dot(normal, light_direction), 0.0);

    float3 ambient = albedo * constants.ambient_strength;
    float3 direct = albedo * constants.light_color.rgb * diffuse;

    float3 color = ambient + direct;

    output_texture[pixel] = float4(color, 1.0);
}