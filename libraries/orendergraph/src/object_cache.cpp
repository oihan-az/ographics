#include <orendergraph/object_cache.hpp>

namespace orendergraph
{

ObjectCache::ObjectCache(const ogfx::PhysicalDevice& physical_device, const ogfx::Device& device)
    : m_physical_device(physical_device), m_device(device)
{
}

void ObjectCache::clear()
{
    m_entries.clear();
}

} // namespace orendergraph