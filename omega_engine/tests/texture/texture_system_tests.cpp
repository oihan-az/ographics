#include <gtest/gtest.h>

#include <omega/core/event/event_system.hpp>
#include <omega/texture/texture_system.hpp>

#include <cstring>
#include <vector>

namespace
{

omega::TextureAsset make_texture_asset(uint32_t width = 4, uint32_t height = 4,
                                       ogfx::Format format = ogfx::Format::RGBA8_UNORM)
{
    omega::TextureAsset asset;

    asset.type = ogfx::ImageType::Image2D;
    asset.width = width;
    asset.height = height;
    asset.depth = 1;
    asset.mip_levels = 1;
    asset.array_layers = 1;
    asset.format = format;

    ogfx::FormatInfo format_info{};
    if (!ogfx::get_format_info(format, format_info))
        return asset;

    omega::TextureSubresource subresource;
    subresource.mip_level = 0;
    subresource.array_layer = 0;

    const size_t data_size = static_cast<size_t>(width) * height * format_info.bytes_per_pixel;

    subresource.data.resize(data_size, std::byte{0xFF});

    asset.subresources.push_back(std::move(subresource));

    return asset;
}

TEST(TextureSystemTests, CreatesTexture)
{
    omega::EventSystem events;
    omega::TextureSystem system(events);

    omega::TextureHandle handle;

    ASSERT_TRUE(system.create_texture(make_texture_asset(), handle));

    EXPECT_TRUE(handle.is_valid());
    EXPECT_EQ(system.texture_count(), 1);

    const omega::TextureData* data = system.get(handle);

    ASSERT_NE(data, nullptr);
    EXPECT_EQ(data->handle, handle);
    EXPECT_EQ(data->asset.width, 4);
    EXPECT_EQ(data->asset.height, 4);
    EXPECT_EQ(data->asset.format, ogfx::Format::RGBA8_UNORM);
    EXPECT_EQ(data->asset.subresources.size(), 1);
}

TEST(TextureSystemTests, RejectsInvalidAsset)
{
    omega::EventSystem events;
    omega::TextureSystem system(events);

    omega::TextureAsset asset;
    omega::TextureHandle handle;

    EXPECT_FALSE(system.create_texture(asset, handle));
    EXPECT_EQ(system.texture_count(), 0);
}

TEST(TextureSystemTests, CreatesMultipleTextures)
{
    omega::EventSystem events;
    omega::TextureSystem system(events);

    omega::TextureHandle first;
    omega::TextureHandle second;

    ASSERT_TRUE(system.create_texture(make_texture_asset(4, 4), first));
    ASSERT_TRUE(system.create_texture(make_texture_asset(8, 8), second));

    const omega::TextureData* first_data = system.get(first);
    const omega::TextureData* second_data = system.get(second);

    ASSERT_NE(first_data, nullptr);
    ASSERT_NE(second_data, nullptr);

    EXPECT_EQ(first_data->asset.width, 4);
    EXPECT_EQ(second_data->asset.width, 8);
    EXPECT_EQ(system.texture_count(), 2);
}

TEST(TextureSystemTests, PreservesTextureData)
{
    omega::EventSystem events;
    omega::TextureSystem system(events);

    omega::TextureAsset asset = make_texture_asset();

    const std::vector<std::byte> expected_data = asset.subresources[0].data;

    omega::TextureHandle handle;

    ASSERT_TRUE(system.create_texture(asset, handle));

    const omega::TextureData* data = system.get(handle);

    ASSERT_NE(data, nullptr);
    ASSERT_EQ(data->asset.subresources.size(), 1);

    EXPECT_EQ(data->asset.subresources[0].data, expected_data);
}

TEST(TextureSystemTests, ReusesTextureSlotWithNewGeneration)
{
    omega::EventSystem events;
    omega::TextureSystem system(events);

    omega::TextureHandle first;
    ASSERT_TRUE(system.create_texture(make_texture_asset(), first));

    ASSERT_TRUE(system.release_texture(first));
    EXPECT_EQ(system.get(first), nullptr);

    omega::TextureHandle second;
    ASSERT_TRUE(system.create_texture(make_texture_asset(), second));

    EXPECT_EQ(first.index, second.index);
    EXPECT_NE(first.generation, second.generation);
    EXPECT_NE(system.get(second), nullptr);
}

TEST(TextureSystemTests, RejectsDoubleRelease)
{
    omega::EventSystem events;
    omega::TextureSystem system(events);

    omega::TextureHandle handle;
    ASSERT_TRUE(system.create_texture(make_texture_asset(), handle));

    EXPECT_TRUE(system.release_texture(handle));
    EXPECT_FALSE(system.release_texture(handle));
    EXPECT_EQ(system.texture_count(), 0);
}

TEST(TextureSystemTests, RejectsStaleHandle)
{
    omega::EventSystem events;
    omega::TextureSystem system(events);

    omega::TextureHandle first;
    ASSERT_TRUE(system.create_texture(make_texture_asset(), first));

    ASSERT_TRUE(system.release_texture(first));

    omega::TextureHandle second;
    ASSERT_TRUE(system.create_texture(make_texture_asset(), second));

    EXPECT_EQ(system.get(first), nullptr);
    EXPECT_NE(system.get(second), nullptr);
}

TEST(TextureSystemTests, SupportsMultipleMipLevels)
{
    omega::EventSystem events;
    omega::TextureSystem system(events);

    omega::TextureAsset asset;
    asset.type = ogfx::ImageType::Image2D;
    asset.width = 4;
    asset.height = 4;
    asset.depth = 1;
    asset.mip_levels = 3;
    asset.array_layers = 1;
    asset.format = ogfx::Format::RGBA8_UNORM;

    for (uint32_t mip = 0; mip < asset.mip_levels; ++mip)
    {
        const uint32_t width = std::max(1u, asset.width >> mip);
        const uint32_t height = std::max(1u, asset.height >> mip);

        omega::TextureSubresource subresource;
        subresource.mip_level = mip;
        subresource.array_layer = 0;
        subresource.data.resize(width * height * 4);

        asset.subresources.push_back(std::move(subresource));
    }

    omega::TextureHandle handle;

    ASSERT_TRUE(system.create_texture(asset, handle));

    const omega::TextureData* data = system.get(handle);

    ASSERT_NE(data, nullptr);
    EXPECT_EQ(data->asset.mip_levels, 3);
    EXPECT_EQ(data->asset.subresources.size(), 3);
}

TEST(TextureSystemTests, RejectsInvalidHandle)
{
    omega::EventSystem events;
    omega::TextureSystem system(events);

    EXPECT_EQ(system.get({}), nullptr);
    EXPECT_FALSE(system.release_texture({}));
}

} // namespace