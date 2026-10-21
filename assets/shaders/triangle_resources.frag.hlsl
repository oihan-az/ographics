struct PSInput
{
    float4 position : SV_Position;
    float3 color : COLOR0;
    float t : TIME;
};

float3 rotate_hue(float3 color, float hue)
{
    const float3 k = float3(0.5773503f, 0.5773503f, 0.5773503f);

    float cos_hue = cos(hue);
    float sin_hue = sin(hue);

    return color * cos_hue
        + cross(k, color) * sin_hue
        + k * dot(k, color) * (1.0f - cos_hue);
}

float4 PSMain(PSInput input) : SV_Target
{
    float hue = input.t;

    float3 color = rotate_hue(input.color, hue);

    return float4(saturate(color), 1.0f);
}