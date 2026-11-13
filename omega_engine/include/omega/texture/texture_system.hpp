
#ifndef OMEGA_TEXTURE_SYSTEM_HPP
#define OMEGA_TEXTURE_SYSTEM_HPP

#include <omega/texture/texture_asset.hpp>
#include <omega/texture/texture_data.hpp>
#include <omega/texture/texture_handle.hpp>

#include <cstdint>
#include <vector>

namespace omega
{

class EventSystem;

class TextureSystem
{
  public:
    TextureSystem(EventSystem& events);

    bool create_texture(const TextureAsset& asset, TextureHandle& out_handle);
    bool release_texture(TextureHandle handle);

    TextureData* get(TextureHandle handle);
    const TextureData* get(TextureHandle handle) const;

    uint32_t texture_count() const;

  private:
    struct TextureSlot
    {
        uint32_t generation = 0;
        bool occupied = false;
        TextureData data;
    };

    std::vector<TextureSlot> m_texture_slots;
    std::vector<uint32_t> m_free_texture_indices;

    uint32_t m_texture_count = 0;

    EventSystem& m_events;
};

} // namespace omega

#endif // OMEGA_TEXTURE_SYSTEM_HPP