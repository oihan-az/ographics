#include <omega/core/performance/performance_system.hpp>

#include <algorithm>
#include <cassert>
#include <numeric>

namespace omega
{

namespace
{

double to_milliseconds(PerformanceSystem::Clock::duration duration)
{
    return std::chrono::duration<double, std::milli>(duration).count();
}

} // namespace

CpuMetricHandle PerformanceSystem::register_cpu_metric(const std::string& name)
{
    for (std::size_t i = 0; i < m_metrics.cpu.stages.size(); ++i)
    {
        if (m_metrics.cpu.stages[i].name == name)
            return static_cast<CpuMetricHandle>(i);
    }

    const auto handle = static_cast<CpuMetricHandle>(m_metrics.cpu.stages.size());

    m_metrics.cpu.stages.push_back({std::move(name), 0.0});

    return handle;
}

void PerformanceSystem::begin_frame()
{
    assert(!m_frame_active);

    for (auto& stage : m_metrics.cpu.stages)
        stage.elapsed_ms = 0.0;

    m_frame_start = Clock::now();
    m_frame_active = true;
}

void PerformanceSystem::end_frame()
{
    assert(m_frame_active);

    const TimePoint frame_end = Clock::now();

    m_metrics.cpu.frame_ms = to_milliseconds(frame_end - m_frame_start);

    m_frame_history[m_history_head] = m_metrics.cpu.frame_ms;

    m_history_head = (m_history_head + 1) % history_capacity;
    m_history_count = std::min(m_history_count + 1, history_capacity);

    ++m_metrics.frame_index;

    update_frame_statistics();

    m_frame_active = false;
}

void PerformanceSystem::record_cpu_time(CpuMetricHandle handle, Clock::duration duration)
{
    if (!m_frame_active || handle >= m_metrics.cpu.stages.size())
        return;

    m_metrics.cpu.stages[handle].elapsed_ms += std::chrono::duration<double, std::milli>(duration).count();
}

const PerformanceMetrics& PerformanceSystem::metrics() const
{
    return m_metrics;
}

std::size_t PerformanceSystem::frame_history_size() const
{
    return m_history_count;
}

double PerformanceSystem::frame_history_at(std::size_t index) const
{
    assert(index < m_history_count);

    if (index >= m_history_count)
        return 0.0;

    const std::size_t oldest = (m_history_head + history_capacity - m_history_count) % history_capacity;

    return m_frame_history[(oldest + index) % history_capacity];
}

bool PerformanceSystem::frame_active() const
{
    return m_frame_active;
}

void PerformanceSystem::update_frame_statistics()
{
    auto& stats = m_metrics.frame;

    if (m_history_count == 0)
        return;

    double sum = 0.0;
    const double first = frame_history_at(0);
    double minimum = first;
    double maximum = first;

    for (std::size_t i = 0; i < m_history_count; ++i)
    {
        const double value = m_frame_history[i];

        sum += value;

        minimum = std::min(minimum, value);
        maximum = std::max(maximum, value);
    }

    stats.sample_count = static_cast<std::uint32_t>(m_history_count);

    stats.average_ms = sum / static_cast<double>(m_history_count);

    stats.minimum_ms = minimum;
    stats.maximum_ms = maximum;

    stats.fps = stats.average_ms > 0.0 ? 1000.0 / stats.average_ms : 0.0;
}

ScopedCpuTimer::ScopedCpuTimer(PerformanceSystem& performance, CpuMetricHandle metric)
    : m_performance(performance), m_metric(metric), m_start(PerformanceSystem::Clock::now())
{
}

ScopedCpuTimer::~ScopedCpuTimer()
{
    m_performance.record_cpu_time(m_metric, PerformanceSystem::Clock::now() - m_start);
}

} // namespace omega