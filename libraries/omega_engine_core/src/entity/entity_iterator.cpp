#include <omega/core/entity/entity_iterator.hpp>

namespace omega
{

EntityIterator::EntityIterator(const std::vector<EntitySlot>& entities, uint32_t index) : m_entities(entities), m_index(index)
{
    skip_retired();
}

Entity EntityIterator::operator*() const
{
    return {.index = m_index, .generation = m_entities[m_index].generation};
}

EntityIterator& EntityIterator::operator++()
{
    ++m_index;
    skip_retired();
    return *this;
}

bool EntityIterator::operator==(const EntityIterator& other) const
{
    return &m_entities == &other.m_entities && m_index == other.m_index;
}

void EntityIterator::skip_retired()
{
    while (m_index < m_entities.size() && !m_entities[m_index].is_alive())
    {
        ++m_index;
    }
}

} // namespace omega