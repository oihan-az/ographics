#include <omega/core/memory/range_allocator.hpp>

namespace omega
{

RangeAllocator::RangeAllocator(uint32_t capacity)
{
    reset(capacity);
}

bool RangeAllocator::allocate(uint32_t count, uint32_t& offset)
{
    if (count == 0)
        return false;

    for (auto it = m_free_ranges.begin(); it != m_free_ranges.end(); ++it)
    {
        if (it->count < count)
            continue;

        offset = it->offset;

        it->offset += count;
        it->count -= count;

        m_available -= count;

        if (it->count == 0)
            m_free_ranges.erase(it);

        return true;
    }

    return false;
}

bool RangeAllocator::release(uint32_t offset, uint32_t count)
{
    if (count == 0 || offset >= m_capacity || count > m_capacity - offset)
        return false;

    const uint32_t end = offset + count;

    auto it = m_free_ranges.begin();

    while (it != m_free_ranges.end() && it->offset < offset)
        ++it;

    // Reject overlapping ranges, including double releases
    if (it != m_free_ranges.begin())
    {
        const auto& previous = *(it - 1);

        if (previous.offset + previous.count > offset)
            return false;
    }

    if (it != m_free_ranges.end() && it->offset < end)
        return false;

    auto inserted = m_free_ranges.insert(it, {offset, count});

    // Merge with the preceding range if adjacent
    if (inserted != m_free_ranges.begin())
    {
        auto previous = inserted - 1;

        if (previous->offset + previous->count == inserted->offset)
        {
            previous->count += inserted->count;
            inserted = m_free_ranges.erase(inserted);
            inserted = previous;
        }
    }

    // Merge with the following range if adjacent
    auto next = inserted + 1;

    if (next != m_free_ranges.end() && inserted->offset + inserted->count == next->offset)
    {
        inserted->count += next->count;
        m_free_ranges.erase(next);
    }

    m_available += count;

    return true;
}

uint32_t RangeAllocator::capacity() const
{
    return m_capacity;
}

uint32_t RangeAllocator::available() const
{
    return m_available;
}

void RangeAllocator::reset(uint32_t capacity)
{
    m_capacity = capacity;
    m_available = capacity;

    m_free_ranges.clear();

    if (capacity > 0)
        m_free_ranges.push_back({0, capacity});
}

} // namespace omega