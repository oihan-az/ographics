#ifndef OMEGA_DENSE_INDEXED_CACHE_HPP
#define OMEGA_DENSE_INDEXED_CACHE_HPP

#include <cstdint>
#include <vector>

namespace omega
{

template <typename T> 
class DenseIndexedCache
{
  public:
    struct CacheEntry
    {
        T data{};
        bool dirty = true;
    };

    void resize(uint32_t size);
    void clear();

    void set(uint32_t dense_index, T data);
    void invalidate(uint32_t dense_index);
    void handle_swap(bool exchanged, uint32_t old_dense_index, uint32_t new_dense_index);

    T* get(uint32_t dense_index);
    const T* get(uint32_t dense_index) const;
    bool is_dirty(uint32_t dense_index) const;

  private:
    std::vector<CacheEntry> m_cache_entries;
};

} // namespace omega

#include "dense_indexed_cache.tpp"

#endif // OMEGA_DENSE_INDEXED_CACHE_HPP