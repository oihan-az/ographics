#include <omega/texture/texture_system.hpp>
#include <omega/texture/texture_event.hpp>

#include <omega/core/event/event_system.hpp>

#include <utility>

namespace omega
{

TextureSystem::TextureSystem(EventSystem& events) : m_events(events) {}

bool TextureSystem::create_texture(const TextureAsset& asset, TextureHandle& out_handle)
{
    if (!validate_texture_asset(asset))
    {
        return false;
    }

    // Obtain a texture slot
    uint32_t texture_index;
    if (!m_free_texture_indices.empty())
    {
        texture_index = m_free_texture_indices.back();
        m_free_texture_indices.pop_back();
    }
    else
    {
        texture_index = static_cast<uint32_t>(m_texture_slots.size());
        m_texture_slots.emplace_back();
    }

    TextureSlot& slot = m_texture_slots[texture_index];
    slot.occupied = true;

    const TextureHandle handle{texture_index, slot.generation};

    slot.data = TextureData{handle, asset};

    ++m_texture_count;
    out_handle = handle;

    m_events.publish(TextureCreatedEvent{handle});

    return true;
}

bool TextureSystem::release_texture(TextureHandle handle)
{
    if (!handle.is_valid() || handle.index >= m_texture_slots.size())
    {
        return false;
    }

    TextureSlot& slot = m_texture_slots[handle.index];
    if (!slot.occupied || slot.generation != handle.generation)
    {
        return false;
    }

    slot.occupied = false;
    slot.data = {};
    ++slot.generation;

    m_free_texture_indices.push_back(handle.index);
    --m_texture_count;

    m_events.publish(TextureReleasedEvent{handle});

    return true;
}

TextureData* TextureSystem::get(TextureHandle handle)
{
    if (!handle.is_valid() || handle.index >= m_texture_slots.size())
    {
        return nullptr;
    }

    TextureSlot& slot = m_texture_slots[handle.index];

    if (!slot.occupied || slot.generation != handle.generation)
    {
        return nullptr;
    }

    return &slot.data;
}

const TextureData* TextureSystem::get(TextureHandle handle) const
{
    if (!handle.is_valid() || handle.index >= m_texture_slots.size())
    {
        return nullptr;
    }

    const TextureSlot& slot = m_texture_slots[handle.index];

    if (!slot.occupied || slot.generation != handle.generation)
    {
        return nullptr;
    }

    return &slot.data;
}

uint32_t TextureSystem::texture_count() const
{
    return m_texture_count;
}

} // namespace omega