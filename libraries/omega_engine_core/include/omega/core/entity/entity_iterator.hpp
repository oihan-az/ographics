#ifndef OMEGA_ENTITY_ITERATOR_HPP
#define OMEGA_ENTITY_ITERATOR_HPP

#include <omega/core/entity/entity.hpp>
#include <omega/core/entity/entity_slot.hpp>

#include <cstdint>

namespace omega
{

class EntityIterator
{
  public:
    EntityIterator(const std::vector<EntitySlot>& entities, uint32_t index);

    Entity operator*() const;
    EntityIterator& operator++();

    bool operator==(const EntityIterator& other) const;

  private:
    const std::vector<EntitySlot>& m_entities;
    uint32_t m_index;

    void skip_retired();
};

} // namespace omega

#endif // OMEGA_ENTITY_ITERATOR_HPP