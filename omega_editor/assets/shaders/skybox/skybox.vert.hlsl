struct SkyboxConstants
{
    float4x4 view_projection;
};

[[vk::push_constant]]
ConstantBuffer<SkyboxConstants> constants;

struct VSOutput
{
    float4 position : SV_Position;
    float3 direction : TEXCOORD0;
};

static const float3 cube_vertices[36] =
{
    // -Z
    float3(-1.0f, -1.0f, -1.0f),
    float3(1.0f, 1.0f, -1.0f),
    float3(1.0f, -1.0f, -1.0f),

    float3(-1.0f, -1.0f, -1.0f),
    float3(-1.0f, 1.0f, -1.0f),
    float3(1.0f, 1.0f, -1.0f),

    // +Z
    float3(-1.0f, -1.0f, 1.0f),
    float3(1.0f, -1.0f, 1.0f),
    float3(1.0f, 1.0f, 1.0f),

    float3(-1.0f, -1.0f, 1.0f),
    float3(1.0f, 1.0f, 1.0f),
    float3(-1.0f, 1.0f, 1.0f),

    // -X
    float3(-1.0f, -1.0f, -1.0f),
    float3(-1.0f, -1.0f, 1.0f),
    float3(-1.0f, 1.0f, 1.0f),

    float3(-1.0f, -1.0f, -1.0f),
    float3(-1.0f, 1.0f, 1.0f),
    float3(-1.0f, 1.0f, -1.0f),

    // +X
    float3(1.0f, -1.0f, -1.0f),
    float3(1.0f, 1.0f, 1.0f),
    float3(1.0f, -1.0f, 1.0f),

    float3(1.0f, -1.0f, -1.0f),
    float3(1.0f, 1.0f, -1.0f),
    float3(1.0f, 1.0f, 1.0f),

    // -Y
    float3(-1.0f, -1.0f, -1.0f),
    float3(1.0f, -1.0f, -1.0f),
    float3(1.0f, -1.0f, 1.0f),

    float3(-1.0f, -1.0f, -1.0f),
    float3(1.0f, -1.0f, 1.0f),
    float3(-1.0f, -1.0f, 1.0f),

    // +Y
    float3(-1.0f, 1.0f, -1.0f),
    float3(1.0f, 1.0f, 1.0f),
    float3(1.0f, 1.0f, -1.0f),

    float3(-1.0f, 1.0f, -1.0f),
    float3(-1.0f, 1.0f, 1.0f),
    float3(1.0f, 1.0f, 1.0f)
};

VSOutput VSMain(uint vertex_id : SV_VertexID)
{
    VSOutput output;

    float3 direction = cube_vertices[vertex_id];

    // w = 0 removes camera translation
    float4 clip = mul(constants.view_projection, float4(direction, 0.0f));

    // Place the skybox at the far depth plane
    clip.z = clip.w;

    output.position = clip;
    output.direction = direction;

    return output;
}
