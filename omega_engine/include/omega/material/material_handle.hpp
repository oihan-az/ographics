#ifndef OMEGA_MATERIAL_HANDLE_HPP
#define OMEGA_MATERIAL_HANDLE_HPP

#include <omega/core/handle.hpp>

namespace omega
{

template<typename T>
struct MaterialTag;

template<typename T>
using MaterialHandle = Handle<MaterialTag<T>>;

template<typename T>
using MaterialHandleHash = HandleHash<MaterialTag<T>>;

} // namespace omega

#endif // OMEGA_MATERIAL_HANDLE_HPP