#ifndef OMEGA_MODEL_HANDLE_HPP
#define OMEGA_MODEL_HANDLE_HPP

#include <omega/core/handle.hpp>

namespace omega
{

struct ModelTag;

using ModelHandle = Handle<ModelTag>;
using ModelHandleHash = HandleHash<ModelTag>;

} // namespace omega

#endif