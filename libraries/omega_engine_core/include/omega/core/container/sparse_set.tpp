#ifndef OMEGA_SPARSE_SET_CONTAINER_TPP
#define OMEGA_SPARSE_SET_CONTAINER_TPP

namespace omega
{

template <typename T, typename Key, typename Indexer>
bool SparseSetContainer<T, Key, Indexer>::contains_key(const Key& key) const
{
    uint32_t id = m_indexer(key);
    if (id >= m_sparse.size())
        return false;
    return m_sparse[id] != TOMBSTONE;
}

template <typename T, typename Key, typename Indexer> 
bool SparseSetContainer<T, Key, Indexer>::contains(const Key& key) const
{
    uint32_t id = m_indexer(key);
    if (id >= m_sparse.size())
        return false;

    uint32_t dense_idx = m_sparse[id];
    if (dense_idx == TOMBSTONE)
        return false;

    return m_dense_to_key[dense_idx] == key;
}

template <typename T, typename Key, typename Indexer>
Key SparseSetContainer<T, Key, Indexer>::key_at(uint32_t index) const
{
    return m_dense_to_key[index];
}

template <typename T, typename Key, typename Indexer> 
T& SparseSetContainer<T, Key, Indexer>::insert(const Key& key, T&& value)
{
    uint32_t id = m_indexer(key);

    if (id >= m_sparse.size())
    {
        m_sparse.resize(id + 1, TOMBSTONE);
    }

    m_sparse[id] = static_cast<uint32_t>(m_dense_data.size());
    m_dense_to_key.push_back(key);
    m_dense_data.push_back(std::move(value));

    return m_dense_data.back();
}


template <typename T, typename Key, typename Indexer> 
T& SparseSetContainer<T, Key, Indexer>::insert(const Key& key, const T& value)
{
    uint32_t id = m_indexer(key);

    if (id >= m_sparse.size())
    {
        m_sparse.resize(id + 1, TOMBSTONE);
    }

    m_sparse[id] = static_cast<uint32_t>(m_dense_data.size());
    m_dense_to_key.push_back(key);
    m_dense_data.push_back(value);

    return m_dense_data.back();
}


template <typename T, typename Key, typename Indexer> 
SparseSetSwapInfo<Key> SparseSetContainer<T, Key, Indexer>::erase(const Key& key)
{
    SparseSetSwapInfo<Key> swap_info;

    if (!contains(key))
        return swap_info;

    uint32_t id = m_indexer(key);
    uint32_t index_to_remove = m_sparse[id];
    uint32_t last_dense_index = static_cast<uint32_t>(m_dense_data.size() - 1);
    Key last_key = m_dense_to_key[last_dense_index];

    if (index_to_remove != last_dense_index)
    {
        m_dense_data[index_to_remove] = std::move(m_dense_data[last_dense_index]);
        m_dense_to_key[index_to_remove] = last_key;
        m_sparse[m_indexer(last_key)] = index_to_remove;

        swap_info.exchanged = true;
        swap_info.moved_key = last_key;
        swap_info.old_index = last_dense_index;
        swap_info.new_index = index_to_remove;
    }

    m_dense_data.pop_back();
    m_dense_to_key.pop_back();
    m_sparse[id] = TOMBSTONE;

    return swap_info;
}

template <typename T, typename Key, typename Indexer>
void SparseSetContainer<T, Key, Indexer>::clear()
{
    m_dense_data.clear();
    m_dense_to_key.clear();
    m_sparse.clear();
}

template <typename T, typename Key, typename Indexer> 
uint32_t SparseSetContainer<T, Key, Indexer>::get_dense_index(const Key& key) const
{
    if (!contains(key))
        return TOMBSTONE;

    return m_sparse[m_indexer(key)];
}

template <typename T, typename Key, typename Indexer> 
T* SparseSetContainer<T, Key, Indexer>::get(const Key& key)
{
    if (!contains(key))
        return nullptr;

    return &m_dense_data[m_sparse[m_indexer(key)]];
}

template <typename T, typename Key, typename Indexer> 
const T* SparseSetContainer<T, Key, Indexer>::get(const Key& key) const
{
    if (!contains(key))
        return nullptr;

    return &m_dense_data[m_sparse[m_indexer(key)]];
}

template <typename T, typename Key, typename Indexer>
std::vector<T>& SparseSetContainer<T, Key, Indexer>::get_dense_data()
{
    return m_dense_data;
}

template <typename T, typename Key, typename Indexer>
const std::vector<T>& SparseSetContainer<T, Key, Indexer>::get_dense_data() const
{
    return m_dense_data;
}

template <typename T, typename Key, typename Indexer> 
uint32_t SparseSetContainer<T, Key, Indexer>::size() const
{
    return static_cast<uint32_t>(m_dense_data.size());
}

} // namespace omega

#endif // OMEGA_SPARSE_SET_CONTAINER_TPP