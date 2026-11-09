#ifndef OMEGA_RANGE_ALLOCATOR_HPP
#define OMEGA_RANGE_ALLOCATOR_HPP

#include <cstdint>
#include <vector>

namespace omega
{

class RangeAllocator
{
  public:
    explicit RangeAllocator(uint32_t capacity = 0);

    bool allocate(uint32_t count, uint32_t& offset);
    bool release(uint32_t offset, uint32_t count);

    uint32_t capacity() const;
    uint32_t available() const;

    void reset(uint32_t capacity);

  private:
    struct Range
    {
        uint32_t offset = 0;
        uint32_t count = 0;
    };

  private:
    uint32_t m_capacity = 0;
    uint32_t m_available = 0;

    std::vector<Range> m_free_ranges;
};

} // namespace omega

#endif // OMEGA_RANGE_ALLOCATOR_HPP