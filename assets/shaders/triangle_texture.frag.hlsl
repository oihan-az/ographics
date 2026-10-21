struct PSInput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

[[vk::binding(0, 0)]]
Texture2D<float4> texture;

[[vk::binding(1, 0)]]
SamplerState texture_sampler;

float4 PSMain(PSInput input) : SV_TARGET
{
    uint texture_width;
    uint texture_height;
    uint mip_levels;

    texture.GetDimensions(0, texture_width, texture_height, mip_levels);

    float u = frac(input.uv.x);

    float mip_level = min(u * mip_levels, (float)mip_levels - 1);

    return texture.SampleLevel(texture_sampler, input.uv, mip_level);
}