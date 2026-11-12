#ifndef ORENDERGRAPH_RESOURCE_POOL_HPP
#define ORENDERGRAPH_RESOURCE_POOL_HPP

#include <orendergraph/resource_handle.hpp>
#include <orendergraph/resource_traits.hpp>

#include <cstdint>
#include <vector>

namespace orendergraph
{

class IResourcePool
{
  public:
    virtual ~IResourcePool() = default;
    virtual void reset() = 0;
};

template <typename T> 
class ResourcePool : public IResourcePool
{
  public:
    using StateType = typename ResourceTraits<T>::StateType;

    struct ResourceSlot
    {
        T* physical_resource = nullptr;
        bool is_imported = false;
        StateType current_state{};
    };

    ~ResourcePool() override;
    void reset() override;

    ResourceHandle<T> add_slot(T* resource, bool is_imported);
    bool remove_slot(ResourceHandle<T> handle);
    bool replace_resource(ResourceHandle<T> handle, T* new_resource);

    ResourceSlot& slot(uint32_t resource_id);
    const ResourceSlot& slot(uint32_t resource_id) const;
    uint32_t slot_count();

    StateType get_current_state(uint32_t id) const;
    void update_current_state(uint32_t id, StateType new_state);

private:
    std::vector<ResourceSlot> m_slots;
};

} // namespace orendergraph

#include <orendergraph/resource_pool.tpp>

#endif // ORENDERGRAPH_RESOURCE_POOL_HPP