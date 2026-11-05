#ifndef OMEGA_ENTITY_ITERATOR_HPP
#define OMEGA_ENTITY_ITERATOR_HPP

#include <omega/core/entity.hpp>

#include <cstdint>

namespace omega
{

class Scene;

class EntityIterator
{
  public:
    EntityIterator(const Scene* scene, uint32_t index);

    Entity operator*() const;
    EntityIterator& operator++();

    bool operator==(const EntityIterator& other) const;

  private:
    const Scene* m_scene;
    uint32_t m_index;

    void skip_retired();
};

} // namespace omega

#endif // OMEGA_ENTITY_ITERATOR_HPP