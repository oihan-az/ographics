#ifndef OMEGA_CORE_PERFORMANCE_SYSTEM_HPP
#define OMEGA_CORE_PERFORMANCE_SYSTEM_HPP

#include <omega/core/performance/performance_metrics.hpp>

#include <array>
#include <chrono>
#include <cstddef>
#include <string_view>
#include <vector>

namespace omega
{

class ScopedCpuTimer;

class PerformanceSystem
{
  public:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;

    static constexpr std::size_t history_capacity = 240;

    void begin_frame();
    void end_frame();
    void cancel_frame();

    const PerformanceMetrics& metrics() const;

    std::size_t frame_history_size() const;
    double frame_history_at(std::size_t index) const;

    bool frame_active() const;

  private:
    friend class ScopedCpuTimer;

    CpuScopeId begin_cpu_scope(std::string_view name);
    void end_cpu_scope(CpuScopeId id, Clock::duration elapsed);

    void calculate_cpu_self_times();
    void update_frame_statistics();

    PerformanceMetrics m_write_metrics{};
    PerformanceMetrics m_read_metrics{};

    std::vector<CpuScopeId> m_scope_stack;

    std::array<double, history_capacity> m_frame_history{};

    std::size_t m_history_head = 0;
    std::size_t m_history_count = 0;

    TimePoint m_frame_start{};
    bool m_frame_active = false;
};

class ScopedCpuTimer
{
  public:
    ScopedCpuTimer(PerformanceSystem& performance, std::string_view name);

    ~ScopedCpuTimer();

    ScopedCpuTimer(const ScopedCpuTimer&) = delete;
    ScopedCpuTimer& operator=(const ScopedCpuTimer&) = delete;
    ScopedCpuTimer(ScopedCpuTimer&&) = delete;
    ScopedCpuTimer& operator=(ScopedCpuTimer&&) = delete;

  private:
    PerformanceSystem& m_performance;
    CpuScopeId m_scope = invalid_cpu_scope;
    PerformanceSystem::TimePoint m_start{};
};

} // namespace omega

#endif