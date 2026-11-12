struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
};

struct VSOutput
{
    float4 position : SV_Position;
    float4 color : COLOR;
    float3 normal : NORMAL;
    float3 world_position : POSITION1;
};

struct VertexConstants
{
    float4x4 model;
    float4x4 view_projection;
    float4 color;
};

[[vk::push_constant]]
ConstantBuffer<VertexConstants> constants;

float3x3 inverse_transpose(float3x3 m)
{
    float3 a = m[0];
    float3 b = m[1];
    float3 c = m[2];

    float3 r0 = cross(b, c);
    float3 r1 = cross(c, a);
    float3 r2 = cross(a, b);

    float determinant = dot(a, r0);

    return float3x3(r0, r1, r2) / determinant;
}

VSOutput VSMain(VSInput input)
{
    VSOutput output;

    float4 world_position = mul(
        constants.model,
        float4(input.position, 1.0));

    output.position = mul(
        constants.view_projection,
        world_position);

    output.world_position = world_position.xyz;
    output.color = constants.color;

    float3x3 normal_matrix = inverse_transpose((float3x3) constants.model);
    output.normal = normalize(mul(normal_matrix, input.normal));

    return output;
}