#ifndef OMEGA_ENTITY_EVENT_HPP
#define OMEGA_ENTITY_EVENT_HPP

#include <omega/core/entity/entity.hpp>

namespace omega
{

struct EntityAddedEvent
{
    Entity entity;
};

struct EntityRemovedEvent
{
    Entity entity;
};

} // namespace omega

#endif // OMEGA_ENTITY_EVENT_HPP