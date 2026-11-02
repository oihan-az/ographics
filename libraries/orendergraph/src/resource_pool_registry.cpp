#include <orendergraph/resource_pool_registry.hpp>

namespace orendergraph
{

void ResourcePoolRegistry::reset()
{
    for (auto& [type_idx, pool] : m_pools)
    {
        pool->reset();
    }
}

} // namespace orendergraph