#ifndef OMEGA_CORE_PERFORMANCE_METRICS_HPP
#define OMEGA_CORE_PERFORMANCE_METRICS_HPP

#include <cstdint>
#include <limits>
#include <string>
#include <vector>

namespace omega
{

using CpuScopeId = std::uint32_t;

inline constexpr CpuScopeId invalid_cpu_scope = std::numeric_limits<CpuScopeId>::max();

struct CpuMetric
{
    std::string_view name;
    CpuScopeId parent = invalid_cpu_scope;

    double elapsed_ms = 0.0;
    double self_ms = 0.0;

    std::uint32_t calls = 0;
};

struct CpuMetrics
{
    double frame_ms = 0.0;
    std::vector<CpuMetric> scopes;
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

#endif