#ifndef OMEGA_MATERIAL_POOL_HPP
#define OMEGA_MATERIAL_POOL_HPP

#include <omega/material/material_handle.hpp>
#include <omega/material/material_slot.hpp>

#include <cstdint>
#include <vector>

namespace omega
{

class IMaterialPool
{
  public:
    virtual ~IMaterialPool() = default;
};

template <typename T> 
class MaterialPool final : public IMaterialPool
{
  public:
    MaterialHandle<T> create(T material);

    T* get(MaterialHandle<T> handle);
    const T* get(MaterialHandle<T> handle) const;

    bool has(MaterialHandle<T> handle) const;
    bool destroy(MaterialHandle<T> handle);

  private:
    std::vector<MaterialSlot<T>> m_slots;
    std::vector<uint32_t> m_free_indices;
};

} // namespace omega

#include <omega/material/material_pool.tpp>

#endif // OMEGA_MATERIAL_POOL_HPP