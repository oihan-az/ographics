#ifndef OMEGA_ENTITY_HPP
#define OMEGA_ENTITY_HPP

#include <omega/core/handle.hpp>

namespace omega
{

struct EntityTag;

using Entity = Handle<EntityTag>;
using EntityHash = HandleHash<EntityTag>;

} // namespace omega

#endif // OMEGA_ENTITY_HPP