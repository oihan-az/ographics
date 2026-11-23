#ifndef OMEGA_TRANSFORM_EVENT_HPP
#define OMEGA_TRANSFORM_EVENT_HPP

#include <omega/core/entity/entity.hpp>

namespace omega
{

struct WorldTransformChangedEvent
{
    Entity entity;
};

} // namespace omega

#endif // OMEGA_TRANSFORM_EVENT_HPP