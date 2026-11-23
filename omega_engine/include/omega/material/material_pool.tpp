#ifndef OMEGA_MATERIAL_POOL_TPP
#define OMEGA_MATERIAL_POOL_TPP

#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

namespace omega
{

template <typename T> 
MaterialHandle<T> MaterialPool<T>::create(T material)
{
    if (!m_free_indices.empty())
    {
        const uint32_t index = m_free_indices.back();
        auto& slot = m_slots[index];

        slot.value = std::move(material);
        slot.occupied = true;

        m_free_indices.pop_back();

        return {index, slot.generation};
    }

    if (m_slots.size() >= MaterialHandle<T>::invalid_index)
        throw std::overflow_error("Material pool capacity exceeded.");

    const uint32_t index = static_cast<uint32_t>(m_slots.size());

    m_slots.push_back({.value = std::move(material), .generation = 0, .occupied = true});

    return {index, 0};
}

template<typename T>
T* MaterialPool<T>::get(MaterialHandle<T> handle)
{
    if (!handle.is_valid() || handle.index >= m_slots.size())
        return nullptr;

    auto& slot = m_slots[handle.index];
    if (!slot.occupied || slot.generation != handle.generation)
        return nullptr;

    return &slot.value;
}

template<typename T>
const T* MaterialPool<T>::get(MaterialHandle<T> handle) const
{
    if (!handle.is_valid() || handle.index >= m_slots.size())
        return nullptr;

    const auto& slot = m_slots[handle.index];
    if (!slot.occupied || slot.generation != handle.generation)
        return nullptr;

    return &slot.value;
}

template <typename T> 
bool MaterialPool<T>::has(MaterialHandle<T> handle) const
{
    return get(handle) != nullptr;
}

template<typename T>
bool MaterialPool<T>::destroy(MaterialHandle<T> handle)
{
    if (!has(handle))
        return false;

    auto& slot = m_slots[handle.index];

    slot.occupied = false;

    if (slot.generation == std::numeric_limits<uint32_t>::max())
        return true;

    ++slot.generation;
    m_free_indices.push_back(handle.index);

    return true;
}

} // namespace omega

#endif // OMEGA_MATERIAL_POOL_TPP