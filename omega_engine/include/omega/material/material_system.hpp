#ifndef OMEGA_MATERIAL_SYSTEM_HPP
#define OMEGA_MATERIAL_SYSTEM_HPP

#include <omega/material/material_pool.hpp>

#include <memory>
#include <typeindex>
#include <unordered_map>

namespace omega
{

class MaterialSystem
{
  public:
    template <typename T> MaterialHandle<T> create(T material);
    template <typename T> bool destroy(MaterialHandle<T> handle);

    template <typename T> T* get(MaterialHandle<T> handle);
    template <typename T> const T* get(MaterialHandle<T> handle) const;

    template <typename T> bool has(MaterialHandle<T> handle) const;

  private:
    template <typename T> MaterialPool<T>& get_or_create_pool();
    template <typename T> MaterialPool<T>* find_pool();
    template <typename T> const MaterialPool<T>* find_pool() const;

    std::unordered_map<std::type_index, std::unique_ptr<IMaterialPool>> m_pools;
};

} // namespace omega

#include <omega/material/material_system.tpp>

#endif // OMEGA_MATERIAL_SYSTEM_HPP