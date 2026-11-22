#ifndef OMEGA_CORE_PERFORMANCE_SYSTEM_HPP
#define OMEGA_CORE_PERFORMANCE_SYSTEM_HPP

#include <omega/core/performance/performance_metrics.hpp>

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>

namespace omega
{

class PerformanceSystem
{
  public:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;

    static constexpr std::size_t history_capacity = 240;

    CpuMetricHandle register_cpu_metric(const std::string& name);

    void begin_frame();
    void end_frame();

    void record_cpu_time(CpuMetricHandle handle, Clock::duration duration);

    const PerformanceMetrics& metrics() const;

    std::size_t frame_history_size() const;
    double frame_history_at(std::size_t index) const;

    bool frame_active() const;

  private:
    void update_frame_statistics();

    PerformanceMetrics m_metrics{};

    std::array<double, history_capacity> m_frame_history{};

    std::size_t m_history_head = 0;
    std::size_t m_history_count = 0;

    TimePoint m_frame_start{};

    bool m_frame_active = false;
};

class ScopedCpuTimer
{
  public:
    ScopedCpuTimer(PerformanceSystem& performance, CpuMetricHandle metric);
    ~ScopedCpuTimer();

    ScopedCpuTimer(const ScopedCpuTimer&) = delete;
    ScopedCpuTimer& operator=(const ScopedCpuTimer&) = delete;

  private:
    PerformanceSystem& m_performance;
    CpuMetricHandle m_metric;
    PerformanceSystem::Clock::time_point m_start;
};

} // namespace omega

#endif // OMEGA_CORE_PERFORMANCE_SYSTEM_HPP