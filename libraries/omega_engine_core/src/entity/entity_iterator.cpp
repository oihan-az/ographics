#include <omega/core/entity/entity_iterator.hpp>
#include <omega/core/scene/scene.hpp>

namespace omega
{

EntityIterator::EntityIterator(const Scene* scene, uint32_t index) : m_scene(scene), m_index(index)
{
    skip_retired();
}

Entity EntityIterator::operator*() const
{
    return {.index = m_index, .generation = m_scene->m_entity_slots[m_index].generation};
}

EntityIterator& EntityIterator::operator++()
{
    ++m_index;
    skip_retired();
    return *this;
}

bool EntityIterator::operator==(const EntityIterator& other) const = default;

void EntityIterator::skip_retired()
{
    while (m_index < m_scene->m_entity_slots.size() && !m_scene->m_entity_slots[m_index].is_alive())
    {
        ++m_index;
    }
}

} // namespace omega