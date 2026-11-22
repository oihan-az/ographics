#ifndef OMEGA_CORE_PERFORMANCE_METRICS_HPP
#define OMEGA_CORE_PERFORMANCE_METRICS_HPP

#include <cstdint>
#include <string>
#include <vector>

namespace omega
{

using CpuMetricHandle = std::uint32_t;

struct CpuMetric
{
    std::string name;
    double elapsed_ms = 0.0;
};

struct CpuMetrics
{
    double frame_ms = 0.0;
    std::vector<CpuMetric> stages;
};

struct FrameStatistics
{
    double fps = 0.0;
    double average_ms = 0.0;
    double minimum_ms = 0.0;
    double maximum_ms = 0.0;

    std::uint32_t sample_count = 0;
};

struct PerformanceMetrics
{
    CpuMetrics cpu;
    FrameStatistics frame;

    std::uint64_t frame_index = 0;
};

} // namespace omega

#endif // OMEGA_CORE_PERFORMANCE_METRICS_HPP