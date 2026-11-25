#ifndef OMEGA_LUMINANCE_HISTOGRAM_COMP_HLSL
#define OMEGA_LUMINANCE_HISTOGRAM_COMP_HLSL

static const uint HISTOGRAM_BIN_COUNT = 256;
static const uint WORKGROUP_SIZE = 16;

struct HistogramConstants
{
    float min_log_luminance;
    float max_log_luminance;
    uint sample_stride;
    uint clear;
};

[[vk::push_constant]]
ConstantBuffer<HistogramConstants> constants;

[[vk::binding(0, 0)]]
Texture2D<float4> hdr_image;

[[vk::binding(1, 0)]]
RWStructuredBuffer<uint> histogram_buffer;

groupshared uint shared_histogram[HISTOGRAM_BIN_COUNT];

[numthreads(16, 16, 1)]
void CSMain(
    uint3 dispatch_id : SV_DispatchThreadID,
    uint3 group_thread_id : SV_GroupThreadID)
{
    uint thread_index = group_thread_id.y * WORKGROUP_SIZE + group_thread_id.x;

    // Clear dispatch: exactly one workgroup
    if (constants.clear != 0)
    {
        histogram_buffer[thread_index] = 0;
        return;
    }

    // Initialize workgroup-local histogram
    shared_histogram[thread_index] = 0;

    GroupMemoryBarrierWithGroupSync();

    uint width, height;
    hdr_image.GetDimensions(width, height);

    uint2 pixel = dispatch_id.xy * constants.sample_stride;
    if (pixel.x < width && pixel.y < height)
    {
        float3 color = max(hdr_image.Load(int3(pixel, 0)).rgb, 0.0f);

        float luminance = dot(color, float3(0.2126f, 0.7152f, 0.0722f));
        float log_luminance = log2(max(luminance, 1e-6f));
        float normalized = saturate((log_luminance - constants.min_log_luminance) / (constants.max_log_luminance - constants.min_log_luminance));

        uint bin = min(uint(normalized * HISTOGRAM_BIN_COUNT), HISTOGRAM_BIN_COUNT - 1);

        InterlockedAdd(shared_histogram[bin], 1);
    }

    GroupMemoryBarrierWithGroupSync();

    // Merge the local histogram into the global buffer
    uint count = shared_histogram[thread_index];
    if (count > 0)
    {
        InterlockedAdd(histogram_buffer[thread_index], count);
    }
}

#endif // OMEGA_LUMINANCE_HISTOGRAM_COMP_HLSL
