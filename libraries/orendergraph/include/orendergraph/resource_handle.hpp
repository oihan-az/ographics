#ifndef ORENDERGRAPH_RESOURCE_HANDLE_HPP
#define ORENDERGRAPH_RESOURCE_HANDLE_HPP

#include <typeindex>

namespace orendergraph
{

struct IResourceHandle
{
    uint32_t id = 0xFFFFFFFF;

    bool is_valid() const { return id != 0xFFFFFFFF; }

    bool operator==(const IResourceHandle& other) const { return id == other.id; }
    bool operator!=(const IResourceHandle& other) const { return id != other.id; }

    virtual std::type_index resource_type_idx() const = 0;
};

template <typename T> 
struct ResourceHandle : public IResourceHandle
{
    std::type_index resource_type_idx() const override
    {
        return std::type_index(typeid(T));
    }
};

} // namespace orendergraph

#endif // ORENDERGRAPH_RESOURCE_HANDLE_HPP