#ifndef OMEGA_TEXTURE_EVENT_HPP
#define OMEGA_TEXTURE_EVENT_HPP

#include <omega/texture/texture_handle.hpp>

namespace omega
{

struct TextureCreatedEvent
{
    TextureHandle handle;
};

struct TextureReleasedEvent
{
    TextureHandle handle;
};

} // namespace omega

#endif // OMEGA_TEXTURE_EVENT_HPP