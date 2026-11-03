struct FragmentInput
{
    float3 normal : NORMAL;
    float3 color : COLOR0;
};

float4 PSMain(FragmentInput input) : SV_Target
{
    float3 normal = normalize(input.normal);

    float3 light_direction = normalize(float3(0.6, -0.8, 1.0));

    float ambient = 0.15;
    float diffuse = max(dot(normal, light_direction), 0.0);

    float lighting = ambient + 0.85 * diffuse;

    float3 lit_color = input.color * lighting;

    return float4(lit_color, 1.0);
}