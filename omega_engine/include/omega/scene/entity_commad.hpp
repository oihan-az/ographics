#ifndef OMEGA_ENTITY_COMMAND_HPP
#define OMEGA_ENTITY_COMMAND_HPP

#include <omega/core/entity.hpp>

#include <cstdint>

namespace omega
{

enum class EntityCommandType
{
    Create,
    Destroy
};

struct EntityCommand
{
    EntityCommandType type;
    Entity entity;
};

} // namespace omega

#endif // OMEGA_ENTITY_COMMAND_HPP