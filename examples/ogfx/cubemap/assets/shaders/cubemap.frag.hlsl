struct PSInput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

[[vk::binding(0, 0)]]
TextureCube<float4> texture;

[[vk::binding(1, 0)]]
SamplerState texture_sampler;

static const float3 face_directions[7][3] =
{
    // 0: Dummy entry for empty cells (+Z)
    {
        float3(1.0, 0.0, 0.0),
        float3(0.0, -1.0, 0.0),
        float3(0.0, 0.0, 1.0)
    },
    // Top (+Y)
    {
        float3(1.0, 0.0, 0.0),
        float3(0.0, 0.0, 1.0),
        float3(0.0, 1.0, 0.0)
    },
    // Left (-X)
    {
        float3(0.0, 0.0, 1.0),
        float3(0.0, -1.0, 0.0),
        float3(-1.0, 0.0, 0.0)
    },
    // Front (+Z)
    {
        float3(1.0, 0.0, 0.0),
        float3(0.0, -1.0, 0.0),
        float3(0.0, 0.0, 1.0)
    },
    // Right (+X)
    {
        float3(0.0, 0.0, -1.0),
        float3(0.0, -1.0, 0.0),
        float3(1.0, 0.0, 0.0)
    },
    // Back (-Z)
    {
        float3(-1.0, 0.0, 0.0),
        float3(0.0, -1.0, 0.0),
        float3(0.0, 0.0, -1.0)
    },
    // Bottom (-Y)
    {
        float3(1.0, 0.0, 0.0),
        float3(0.0, 0.0, -1.0),
        float3(0.0, -1.0, 0.0)
    }
};

float3 face_direction(uint face, float2 uv)
{
    float2 p = uv * 2.0 - 1.0;

    return face_directions[face][0] * p.x
         + face_directions[face][1] * p.y
         + face_directions[face][2];
}

static const int face_indices[3][4] =
{
    { 0, 1, 0, 0 }, // Empty, Top, Empty, Empty
    { 2, 3, 4, 5 }, // Left, Front, Right, Back
    { 0, 6, 0, 0 }  // Empty, Bottom, Empty, Empty
};

float4 PSMain(PSInput input) : SV_TARGET
{
    float2 screen_uv = float2(input.uv.x, 1.0 - input.uv.y);
    float2 grid_uv = screen_uv * float2(4.0, 3.0);

    uint column = (uint) min(floor(grid_uv.x), 3.99);
    uint row = (uint) min(floor(grid_uv.y), 2.99);

    uint face = face_indices[row][column];
    
    uint texture_width;
    uint texture_height;
    uint mip_levels;
    texture.GetDimensions(0, texture_width, texture_height, mip_levels);
    
    float2 local_uv = frac(grid_uv);
    local_uv = (floor(local_uv * float(texture_width)) + 0.5) / float(texture_width);

    float3 direction = face_direction(face, local_uv);
    
    float4 sampled_color = texture.Sample(texture_sampler, normalize(direction));

    float is_valid = step(1.0, (float) face);
    const float4 empty_color = float4(0.08, 0.08, 0.08, 1.0);

    return lerp(empty_color, sampled_color, is_valid);
}