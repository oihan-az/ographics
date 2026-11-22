#include <omega/core/performance/performance_system.hpp>

#include <algorithm>
#include <cassert>
#include <chrono>

namespace omega
{

namespace
{

double to_milliseconds(PerformanceSystem::Clock::duration duration)
{
    return std::chrono::duration<double, std::milli>(duration).count();
}

} // namespace

void PerformanceSystem::begin_frame()
{
    assert(!m_frame_active);
    assert(m_scope_stack.empty());

    m_write_metrics.cpu.scopes.clear();
    m_scope_stack.clear();

    m_write_metrics.cpu.frame_ms = 0.0;

    m_frame_start = Clock::now();
    m_frame_active = true;
}

void PerformanceSystem::end_frame()
{
    assert(m_frame_active);
    assert(m_scope_stack.empty());

    if (!m_frame_active || !m_scope_stack.empty())
        return;

    const TimePoint frame_end = Clock::now();

    m_write_metrics.cpu.frame_ms = to_milliseconds(frame_end - m_frame_start);

    calculate_cpu_self_times();

    m_frame_history[m_history_head] = m_write_metrics.cpu.frame_ms;
    m_history_head = (m_history_head + 1) % history_capacity;
    m_history_count = std::min(m_history_count + 1, history_capacity);

    m_write_metrics.frame_index = m_read_metrics.frame_index + 1;

    update_frame_statistics();

    std::swap(m_read_metrics, m_write_metrics);

    m_frame_active = false;
}

void PerformanceSystem::cancel_frame()
{
    assert(m_scope_stack.empty());

    if (!m_frame_active || !m_scope_stack.empty())
        return;

    m_write_metrics.cpu.scopes.clear();
    m_write_metrics.cpu.frame_ms = 0.0;

    m_frame_active = false;
}

CpuScopeId PerformanceSystem::begin_cpu_scope(std::string_view name)
{
    if (!m_frame_active)
        return invalid_cpu_scope;

    const CpuScopeId parent = m_scope_stack.empty() ? invalid_cpu_scope : m_scope_stack.back();

    auto& scopes = m_write_metrics.cpu.scopes;
    assert(scopes.size() < invalid_cpu_scope);
    if (scopes.size() >= invalid_cpu_scope)
        return invalid_cpu_scope;

    const CpuScopeId id = static_cast<CpuScopeId>(scopes.size());
    scopes.push_back({.name = name, .parent = parent, .elapsed_ms = 0.0, .self_ms = 0.0, .calls = 1});
    m_scope_stack.push_back(id);

    return id;
}

void PerformanceSystem::end_cpu_scope(CpuScopeId id, Clock::duration elapsed)
{
    if (id == invalid_cpu_scope)
        return;

    assert(m_frame_active);
    assert(!m_scope_stack.empty());
    assert(m_scope_stack.back() == id);

    if (!m_frame_active || m_scope_stack.empty() || m_scope_stack.back() != id)
    {
        return;
    }

    m_write_metrics.cpu.scopes[id].elapsed_ms = to_milliseconds(elapsed);

    m_scope_stack.pop_back();
}

void PerformanceSystem::calculate_cpu_self_times()
{
    auto& scopes = m_write_metrics.cpu.scopes;

    // Initially, self time equals the full scope duration
    for (auto& scope : scopes)
        scope.self_ms = scope.elapsed_ms;

    // Remove the inclusive time of immediate children
    for (const auto& scope : scopes)
    {
        if (scope.parent == invalid_cpu_scope)
            continue;

        scopes[scope.parent].self_ms -= scope.elapsed_ms;
    }

    // Protect against small floating-point precision errors
    for (auto& scope : scopes)
        scope.self_ms = std::max(0.0, scope.self_ms);
}

void PerformanceSystem::update_frame_statistics()
{
    auto& stats = m_write_metrics.frame;

    if (m_history_count == 0)
    {
        stats = {};
        return;
    }

    const double first = frame_history_at(0);

    double minimum = first;
    double maximum = first;
    double sum = 0.0;

    for (std::size_t i = 0; i < m_history_count; ++i)
    {
        const double value = frame_history_at(i);

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

const PerformanceMetrics& PerformanceSystem::metrics() const
{
    return m_read_metrics;
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

ScopedCpuTimer::ScopedCpuTimer(PerformanceSystem& performance, std::string_view name) : m_performance(performance)
{
    m_scope = m_performance.begin_cpu_scope(name);

    if (m_scope != invalid_cpu_scope)
        m_start = PerformanceSystem::Clock::now();
}

ScopedCpuTimer::~ScopedCpuTimer()
{
    if (m_scope == invalid_cpu_scope)
        return;

    m_performance.end_cpu_scope(m_scope, PerformanceSystem::Clock::now() - m_start);
}

} // namespace omega