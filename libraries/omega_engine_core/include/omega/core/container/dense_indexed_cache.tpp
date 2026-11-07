#ifndef OMEGA_DENSE_INDEXED_CACHE_TPP
#define OMEGA_DENSE_INDEXED_CACHE_TPP

namespace omega
{

template <typename T> 
void DenseIndexedCache<T>::resize(uint32_t pool_size)
{
    if (pool_size > m_cache_entries.size())
    {
        m_cache_entries.resize(pool_size, CacheEntry{T{}, true});
    }
}

template <typename T> 
void DenseIndexedCache<T>::clear()
{
    m_cache_entries.clear();
}

template <typename T> 
void DenseIndexedCache<T>::set(uint32_t dense_index, T data)
{
    if (dense_index >= m_cache_entries.size())
    {
        m_cache_entries.resize(dense_index + 1, CacheEntry{T{}, true});
    }
    m_cache_entries[dense_index].data = std::move(data);
    m_cache_entries[dense_index].dirty = false;
}

template <typename T> 
void DenseIndexedCache<T>::invalidate(uint32_t dense_index)
{
    if (dense_index < m_cache_entries.size())
    {
        m_cache_entries[dense_index].dirty = true;
    }
}

template <typename T>
void DenseIndexedCache<T>::handle_swap(bool exchanged, uint32_t old_dense_index, uint32_t new_dense_index)
{
    if (exchanged)
    {
        m_cache_entries[new_dense_index] = std::move(m_cache_entries[old_dense_index]);
    }

    if (!m_cache_entries.empty())
    {
        m_cache_entries.pop_back();
    }
}

template <typename T> 
T* DenseIndexedCache<T>::get(uint32_t dense_index)
{
    if (dense_index >= m_cache_entries.size())
        return nullptr;

    return &m_cache_entries[dense_index].data;
}

template <typename T> 
const T* DenseIndexedCache<T>::get(uint32_t dense_index) const
{
    if (dense_index >= m_cache_entries.size())
        return nullptr;

    return &m_cache_entries[dense_index].data;
}

template <typename T> 
bool DenseIndexedCache<T>::is_dirty(uint32_t dense_index) const
{
    if (dense_index >= m_cache_entries.size())
        return true;

    return m_cache_entries[dense_index].dirty;
}

} // namespace omega

#endif // OMEGA_DENSE_INDEXED_CACHE_TPP