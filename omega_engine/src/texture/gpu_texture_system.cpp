#include <omega/texture/gpu_texture_system.hpp>
#include <omega/texture/texture_system.hpp>
#include <omega/texture/texture_data.hpp>
#include <omega/texture/texture_asset.hpp>

#include <ogfx/buffer.hpp>
#include <ogfx/commands/transfer_commands.hpp>
#include <ogfx/commands/barrier_commands.hpp>
#include <ogfx/command_buffer.hpp>

namespace omega
{

GpuTextureSystem::GpuTextureSystem(const ogfx::PhysicalDevice& physical_device, const ogfx::Device& device,
                                   orendergraph::ResourcePoolRegistry& resource_registry,
                                   const TextureSystem& texture_system, EventSystem& events)
    : m_texture_system(texture_system), m_events(events), 
      m_physical_device(physical_device), m_device(device),
      m_resources(resource_registry)
{
}

GpuTextureSystem::~GpuTextureSystem()
{
    for (uint32_t i = 0; i < m_texture_slots.size(); ++i)
    {
        TextureSlot& slot = m_texture_slots[i];
        if (!slot.occupied)
            continue;

        release_gpu_texture(TextureHandle{i, slot.generation});
    }

    for (TextureViewSlot& slot : m_view_slots)
    {
        slot.view.reset();
        slot.occupied = false;
    }
}

bool GpuTextureSystem::make_resident(TextureHandle handle)
{
    if (!ensure_texture_slot(handle))
        return false;

    TextureSlot& slot = m_texture_slots[handle.index];
    slot.requested_resident = true;
    if (slot.state == GpuTextureState::NotResident)
        slot.state = GpuTextureState::UploadPending;

    if (slot.state != GpuTextureState::Resident)
        enqueue_pending(handle);

    return true;
}

bool GpuTextureSystem::make_non_resident(TextureHandle handle)
{
    if (!is_valid_slot(handle))
        return false;

    TextureSlot& slot = m_texture_slots[handle.index];

    slot.requested_resident = false;
    enqueue_pending(handle);

    return true;
}

TextureUploadBatch GpuTextureSystem::synchronize(ogfx::CommandBuffer& cmd)
{
    const TextureUploadBatch handle = create_upload_batch();
    UploadBatch& batch = m_upload_batches[handle.index];

    process_pending_requests(cmd, batch);
    process_pending_views();

    if (batch.textures.empty())
    {
        batch.occupied = false;
        ++batch.generation;
        m_free_upload_batch_indices.push_back(handle.index);

        return {};
    }

    return handle;
}

void GpuTextureSystem::complete_uploads(TextureUploadBatch handle)
{
    if (!handle.is_valid() || handle.index >= m_upload_batches.size())
    {
        return;
    }

    UploadBatch& batch = m_upload_batches[handle.index];
    if (!batch.occupied || batch.generation != handle.generation)
        return;

    for (const TextureHandle texture : batch.textures)
    {
        if (texture.index >= m_texture_slots.size())
            continue;

        TextureSlot& slot = m_texture_slots[texture.index];
        if (!slot.occupied || slot.generation != texture.generation)
            continue;

        // The GPU has finished using the staging buffer and image upload
        if (m_texture_system.get(texture) != nullptr && slot.requested_resident)
        {
            slot.state = GpuTextureState::Resident;
            ++m_resident_texture_count;

            for (const auto view_handle : slot.views)
            {
                if (!is_valid_view_slot(view_handle))
                    continue;

                auto& view_slot = m_view_slots[view_handle.index];
                if (view_slot.pending)
                    m_pending_views.push_back(view_handle);
            }
        }
        else
        {
            // It may have been requested non-resident while uploading.
            release_gpu_texture(texture);

            if (m_texture_system.get(texture) == nullptr)
                invalidate_texture_views(slot);
        }
    }

    // The GPU has completed the submission.
    batch.staging_buffers.clear();
    batch.textures.clear();

    batch.occupied = false;
    ++batch.generation;
    m_free_upload_batch_indices.push_back(handle.index);
}

GpuTextureState GpuTextureSystem::get_state(TextureHandle handle) const
{
    if (!is_valid_slot(handle))
        return GpuTextureState::NotResident;

    return m_texture_slots[handle.index].state;
}

bool GpuTextureSystem::is_resident(TextureHandle handle) const
{
    return get_state(handle) == GpuTextureState::Resident;
}

bool GpuTextureSystem::is_uploading(TextureHandle handle) const
{
    return get_state(handle) == GpuTextureState::Uploading;
}

bool GpuTextureSystem::is_pending(TextureHandle handle) const
{
    if (!is_valid_slot(handle))
        return false;

    const TextureSlot& slot = m_texture_slots[handle.index];

    return slot.pending || slot.state == GpuTextureState::UploadPending || slot.state == GpuTextureState::Uploading;
}

TextureViewHandle GpuTextureSystem::create_view(TextureHandle texture, const TextureViewDesc& desc)
{
    if (!ensure_texture_slot(texture))
        return {};

    const TextureData* texture_data = m_texture_system.get(texture);
    if (texture_data == nullptr || !validate_view_desc(texture_data->asset, desc))
    {
        return {};
    }

    uint32_t index;

    if (!m_free_view_indices.empty())
    {
        index = m_free_view_indices.back();
        m_free_view_indices.pop_back();
    }
    else
    {
        index = static_cast<uint32_t>(m_view_slots.size());
        m_view_slots.emplace_back();
    }

    TextureViewSlot& view_slot = m_view_slots[index];
    view_slot.occupied = true;
    view_slot.pending = true;
    view_slot.texture = texture;
    view_slot.desc = desc;
    view_slot.view.reset();

    const TextureViewHandle handle{index, view_slot.generation};

    m_texture_slots[texture.index].views.push_back(handle);
    m_pending_views.push_back(handle);

    ++m_texture_view_count;
    return handle;
}

bool GpuTextureSystem::release_view(TextureViewHandle handle)
{
    if (!is_valid_view_slot(handle))
        return false;

    TextureViewSlot& view_slot = m_view_slots[handle.index];
    const TextureHandle texture = view_slot.texture;

    if (is_valid_slot(texture))
    {
        TextureSlot& texture_slot = m_texture_slots[texture.index];

        auto& views = texture_slot.views;
        views.erase(std::remove(views.begin(), views.end(), handle), views.end());
    }

    view_slot.view.reset();
    view_slot.occupied = false;
    view_slot.pending = false;
    view_slot.texture = {};
    view_slot.desc = {};

    if (m_texture_view_count > 0)
        --m_texture_view_count;

    ++view_slot.generation;
    m_free_view_indices.push_back(handle.index);

    return true;
}

bool GpuTextureSystem::is_view_valid(TextureViewHandle handle) const
{
    return is_valid_view_slot(handle);
}

const ogfx::Image* GpuTextureSystem::get_image(TextureHandle handle) const
{
    if (!is_resident(handle))
        return nullptr;

    return m_texture_slots[handle.index].image.get();
}

const ogfx::ImageView* GpuTextureSystem::get_view(TextureViewHandle handle) const
{
    if (!is_valid_view_slot(handle))
        return nullptr;

    const TextureViewSlot& slot = m_view_slots[handle.index];
    if (slot.pending)
        return nullptr;

    return slot.view.get();
}

uint32_t GpuTextureSystem::resident_texture_count() const
{
    return m_resident_texture_count;
}

uint32_t GpuTextureSystem::texture_view_count() const
{
    return m_texture_view_count;
}

bool GpuTextureSystem::upload_texture(TextureHandle handle, ogfx::CommandBuffer& cmd, UploadBatch& batch)
{
    const TextureData* texture_data = m_texture_system.get(handle);
    if (texture_data == nullptr)
        return false;

    const TextureAsset& asset = texture_data->asset;
    if (!validate_texture_asset(asset))
        return false;

    TextureSlot& slot = m_texture_slots[handle.index];

    ogfx::ImageDesc image_desc{};
    image_desc.type = asset.type;
    image_desc.width = asset.width;
    image_desc.height = asset.height;
    image_desc.depth = asset.depth;
    image_desc.mip_levels = asset.mip_levels;
    image_desc.array_layers = asset.array_layers;
    image_desc.format = asset.format;
    image_desc.usage = ogfx::ImageUsage::TransferDst | ogfx::ImageUsage::Sampled;

    auto image = std::make_unique<ogfx::Image>(m_physical_device, m_device, image_desc);
    image->allocate_memory();
    image->bind_memory();

    // Pack all subresources into one staging buffer
    std::vector<uint64_t> offsets;
    offsets.reserve(asset.subresources.size());
    uint64_t total_size = 0;
    for (const TextureSubresource& subresource : asset.subresources)
    {
        offsets.push_back(total_size);

        if (subresource.data.size() > std::numeric_limits<uint64_t>::max() - total_size)
        {
            return false;
        }

        total_size += subresource.data.size();
    }

    if (total_size == 0)
        return false;

    ogfx::BufferDesc staging_desc{};
    staging_desc.size = total_size;
    staging_desc.usage = ogfx::BufferUsage::TransferSrc;
    staging_desc.memory_properties = ogfx::MemoryProperty::HostVisible | ogfx::MemoryProperty::HostCoherent;
    auto staging_buffer = std::make_unique<ogfx::Buffer>(m_physical_device, m_device, staging_desc);
    staging_buffer->allocate_memory();
    staging_buffer->bind_memory();

    std::vector<std::byte> staging_data;
    staging_data.reserve(static_cast<size_t>(total_size));
    for (const TextureSubresource& subresource : asset.subresources)
    {
        staging_data.insert(staging_data.end(), subresource.data.begin(), subresource.data.end());
    }

    ogfx::BufferWriteInfo write_info{};
    write_info.data = staging_data.data();
    write_info.size = total_size;
    staging_buffer->write(write_info);

    ogfx::ImageTransitionInfo transition_to_transfer{};
    transition_to_transfer.image = image.get();
    transition_to_transfer.aspect = ogfx::ImageAspect::Color;
    transition_to_transfer.old_state = ogfx::ImageState::Undefined;
    transition_to_transfer.new_state = ogfx::ImageState::TransferDst;
    transition_to_transfer.base_mip_level = 0;
    transition_to_transfer.mip_level_count = asset.mip_levels;
    transition_to_transfer.base_array_layer = 0;
    transition_to_transfer.array_layer_count = asset.array_layers;

    cmd.transition_image(transition_to_transfer);

    for (size_t i = 0; i < asset.subresources.size(); ++i)
    {
        const TextureSubresource& subresource = asset.subresources[i];

        const uint32_t mip_width = std::max(1u, asset.width >> subresource.mip_level);
        const uint32_t mip_height = std::max(1u, asset.height >> subresource.mip_level);
        const uint32_t mip_depth = std::max(1u, asset.depth >> subresource.mip_level);

        ogfx::BufferToImageCopyInfo copy_info{};
        copy_info.buffer = staging_buffer.get();
        copy_info.buffer_offset = offsets[i];
        copy_info.image = image.get();

        copy_info.width = mip_width;
        copy_info.height = mip_height;
        copy_info.depth = asset.type == ogfx::ImageType::Image3D ? mip_depth : 1;

        copy_info.mip_level = subresource.mip_level;
        copy_info.base_array_layer = subresource.array_layer;
        copy_info.array_layer_count = 1;

        cmd.copy_buffer_to_image(copy_info);
    }

    ogfx::ImageTransitionInfo transition_to_shader_read{};
    transition_to_shader_read.image = image.get();
    transition_to_shader_read.aspect = ogfx::ImageAspect::Color;
    transition_to_shader_read.old_state = ogfx::ImageState::TransferDst;
    transition_to_shader_read.new_state = ogfx::ImageState::ShaderRead;
    transition_to_shader_read.base_mip_level = 0;
    transition_to_shader_read.mip_level_count = asset.mip_levels;
    transition_to_shader_read.base_array_layer = 0;
    transition_to_shader_read.array_layer_count = asset.array_layers;

    cmd.transition_image(transition_to_shader_read);

    // Create its requested views
    slot.image = std::move(image);
    for (const TextureViewHandle view_handle : slot.views)
    {
        if (!is_valid_view_slot(view_handle))
            continue;

        TextureViewSlot& view_slot = m_view_slots[view_handle.index];

        if (!create_gpu_view(view_slot, *slot.image))
        {
            release_gpu_texture(handle);
            return false;
        }
    }

    slot.state = GpuTextureState::Uploading;

    batch.staging_buffers.push_back(std::move(staging_buffer));
    batch.textures.push_back(handle);

    return true;
}

void GpuTextureSystem::release_gpu_texture(TextureHandle handle)
{
    if (handle.index >= m_texture_slots.size())
        return;

    TextureSlot& slot = m_texture_slots[handle.index];
    if (!slot.occupied || slot.generation != handle.generation)
        return;

    for (const TextureViewHandle view_handle : slot.views)
    {
        if (!is_valid_view_slot(view_handle))
            continue;

        TextureViewSlot& view_slot = m_view_slots[view_handle.index];

        view_slot.view.reset();
        view_slot.pending = true;
    }

    if (slot.state == GpuTextureState::Resident && m_resident_texture_count > 0)
    {
        --m_resident_texture_count;
    }

    slot.image.reset();
    slot.state = GpuTextureState::NotResident;
}

void GpuTextureSystem::invalidate_texture_views(TextureSlot& slot)
{
    for (const TextureViewHandle handle : slot.views)
    {
        if (!is_valid_view_slot(handle))
            continue;

        TextureViewSlot& view_slot = m_view_slots[handle.index];

        view_slot.view.reset();
        view_slot.occupied = false;
        view_slot.texture = {};
        view_slot.desc = {};

        ++view_slot.generation;
        m_free_view_indices.push_back(handle.index);

        if (m_texture_view_count > 0)
            --m_texture_view_count;
    }

    slot.views.clear();
}

bool GpuTextureSystem::create_gpu_view(TextureViewSlot& view_slot, const ogfx::Image& image)
{
    ogfx::ImageViewDesc desc{};
    desc.image = &image;
    desc.type = view_slot.desc.type;
    desc.aspect = view_slot.desc.aspect;

    desc.base_mip_level = view_slot.desc.base_mip_level;
    desc.mip_level_count = view_slot.desc.mip_level_count;

    desc.base_array_layer = view_slot.desc.base_array_layer;
    desc.array_layer_count = view_slot.desc.array_layer_count;

    view_slot.view = std::make_unique<ogfx::ImageView>(desc);
    return view_slot.view != nullptr;
}

bool GpuTextureSystem::validate_view_desc(const TextureAsset& asset, const TextureViewDesc& desc) const
{
    if (desc.type == ogfx::ImageViewType::Undefined || desc.mip_level_count == 0 || desc.array_layer_count == 0)
    {
        return false;
    }

    if (desc.base_mip_level >= asset.mip_levels || desc.mip_level_count > asset.mip_levels - desc.base_mip_level)
    {
        return false;
    }

    if (desc.base_array_layer >= asset.array_layers ||
        desc.array_layer_count > asset.array_layers - desc.base_array_layer)
    {
        return false;
    }

    switch (desc.type)
    {
    case ogfx::ImageViewType::View1D:
        return asset.type == ogfx::ImageType::Image1D && desc.array_layer_count == 1;

    case ogfx::ImageViewType::View1DArray:
        return asset.type == ogfx::ImageType::Image1D;

    case ogfx::ImageViewType::View2D:
        return (asset.type == ogfx::ImageType::Image2D || asset.type == ogfx::ImageType::Cube) &&
               desc.array_layer_count == 1;

    case ogfx::ImageViewType::View2DArray:
        return asset.type == ogfx::ImageType::Image2D || asset.type == ogfx::ImageType::Cube;

    case ogfx::ImageViewType::View3D:
        return asset.type == ogfx::ImageType::Image3D && desc.base_array_layer == 0 && desc.array_layer_count == 1;

    case ogfx::ImageViewType::Cube:
        return asset.type == ogfx::ImageType::Cube && desc.array_layer_count == 6 && desc.base_array_layer % 6 == 0;

    case ogfx::ImageViewType::CubeArray:
        return asset.type == ogfx::ImageType::Cube && desc.array_layer_count % 6 == 0 && desc.base_array_layer % 6 == 0;

    default:
        return false;
    }
}

bool GpuTextureSystem::is_valid_slot(TextureHandle handle) const
{
    if (!handle.is_valid() || handle.index >= m_texture_slots.size())
        return false;

    const TextureSlot& slot = m_texture_slots[handle.index];

    return slot.occupied && slot.generation == handle.generation && m_texture_system.get(handle) != nullptr;
}

bool GpuTextureSystem::is_valid_view_slot(TextureViewHandle handle) const
{
    if (!handle.is_valid() || handle.index >= m_view_slots.size())
        return false;

    const TextureViewSlot& slot = m_view_slots[handle.index];

    return slot.occupied && slot.generation == handle.generation;
}

bool GpuTextureSystem::ensure_texture_slot(TextureHandle handle)
{
    if (!handle.is_valid() || m_texture_system.get(handle) == nullptr)
        return false;

    if (handle.index >= m_texture_slots.size())
        m_texture_slots.resize(static_cast<size_t>(handle.index) + 1);

    TextureSlot& slot = m_texture_slots[handle.index];
    if (slot.occupied && slot.generation != handle.generation)
    {
        release_gpu_texture(TextureHandle{handle.index, slot.generation});
        invalidate_texture_views(slot);
        slot = {};
    }

    if (!slot.occupied)
    {
        slot.generation = handle.generation;
        slot.occupied = true;
    }

    return true;
}

TextureUploadBatch GpuTextureSystem::create_upload_batch()
{
    uint32_t index;

    if (!m_free_upload_batch_indices.empty())
    {
        index = m_free_upload_batch_indices.back();
        m_free_upload_batch_indices.pop_back();
    }
    else
    {
        index = static_cast<uint32_t>(m_upload_batches.size());
        m_upload_batches.emplace_back();
    }

    UploadBatch& batch = m_upload_batches[index];
    batch.occupied = true;
    batch.staging_buffers.clear();
    batch.textures.clear();

    return {index, batch.generation};
}

void GpuTextureSystem::process_pending_requests(ogfx::CommandBuffer& cmd, UploadBatch& batch)
{
    std::vector<TextureHandle> pending;
    pending.swap(m_pending_textures);

    for (const TextureHandle handle : pending)
    {
        if (!is_valid_slot(handle))
            continue;

        TextureSlot& slot = m_texture_slots[handle.index];
        if (!slot.pending)
            continue;

        slot.pending = false;

        if (slot.requested_resident)
        {
            if (slot.state == GpuTextureState::NotResident || slot.state == GpuTextureState::UploadPending)
            {
                slot.state = GpuTextureState::UploadPending;
                if (!upload_texture(handle, cmd, batch))
                    enqueue_pending(handle);
            }
        }
        else
        {
            // An in-flight upload cannot be destroyed yet
            if (slot.state == GpuTextureState::Uploading)
                continue;

            release_gpu_texture(handle);
        }
    }
}

void GpuTextureSystem::process_pending_views()
{
    std::vector<TextureViewHandle> pending;
    pending.swap(m_pending_views);

    for (const TextureViewHandle handle : pending)
    {
        if (!is_valid_view_slot(handle))
            continue;

        TextureViewSlot& view_slot = m_view_slots[handle.index];

        if (!view_slot.pending)
            continue;

        if (!is_valid_slot(view_slot.texture))
            continue;

        TextureSlot& texture_slot = m_texture_slots[view_slot.texture.index];

        if (texture_slot.state != GpuTextureState::Resident || !texture_slot.image)
        {
            // Keep waiting until the texture is resident
            m_pending_views.push_back(handle);
            continue;
        }

        if (create_gpu_view(view_slot, *texture_slot.image))
            view_slot.pending = false;
        else
            m_pending_views.push_back(handle);
    }
}

void GpuTextureSystem::enqueue_pending(TextureHandle handle)
{
    if (!is_valid_slot(handle))
        return;

    TextureSlot& slot = m_texture_slots[handle.index];

    if (slot.pending)
        return;

    slot.pending = true;
    m_pending_textures.push_back(handle);
}

}