#include <gtest/gtest.h>

#include <omega/texture/texture_system.hpp>
#include <omega/texture/gpu_texture_system.hpp>
#include <omega/core/event/event_system.hpp>

#include <orendergraph/resource_pool_registry.hpp>

#include <ogfx/command_buffer.hpp>
#include <ogfx/command_pool.hpp>
#include <ogfx/device.hpp>
#include <ogfx/fence.hpp>
#include <ogfx/instance.hpp>
#include <ogfx/physical_device.hpp>
#include <ogfx/queue.hpp>

#include <memory>
#include <vector>

namespace
{

class GpuTextureSystemTests : public ::testing::Test
{
  protected:
    static void SetUpTestSuite()
    {
        ogfx::InstanceDesc instance_desc;
        instance_desc.application_name = "ogfx Instance Test";
        instance_desc.engine_name = "ographics";
        instance_desc.debug = true;
        instance_desc.extensions = {};

        s_instance = std::make_unique<ogfx::Instance>(instance_desc);

        const auto devices = s_instance->enumerate_physical_devices();
        s_physical_device = std::make_unique<ogfx::PhysicalDevice>(s_instance->pick_physical_device(devices));

        ogfx::DeviceDesc device_desc;
        device_desc.queues = ogfx::DeviceQueue::Graphics;

        s_device = std::make_unique<ogfx::Device>(*s_physical_device, device_desc);
    }

    static void TearDownTestSuite()
    {
        s_device.reset();
        s_physical_device.reset();
        s_instance.reset();
    }

    void SetUp() override
    {
        m_texture_system = std::make_unique<omega::TextureSystem>(m_events);

        m_gpu_texture_system = std::make_unique<omega::GpuTextureSystem>(*s_physical_device, *s_device, m_resource_registry, *m_texture_system, m_events);

        auto& queue = s_device->queue(ogfx::QueueType::Graphics);

        ogfx::CommandPoolDesc pool_desc;
        pool_desc.transient = true;
        pool_desc.resettable = false;

        m_command_pool = std::make_unique<ogfx::CommandPool>(*s_device, queue, pool_desc);
    }

    void TearDown() override
    {
        m_gpu_texture_system.reset();
        m_texture_system.reset();
        m_command_pool.reset();
    }

    omega::TextureAsset make_texture_asset(uint32_t width = 4, uint32_t height = 4) const
    {
        omega::TextureAsset asset;
        asset.type = ogfx::ImageType::Image2D;
        asset.width = width;
        asset.height = height;
        asset.depth = 1;
        asset.mip_levels = 1;
        asset.array_layers = 1;
        asset.format = ogfx::Format::RGBA8_UNORM;

        omega::TextureSubresource subresource;
        subresource.mip_level = 0;
        subresource.array_layer = 0;
        subresource.data.resize(static_cast<size_t>(width) * height * 4, std::byte{0x7F});

        asset.subresources.push_back(std::move(subresource));
        return asset;
    }

    omega::TextureHandle create_texture()
    {
        omega::TextureHandle handle;
        const auto asset = make_texture_asset();

        EXPECT_TRUE(m_texture_system->create_texture(asset, handle));
        return handle;
    }

    omega::TextureUploadBatch upload_pending_textures()
    {
        ogfx::CommandBuffer cmd(*m_command_pool);

        cmd.begin();
        const auto batch = m_gpu_texture_system->synchronize(cmd);
        cmd.end();

        if (!batch.is_valid())
            return batch;

        auto& queue = s_device->queue(ogfx::QueueType::Graphics);

        ogfx::Fence fence(*s_device);

        ogfx::SubmitDesc submit_desc;
        submit_desc.command_buffers.push_back(&cmd);
        submit_desc.fence = &fence;

        queue.submit(submit_desc);
        fence.wait();

        m_gpu_texture_system->complete_uploads(batch);
        return batch;
    }

    omega::TextureViewDesc make_view_desc() const
    {
        omega::TextureViewDesc desc;
        desc.type = ogfx::ImageViewType::View2D;
        desc.aspect = ogfx::ImageAspect::Color;
        desc.base_mip_level = 0;
        desc.mip_level_count = 1;
        desc.base_array_layer = 0;
        desc.array_layer_count = 1;
        return desc;
    }

    inline static std::unique_ptr<ogfx::Instance> s_instance;
    inline static std::unique_ptr<ogfx::PhysicalDevice> s_physical_device;
    inline static std::unique_ptr<ogfx::Device> s_device;

    omega::EventSystem m_events;
    orendergraph::ResourcePoolRegistry m_resource_registry;

