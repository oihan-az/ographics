#include "pbr/pbr.hlsli"
#include "pbr/specular_aa.hlsli"
#include "normal_mapping/normal_mapping.hlsli"

struct PSInput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 world_position : POSITION1;
};

struct PSOutput
{
    float4 color : SV_TARGET0;
};

[[vk::binding(0, 0)]]
StructuredBuffer<DirectionalLight> directional_lights;

[[vk::binding(1, 0)]]
StructuredBuffer<PointLight> point_lights;

[[vk::binding(2, 0)]]
StructuredBuffer<SpotLight> spot_lights;

[[vk::binding(0, 1)]]
Texture2D<float4> base_color_texture;

[[vk::binding(1, 1)]]
Texture2D<float4> metallic_roughness_texture;

[[vk::binding(2, 1)]]
Texture2D<float4> normal_texture;

[[vk::binding(3, 1)]]
Texture2D<float4> occlusion_texture;

[[vk::binding(4, 1)]]
Texture2D<float4> emissive_texture;

[[vk::binding(0, 2)]]
SamplerState material_sampler;

struct FragmentConstants
{
    // Lighting
    [[vk::offset(128)]]
    uint directional_light_count;

    uint point_light_count;
    uint spot_light_count;
    uint padding;

    // PBR material
    float4 factors; // Metallic, roughness, normal scale, AO strength
    float4 base_color;
    float4 emissive;

    uint texture_flags;
    uint padding0;
    uint padding1;
    uint padding2;
    
    float4 camera_position;
};

[[vk::push_constant]]
ConstantBuffer<FragmentConstants> constants;

static const uint HAS_BASE_COLOR = 1u << 0;
static const uint HAS_METALLIC_ROUGHNESS = 1u << 1;
static const uint HAS_NORMAL = 1u << 2;
static const uint HAS_OCCLUSION = 1u << 3;
static const uint HAS_EMISSIVE = 1u << 4;

PSOutput PSMain(PSInput input)
{
    PSOutput output;

    const uint flags = constants.texture_flags;

    // Base color
    float4 base_color = constants.base_color;
    if ((flags & HAS_BASE_COLOR) != 0)
    {
        base_color *= base_color_texture.Sample(material_sampler, input.uv);
    }

    // Normal
    float3 N = normalize(input.normal);
    if ((flags & HAS_NORMAL) != 0)
    {
        float3 sampled_normal = normal_texture.Sample(material_sampler, input.uv).xyz;
        N = normal_mapping(input.normal, input.tangent, sampled_normal, constants.factors.z);
    }
    
    // Metallic and roughness
    float metallic = constants.factors.x;
    float roughness = constants.factors.y;
    if ((flags & HAS_METALLIC_ROUGHNESS) != 0)
    {
        float4 mr = metallic_roughness_texture.Sample(material_sampler, input.uv);
        roughness *= mr.g;
        metallic *= mr.b;
    }
    metallic = saturate(metallic);
    roughness = filter_specular_roughness(N, roughness);
    
    // Emissive
    float3 emissive = constants.emissive.rgb;
    if ((flags & HAS_EMISSIVE) != 0)
    {
        emissive *= emissive_texture.Sample(material_sampler, input.uv).rgb;
    }

    // PBR surface
    PbrSurface surface;
    surface.position = input.world_position;
    surface.normal = N;
    surface.base_color = base_color.rgb;
    surface.metallic = metallic;
    surface.roughness = roughness;
    surface.view_direction = normalize(constants.camera_position.xyz - input.world_position);

    // Direct lighting
    float3 lighting = evaluate_pbr_lighting(
        surface,
        directional_lights,
        constants.directional_light_count,
        point_lights,
        constants.point_light_count,
        spot_lights,
        constants.spot_light_count
    );

    // Final color
    const float ambient = 0.4f;
    output.color = float4(lighting + emissive + (ambient * base_color.rgb), base_color.a);

    return output;
}