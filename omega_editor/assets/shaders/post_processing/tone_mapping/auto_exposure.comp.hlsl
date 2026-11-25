#ifndef OMEGA_AUTO_EXPOSURE_COMP_HLSL
#define OMEGA_AUTO_EXPOSURE_COMP_HLSL

static const uint HISTOGRAM_BIN_COUNT = 256;

struct ExposureConstants
{
    float min_log_luminance;
    float max_log_luminance;
    float low_percentile;
    float high_percentile;
    
    float highlight_weight;

    float middle_gray;
    float min_exposure;
    float max_exposure;
    float delta_time;

    float speed_up;
    float speed_down;
    uint has_history;
};

[[vk::push_constant]]
ConstantBuffer<ExposureConstants> constants;

[[vk::binding(0, 0)]]
StructuredBuffer<uint> histogram_buffer;

[[vk::binding(1, 0)]]
RWStructuredBuffer<float> exposure_buffer;

[numthreads(1, 1, 1)]
void CSMain(uint3 dispatch_id : SV_DispatchThreadID)
{
    uint total_samples = 0;
    for (uint i = 0; i < HISTOGRAM_BIN_COUNT; ++i)
    {
        total_samples += histogram_buffer[i];
    }

    if (total_samples == 0)
    {
        exposure_buffer[0] = 1.0f;
        return;
    }

    float low_sample = float(total_samples) * constants.low_percentile;
    float high_sample = float(total_samples) * constants.high_percentile;

    float cumulative = 0.0f;
    float weighted_log_sum = 0.0f;
    float weighted_count = 0.0f;

    for (uint i = 0; i < HISTOGRAM_BIN_COUNT; ++i)
    {
        float count = float(histogram_buffer[i]);

        float bin_start = cumulative;
        float bin_end = cumulative + count;

        // Number of samples inside the selected percentile range
        float included = max(min(bin_end, high_sample) - max(bin_start, low_sample), 0.0f);
        if (included > 0.0f)
        {
            float normalized_bin = (float(i) + 0.5f) / float(HISTOGRAM_BIN_COUNT);
            float log_luminance = lerp(constants.min_log_luminance, constants.max_log_luminance, normalized_bin);
            
            float weight = exp2(constants.highlight_weight * (log_luminance - constants.min_log_luminance));
            
            weighted_log_sum += log_luminance * included * weight;
            weighted_count += included * weight;
        }

        cumulative = bin_end;
    }

    float average_log_luminance = weighted_log_sum / max(weighted_count, 1.0f);
    float metered_luminance = exp2(average_log_luminance);
    
    float target_exposure = clamp(constants.middle_gray / max(metered_luminance, 1e-6f), constants.min_exposure, constants.max_exposure);

    // Read the previous frame only after history is initialized
    float previous_exposure = target_exposure;
    if (constants.has_history != 0)
    {
        previous_exposure = exposure_buffer[0];
    }

    // Decreasing exposure: adapting to a brighter scene.
    // Increasing exposure: adapting to a darker scene.
    float adaptation_speed = target_exposure > previous_exposure ? constants.speed_down: constants.speed_up;
    float adaptation = 1.0f - exp(-adaptation_speed * constants.delta_time);

    float exposure = lerp(previous_exposure, target_exposure, adaptation);
    exposure_buffer[0] = exposure;
}

#endif // OMEGA_AUTO_EXPOSURE_COMP_HLSL
