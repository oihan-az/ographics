#ifndef OMEGA_TEXTURE_HANDLE_HPP
#define OMEGA_TEXTURE_HANDLE_HPP

#include <omega/core/handle.hpp>

namespace omega
{

struct TextureTag;

using TextureHandle = Handle<TextureTag>;
using TextureHandleHash = HandleHash<TextureTag>;

} // namespace omega

#endif // OMEGA_TEXTURE_HANDLE_HPP