#ifndef ORENDERGRAPH_RESOURCE_ACCESS_HPP
#define ORENDERGRAPH_RESOURCE_ACCESS_HPP

#include <orendergraph/resource_handle.hpp>
#include <orendergraph/resource_pool_registry.hpp>

#include <cstdint>
#include <typeindex>
#include <typeinfo>

namespace orendergraph
{

struct IResourceAccess
{
    virtual ~IResourceAccess() = default;

    virtual uint32_t resource_id() const = 0;
    virtual std::type_index resource_type_idx() const = 0;

    virtual void transition(ogfx::CommandBuffer& cmd, ResourcePoolRegistry& registry) = 0;
};

template <typename T> 
struct ResourceAccess : public IResourceAccess
{
    using Traits = ResourceTraits<T>;
    using StateType = typename Traits::StateType;
    using DataType = typename Traits::DataType;

    ResourceHandle<T> m_handle;
    StateType m_requested_state;
    DataType m_data;

    ResourceAccess(ResourceHandle<T> handle, StateType requested_state, DataType data);

    uint32_t resource_id() const override;
    std::type_index resource_type_idx() const override;

    void transition(ogfx::CommandBuffer& cmd, ResourcePoolRegistry& registry) override;
};

} // namespace orendergraph

#include <orendergraph/resource_access.tpp>

#endif // ORENDERGRAPH_RESOURCE_ACCESS_HPP