    std::unique_ptr<omega::TextureSystem> m_texture_system;
    std::unique_ptr<omega::GpuTextureSystem> m_gpu_texture_system;
    std::unique_ptr<ogfx::CommandPool> m_command_pool;
};

TEST_F(GpuTextureSystemTests, TextureIsNotResidentByDefault)
{
    const auto texture = create_texture();

    EXPECT_EQ(m_gpu_texture_system->get_state(texture), omega::GpuTextureState::NotResident);

    EXPECT_FALSE(m_gpu_texture_system->is_resident(texture));
    EXPECT_FALSE(m_gpu_texture_system->is_uploading(texture));
    EXPECT_FALSE(m_gpu_texture_system->is_pending(texture));
    EXPECT_EQ(m_gpu_texture_system->get_image(texture), nullptr);
    EXPECT_EQ(m_gpu_texture_system->resident_texture_count(), 0);
}

TEST_F(GpuTextureSystemTests, MakeResidentUploadsTexture)
{
    const auto texture = create_texture();

    ASSERT_TRUE(m_gpu_texture_system->make_resident(texture));

    EXPECT_EQ(m_gpu_texture_system->get_state(texture), omega::GpuTextureState::UploadPending);

    EXPECT_TRUE(m_gpu_texture_system->is_pending(texture));
    EXPECT_FALSE(m_gpu_texture_system->is_resident(texture));

    upload_pending_textures();

    EXPECT_EQ(m_gpu_texture_system->get_state(texture), omega::GpuTextureState::Resident);

    EXPECT_TRUE(m_gpu_texture_system->is_resident(texture));
    EXPECT_FALSE(m_gpu_texture_system->is_uploading(texture));
    EXPECT_FALSE(m_gpu_texture_system->is_pending(texture));
    EXPECT_NE(m_gpu_texture_system->get_image(texture), nullptr);
    EXPECT_EQ(m_gpu_texture_system->resident_texture_count(), 1);
}

TEST_F(GpuTextureSystemTests, ViewCanBeCreatedBeforeTextureIsResident)
{
    const auto texture = create_texture();

    const auto view = m_gpu_texture_system->create_view(texture, make_view_desc());

    ASSERT_TRUE(view.is_valid());
    EXPECT_TRUE(m_gpu_texture_system->is_view_valid(view));
    EXPECT_EQ(m_gpu_texture_system->get_view(view), nullptr);

    ASSERT_TRUE(m_gpu_texture_system->make_resident(texture));

    upload_pending_textures();

    // View creation is deferred until synchronize().
    EXPECT_EQ(m_gpu_texture_system->get_view(view), nullptr);

    ogfx::CommandBuffer cmd(*m_command_pool);
    cmd.begin();
    const auto batch = m_gpu_texture_system->synchronize(cmd);
    cmd.end();

    EXPECT_FALSE(batch.is_valid());
    EXPECT_NE(m_gpu_texture_system->get_view(view), nullptr);
}

TEST_F(GpuTextureSystemTests, ViewCreationIsDeferredForResidentTexture)
{
    const auto texture = create_texture();

    ASSERT_TRUE(m_gpu_texture_system->make_resident(texture));
    upload_pending_textures();

    const auto view = m_gpu_texture_system->create_view(texture, make_view_desc());

    ASSERT_TRUE(view.is_valid());
    EXPECT_EQ(m_gpu_texture_system->get_view(view), nullptr);

    ogfx::CommandBuffer cmd(*m_command_pool);
    cmd.begin();
    const auto batch = m_gpu_texture_system->synchronize(cmd);
    cmd.end();

    EXPECT_FALSE(batch.is_valid());
    EXPECT_NE(m_gpu_texture_system->get_view(view), nullptr);
}

TEST_F(GpuTextureSystemTests, ReleasingViewInvalidatesHandle)
{
    const auto texture = create_texture();

    const auto view = m_gpu_texture_system->create_view(texture, make_view_desc());

    ASSERT_TRUE(view.is_valid());
    ASSERT_TRUE(m_gpu_texture_system->is_view_valid(view));

    EXPECT_TRUE(m_gpu_texture_system->release_view(view));

    EXPECT_FALSE(m_gpu_texture_system->is_view_valid(view));
    EXPECT_EQ(m_gpu_texture_system->get_view(view), nullptr);
    EXPECT_EQ(m_gpu_texture_system->texture_view_count(), 0);
}

TEST_F(GpuTextureSystemTests, MakeNonResidentReleasesTexture)
{
    const auto texture = create_texture();

    ASSERT_TRUE(m_gpu_texture_system->make_resident(texture));
    upload_pending_textures();

    ASSERT_TRUE(m_gpu_texture_system->is_resident(texture));

    ASSERT_TRUE(m_gpu_texture_system->make_non_resident(texture));

    ogfx::CommandBuffer cmd(*m_command_pool);
    cmd.begin();
    const auto batch = m_gpu_texture_system->synchronize(cmd);
    cmd.end();

    EXPECT_FALSE(batch.is_valid());
    EXPECT_FALSE(m_gpu_texture_system->is_resident(texture));
    EXPECT_EQ(m_gpu_texture_system->get_image(texture), nullptr);
    EXPECT_EQ(m_gpu_texture_system->resident_texture_count(), 0);
}

TEST_F(GpuTextureSystemTests, RequestedViewsSurviveNonResidency)
{
    const auto texture = create_texture();

    const auto view = m_gpu_texture_system->create_view(texture, make_view_desc());

    ASSERT_TRUE(view.is_valid());
    ASSERT_TRUE(m_gpu_texture_system->make_resident(texture));

    upload_pending_textures();

    ogfx::CommandBuffer create_view_cmd(*m_command_pool);
    create_view_cmd.begin();
    m_gpu_texture_system->synchronize(create_view_cmd);
    create_view_cmd.end();

    ASSERT_NE(m_gpu_texture_system->get_view(view), nullptr);

    ASSERT_TRUE(m_gpu_texture_system->make_non_resident(texture));

    ogfx::CommandBuffer release_cmd(*m_command_pool);
    release_cmd.begin();
    m_gpu_texture_system->synchronize(release_cmd);
    release_cmd.end();

    EXPECT_TRUE(m_gpu_texture_system->is_view_valid(view));
    EXPECT_EQ(m_gpu_texture_system->get_view(view), nullptr);

    ASSERT_TRUE(m_gpu_texture_system->make_resident(texture));
    upload_pending_textures();

    ogfx::CommandBuffer recreate_view_cmd(*m_command_pool);
    recreate_view_cmd.begin();
    m_gpu_texture_system->synchronize(recreate_view_cmd);
    recreate_view_cmd.end();

    EXPECT_NE(m_gpu_texture_system->get_view(view), nullptr);
}

TEST_F(GpuTextureSystemTests, MakeResidentRejectsInvalidHandle)
{
    omega::TextureHandle invalid;

    EXPECT_FALSE(m_gpu_texture_system->make_resident(invalid));
    EXPECT_FALSE(m_gpu_texture_system->is_resident(invalid));
    EXPECT_EQ(m_gpu_texture_system->get_state(invalid), omega::GpuTextureState::NotResident);
}

TEST_F(GpuTextureSystemTests, RepeatedMakeResidentDoesNotDuplicateUpload)
{
    const auto texture = create_texture();

    ASSERT_TRUE(m_gpu_texture_system->make_resident(texture));
    ASSERT_TRUE(m_gpu_texture_system->make_resident(texture));

    EXPECT_EQ(m_gpu_texture_system->get_state(texture), omega::GpuTextureState::UploadPending);

    upload_pending_textures();

    EXPECT_TRUE(m_gpu_texture_system->is_resident(texture));
    EXPECT_EQ(m_gpu_texture_system->resident_texture_count(), 1);
}

TEST_F(GpuTextureSystemTests, RepeatedMakeNonResidentIsSafe)
{
    const auto texture = create_texture();

    ASSERT_TRUE(m_gpu_texture_system->make_resident(texture));
    upload_pending_textures();

    ASSERT_TRUE(m_gpu_texture_system->make_non_resident(texture));
    ASSERT_TRUE(m_gpu_texture_system->make_non_resident(texture));

    ogfx::CommandBuffer cmd(*m_command_pool);
    cmd.begin();
    const auto batch = m_gpu_texture_system->synchronize(cmd);
    cmd.end();

    EXPECT_FALSE(batch.is_valid());
    EXPECT_FALSE(m_gpu_texture_system->is_resident(texture));
    EXPECT_EQ(m_gpu_texture_system->resident_texture_count(), 0);
}

TEST_F(GpuTextureSystemTests, CancelPendingUploadBeforeSynchronize)
{
    const auto texture = create_texture();

    ASSERT_TRUE(m_gpu_texture_system->make_resident(texture));
    ASSERT_TRUE(m_gpu_texture_system->is_pending(texture));

    ASSERT_TRUE(m_gpu_texture_system->make_non_resident(texture));

    ogfx::CommandBuffer cmd(*m_command_pool);
    cmd.begin();
    const auto batch = m_gpu_texture_system->synchronize(cmd);
    cmd.end();

    EXPECT_FALSE(batch.is_valid());
    EXPECT_FALSE(m_gpu_texture_system->is_resident(texture));
    EXPECT_FALSE(m_gpu_texture_system->is_uploading(texture));
    EXPECT_EQ(m_gpu_texture_system->get_image(texture), nullptr);
    EXPECT_EQ(m_gpu_texture_system->resident_texture_count(), 0);
}

TEST_F(GpuTextureSystemTests, UploadsMultipleTexturesInOneBatch)
{
    const auto texture_a = create_texture();
    const auto texture_b = create_texture();
    const auto texture_c = create_texture();

    ASSERT_TRUE(m_gpu_texture_system->make_resident(texture_a));
    ASSERT_TRUE(m_gpu_texture_system->make_resident(texture_b));
    ASSERT_TRUE(m_gpu_texture_system->make_resident(texture_c));

    upload_pending_textures();

    EXPECT_TRUE(m_gpu_texture_system->is_resident(texture_a));
    EXPECT_TRUE(m_gpu_texture_system->is_resident(texture_b));
    EXPECT_TRUE(m_gpu_texture_system->is_resident(texture_c));

    EXPECT_NE(m_gpu_texture_system->get_image(texture_a), nullptr);
    EXPECT_NE(m_gpu_texture_system->get_image(texture_b), nullptr);
    EXPECT_NE(m_gpu_texture_system->get_image(texture_c), nullptr);

    EXPECT_EQ(m_gpu_texture_system->resident_texture_count(), 3);
}

TEST_F(GpuTextureSystemTests, ReleasingViewAllowsHandleReuse)
{
    const auto texture = create_texture();

    const auto first = m_gpu_texture_system->create_view(texture, make_view_desc());

    ASSERT_TRUE(first.is_valid());
    ASSERT_TRUE(m_gpu_texture_system->release_view(first));

    const auto second = m_gpu_texture_system->create_view(texture, make_view_desc());

    ASSERT_TRUE(second.is_valid());

    EXPECT_EQ(first.index, second.index);
    EXPECT_NE(first.generation, second.generation);

    EXPECT_FALSE(m_gpu_texture_system->is_view_valid(first));
    EXPECT_TRUE(m_gpu_texture_system->is_view_valid(second));
}

TEST_F(GpuTextureSystemTests, MakeNonResidentWhileUploading)
{
    const auto texture = create_texture();

    ASSERT_TRUE(m_gpu_texture_system->make_resident(texture));

    ogfx::CommandBuffer cmd(*m_command_pool);
    cmd.begin();
    const auto batch = m_gpu_texture_system->synchronize(cmd);
    cmd.end();

    ASSERT_TRUE(batch.is_valid());
    EXPECT_EQ(m_gpu_texture_system->get_state(texture), omega::GpuTextureState::Uploading);

    ASSERT_TRUE(m_gpu_texture_system->make_non_resident(texture));

    auto& queue = s_device->queue(ogfx::QueueType::Graphics);
    ogfx::Fence fence(*s_device);

    ogfx::SubmitDesc submit_desc;
    submit_desc.command_buffers.push_back(&cmd);
    submit_desc.fence = &fence;

    queue.submit(submit_desc);
    fence.wait();

    m_gpu_texture_system->complete_uploads(batch);

    EXPECT_FALSE(m_gpu_texture_system->is_resident(texture));
    EXPECT_EQ(m_gpu_texture_system->get_state(texture), omega::GpuTextureState::NotResident);
    EXPECT_EQ(m_gpu_texture_system->get_image(texture), nullptr);
    EXPECT_EQ(m_gpu_texture_system->resident_texture_count(), 0);
}

TEST_F(GpuTextureSystemTests, MultipleUploadBatchesCompleteIndependently)
{
    const auto texture_a = create_texture();
    const auto texture_b = create_texture();

    ASSERT_TRUE(m_gpu_texture_system->make_resident(texture_a));

    ogfx::CommandBuffer cmd_a(*m_command_pool);
    cmd_a.begin();
    const auto batch_a = m_gpu_texture_system->synchronize(cmd_a);
    cmd_a.end();

    ASSERT_TRUE(batch_a.is_valid());

    ASSERT_TRUE(m_gpu_texture_system->make_resident(texture_b));

    ogfx::CommandBuffer cmd_b(*m_command_pool);
    cmd_b.begin();
    const auto batch_b = m_gpu_texture_system->synchronize(cmd_b);
    cmd_b.end();

    ASSERT_TRUE(batch_b.is_valid());

    auto& queue = s_device->queue(ogfx::QueueType::Graphics);
    ogfx::Fence fence_a(*s_device);
    ogfx::Fence fence_b(*s_device);

    ogfx::SubmitDesc submit_a;
    submit_a.command_buffers.push_back(&cmd_a);
    submit_a.fence = &fence_a;
    queue.submit(submit_a);

    ogfx::SubmitDesc submit_b;
    submit_b.command_buffers.push_back(&cmd_b);
    submit_b.fence = &fence_b;
    queue.submit(submit_b);

    fence_a.wait();
    m_gpu_texture_system->complete_uploads(batch_a);

    EXPECT_TRUE(m_gpu_texture_system->is_resident(texture_a));
    EXPECT_EQ(m_gpu_texture_system->get_state(texture_b), omega::GpuTextureState::Uploading);

    fence_b.wait();
    m_gpu_texture_system->complete_uploads(batch_b);

    EXPECT_TRUE(m_gpu_texture_system->is_resident(texture_b));
    EXPECT_EQ(m_gpu_texture_system->resident_texture_count(), 2);
}

TEST_F(GpuTextureSystemTests, RepeatedResidencyCycles)
{
    const auto texture = create_texture();

    for (uint32_t i = 0; i < 10; ++i)
    {
        ASSERT_TRUE(m_gpu_texture_system->make_resident(texture));
        upload_pending_textures();

        ASSERT_TRUE(m_gpu_texture_system->is_resident(texture));
        ASSERT_EQ(m_gpu_texture_system->resident_texture_count(), 1);

        ASSERT_TRUE(m_gpu_texture_system->make_non_resident(texture));

        ogfx::CommandBuffer cmd(*m_command_pool);
        cmd.begin();
        const auto batch = m_gpu_texture_system->synchronize(cmd);
        cmd.end();

        EXPECT_FALSE(batch.is_valid());
        EXPECT_FALSE(m_gpu_texture_system->is_resident(texture));
        EXPECT_EQ(m_gpu_texture_system->resident_texture_count(), 0);
    }
}

TEST_F(GpuTextureSystemTests, StressTextureResidencyAndViews)
{
    constexpr uint32_t texture_count = 64;
    constexpr uint32_t cycles = 5;

    std::vector<omega::TextureHandle> textures;
    std::vector<omega::TextureViewHandle> views;

    textures.reserve(texture_count);
    views.reserve(texture_count);

    for (uint32_t i = 0; i < texture_count; ++i)
    {
        const auto texture = create_texture();
        const auto view = m_gpu_texture_system->create_view(texture, make_view_desc());

        ASSERT_TRUE(texture.is_valid());
        ASSERT_TRUE(view.is_valid());

        textures.push_back(texture);
        views.push_back(view);
    }

    for (uint32_t cycle = 0; cycle < cycles; ++cycle)
    {
        // Request residency for every texture
        for (const auto texture : textures)
            ASSERT_TRUE(m_gpu_texture_system->make_resident(texture));

        // Record and complete uploads
        upload_pending_textures();

        EXPECT_EQ(m_gpu_texture_system->resident_texture_count(), texture_count);

        // Process deferred view creation
        ogfx::CommandBuffer create_views_cmd(*m_command_pool);
        create_views_cmd.begin();
        const auto create_batch = m_gpu_texture_system->synchronize(create_views_cmd);
        create_views_cmd.end();

        EXPECT_FALSE(create_batch.is_valid());

        for (const auto view : views)
        {
            EXPECT_TRUE(m_gpu_texture_system->is_view_valid(view));
            EXPECT_NE(m_gpu_texture_system->get_view(view), nullptr);
        }

        // Release every texture from GPU residency
        for (const auto texture : textures)
            ASSERT_TRUE(m_gpu_texture_system->make_non_resident(texture));

        ogfx::CommandBuffer release_cmd(*m_command_pool);
        release_cmd.begin();
        const auto release_batch = m_gpu_texture_system->synchronize(release_cmd);
        release_cmd.end();

        EXPECT_FALSE(release_batch.is_valid());
        EXPECT_EQ(m_gpu_texture_system->resident_texture_count(), 0);

        for (const auto view : views)
        {
            EXPECT_TRUE(m_gpu_texture_system->is_view_valid(view));
            EXPECT_EQ(m_gpu_texture_system->get_view(view), nullptr);
        }
    }

    // Release all view requests
    for (const auto view : views)
        EXPECT_TRUE(m_gpu_texture_system->release_view(view));

    EXPECT_EQ(m_gpu_texture_system->texture_view_count(), 0);
}

}
