#include "pbr/pbr.hlsli"
#include "pbr/specular_aa.hlsli"
#include "pbr/ibl.hlsli"
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

[[vk::binding(0, 3)]]
TextureCube<half4> prefiltered_environment;

[[vk::binding(1, 3)]]
SamplerState environment_sampler;

[[vk::binding(2, 3)]]
Texture2D<half2> brdf_lut;

[[vk::binding(3, 3)]]
SamplerState brdf_lut_sampler;

[[vk::binding(4, 3)]]
TextureCube<half4> irradiance_map;

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
    float mip_variance = 0.0f;
    if ((flags & HAS_NORMAL) != 0)
    {
        float3 sampled_normal = normal_texture.Sample(material_sampler, input.uv).xyz;
        mip_variance = normal_map_variance(sampled_normal);
        N = normal_mapping(input.normal, input.tangent, sampled_normal, constants.factors.z);
    }
    float geometric_variance = geometric_normal_variance(N);
    
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
    roughness = filter_specular_roughness(roughness, geometric_variance + mip_variance);
    roughness = clamp(roughness, 0.04f, 1.0f);

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
    float3 direct_lighting  = evaluate_pbr_lighting(
        surface,
        directional_lights,
        constants.directional_light_count,
        point_lights,
        constants.point_light_count,
        spot_lights,
        constants.spot_light_count
    );

    // Indirect specular
    float3 indirect_specular = evaluate_specular_ibl(
        surface,
        prefiltered_environment,
        environment_sampler,
        brdf_lut,
        brdf_lut_sampler
    );
    
    // Indirect diffuse
    float3 indirect_diffuse = evaluate_diffuse_ibl(
        surface,
        irradiance_map,
        environment_sampler
    );
    
    // Final color
    output.color = float4(direct_lighting + indirect_diffuse + indirect_specular + emissive, 1.0f);
    
    return output;
}