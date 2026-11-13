#ifndef OMEGA_GPU_TEXTURE_SYSTEM_HPP
#define OMEGA_GPU_TEXTURE_SYSTEM_HPP

#include <omega/texture/texture_handle.hpp>
#include <omega/texture/texture_asset.hpp>
#include <omega/texture/texture_view_handle.hpp>
#include <omega/texture/texture_view_desc.hpp>

#include <ogfx/image_view.hpp>

#include <vector>
#include <memory>
#include <limits>

namespace ogfx
{

class PhysicalDevice;
class Device;
class Buffer;
class CommandBuffer;

} // namespace ogfx

namespace orendergraph
{

class ResourcePoolRegistry;

} // namespace ogfx

namespace omega
{

class TextureSystem;
class EventSystem;

enum class GpuTextureState : uint8_t
{
    NotResident,
    UploadPending,
    Uploading,
    Resident
};

struct TextureUploadBatch
{
    uint32_t index = std::numeric_limits<uint32_t>::max();
    uint32_t generation = 0;

    bool is_valid() const
    {
        return index != std::numeric_limits<uint32_t>::max();
    }
};

class GpuTextureSystem
{
  public:
    GpuTextureSystem(const ogfx::PhysicalDevice& physical_device, const ogfx::Device& device,
                     orendergraph::ResourcePoolRegistry& resource_registry, 
                     const TextureSystem& texture_system, EventSystem& events);

    ~GpuTextureSystem();

    bool make_resident(TextureHandle handle);
    bool make_non_resident(TextureHandle handle);

    TextureUploadBatch synchronize(ogfx::CommandBuffer& cmd);
    void complete_uploads(TextureUploadBatch batch);

    GpuTextureState get_state(TextureHandle handle) const;
    bool is_resident(TextureHandle handle) const;
    bool is_uploading(TextureHandle handle) const;
    bool is_pending(TextureHandle handle) const;

    TextureViewHandle create_view(TextureHandle texture, const TextureViewDesc& desc);
    bool release_view(TextureViewHandle handle);
    bool is_view_valid(TextureViewHandle handle) const;

    const ogfx::Image* get_image(TextureHandle handle) const;
    const ogfx::ImageView* get_view(TextureViewHandle handle) const;

    uint32_t resident_texture_count() const;
    uint32_t texture_view_count() const;

  private:

    struct TextureSlot
    {
        uint32_t generation = 0;
        bool occupied = false;
        bool pending = false;
        bool requested_resident = false;

        GpuTextureState state = GpuTextureState::NotResident;

        std::unique_ptr<ogfx::Image> image;
        std::vector<TextureViewHandle> views;
    };

    struct TextureViewSlot
    {
        uint32_t generation = 0;
        bool occupied = false;
        bool pending = false;

        TextureHandle texture;
        TextureViewDesc desc;

        std::unique_ptr<ogfx::ImageView> view;
    };

    struct UploadBatch
    {
        uint32_t generation = 0;
        bool occupied = false;

        std::vector<std::unique_ptr<ogfx::Buffer>> staging_buffers;
        std::vector<TextureHandle> textures;
    };

    bool upload_texture(TextureHandle handle, ogfx::CommandBuffer& cmd, UploadBatch& batch);
    void release_gpu_texture(TextureHandle handle);

    bool create_gpu_view(TextureViewSlot& view, const ogfx::Image& image);
    void invalidate_texture_views(TextureSlot& slot);

    bool validate_view_desc(const TextureAsset& asset, const TextureViewDesc& desc) const;

    bool is_valid_slot(TextureHandle handle) const;
    bool is_valid_view_slot(TextureViewHandle handle) const;

    bool ensure_texture_slot(TextureHandle handle);
    TextureUploadBatch create_upload_batch();
    void process_pending_requests(ogfx::CommandBuffer& cmd, UploadBatch& batch);
    void process_pending_views();

    void enqueue_pending(TextureHandle handle);

    const TextureSystem& m_texture_system;
    EventSystem& m_events;

    const ogfx::PhysicalDevice& m_physical_device;
    const ogfx::Device& m_device;
    orendergraph::ResourcePoolRegistry& m_resources;

    std::vector<TextureSlot> m_texture_slots;
    std::vector<TextureViewSlot> m_view_slots;

    std::vector<uint32_t> m_free_view_indices;

    std::vector<TextureHandle> m_pending_textures;
    std::vector<TextureViewHandle> m_pending_views;

    std::vector<UploadBatch> m_upload_batches;
    std::vector<uint32_t> m_free_upload_batch_indices;

    uint32_t m_resident_texture_count = 0;
    uint32_t m_texture_view_count = 0;
};

} // namespace omega

#endif // OMEGA_GPU_TEXTURE_SYSTEM_HPP