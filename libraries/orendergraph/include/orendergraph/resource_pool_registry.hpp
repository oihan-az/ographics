#ifndef ORENDERGRAPH_RESOURCE_POOL_REGISTRY_HPP
#define ORENDERGRAPH_RESOURCE_POOL_REGISTRY_HPP

#include <orendergraph/resource_pool.hpp>
#include <orendergraph/resource_traits.hpp>

#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>

namespace ogfx
{

class PhysicalDevice;
class Device;

}

namespace orendergraph
{

class ResourcePoolRegistry
{
  public:

    template <typename T> 
    ResourcePool<T>* get_pool();

    template <typename T> 
    T* get_resource(ResourceHandle<T> handle);

    template <typename T>
    ResourceHandle<T> create_resource(const ogfx::PhysicalDevice& physical_device,
                                      const ogfx::Device& device, 
                                      typename ResourceTraits<T>::DescType& desc);

    template <typename T> 
    ResourceHandle<T> import_resource(T* resource);

    void reset();

  private:
    std::unordered_map<std::type_index, std::unique_ptr<IResourcePool>> m_pools;
};

} // namespace orendergraph

#include <orendergraph/resource_pool_registry.tpp>

#endif // ORENDERGRAPH_RESOURCE_POOL_REGISTRY_HPP