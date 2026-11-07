#ifndef OMEGA_SPARSE_SET_CONTAINER_HPP
#define OMEGA_SPARSE_SET_CONTAINER_HPP

#include <cstdint>
#include <vector>

namespace omega
{

template <typename Key> 
struct KeyIndexer
{
    uint32_t operator()(const Key& key) const
    {
        return static_cast<uint32_t>(key);
    }
};

template <typename Key> struct SparseSetSwapInfo
{
    bool exchanged = false;
    Key moved_key{};
    uint32_t old_index = 0;
    uint32_t new_index = 0;
};

template <typename T, typename Key, typename Indexer = KeyIndexer<Key>> 
class SparseSetContainer
{
  public:
    static constexpr uint32_t TOMBSTONE = static_cast<uint32_t>(-1);

    bool contains(const Key& key) const;
    Key key_at(uint32_t index) const;

    T& insert(const Key& key, T&& value);
    T& insert(const Key& key, const T& value);
    SparseSetSwapInfo<Key> erase(const Key& key);
    void clear();

    T* get(const Key& key);
    const T* get(const Key& key) const;
    std::vector<T>& get_dense_data();
    const std::vector<T>& get_dense_data() const;
    uint32_t size() const;

  private:
    Indexer m_indexer;
    std::vector<uint32_t> m_sparse;
    std::vector<Key> m_dense_to_key;
    std::vector<T> m_dense_data;
};

} // namespace omega

#include "sparse_set.tpp"

#endif // OMEGA_SPARSE_SET_CONTAINER_HPP