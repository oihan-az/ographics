#include <gtest/gtest.h>

#include <omega/mesh/vertex_layout_registry.hpp>

namespace
{

omega::VertexLayout make_layout()
{
    omega::VertexLayout layout;

    layout.desc.bindings.push_back({0, 12, ogfx::VertexInputRate::Vertex});

    layout.desc.attributes.push_back({0, 0, ogfx::Format::RGB32_FLOAT, 0});

    return layout;
}

} // namespace

TEST(VertexLayoutRegistryTests, RegistersNewLayout)
{
    omega::VertexLayoutRegistry registry;

    const auto id = registry.register_layout(make_layout());

    EXPECT_TRUE(id.is_valid());
    EXPECT_EQ(id.value, 0);
    EXPECT_EQ(registry.size(), 1);
}

TEST(VertexLayoutRegistryTests, ReusesExistingLayout)
{
    omega::VertexLayoutRegistry registry;

    const auto first = registry.register_layout(make_layout());
    const auto second = registry.register_layout(make_layout());

    EXPECT_EQ(first, second);
    EXPECT_EQ(registry.size(), 1);
}

TEST(VertexLayoutRegistryTests, RegistersDifferentLayoutsSeparately)
{
    omega::VertexLayoutRegistry registry;

    auto first_desc = make_layout();
    auto second_desc = make_layout();

    second_desc.desc.bindings[0].stride = 16;

    const auto first = registry.register_layout(first_desc);
    const auto second = registry.register_layout(second_desc);

    EXPECT_NE(first, second);
    EXPECT_EQ(registry.size(), 2);
}

TEST(VertexLayoutRegistryTests, RetrievesRegisteredLayout)
{
    omega::VertexLayoutRegistry registry;

    const auto desc = make_layout();
    const auto id = registry.register_layout(desc);

    const auto* result = registry.get(id);

    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->desc.bindings[0].stride, 12);
    EXPECT_EQ(result->desc.attributes[0].format, ogfx::Format::RGB32_FLOAT);
}

TEST(VertexLayoutRegistryTests, RejectsInvalidId)
{
    omega::VertexLayoutRegistry registry;

    EXPECT_FALSE(registry.contains({}));
    EXPECT_EQ(registry.get({}), nullptr);
}

TEST(VertexLayoutRegistryTests, RejectsOutOfRangeId)
{
    omega::VertexLayoutRegistry registry;

    EXPECT_FALSE(registry.contains({5}));
    EXPECT_EQ(registry.get({5}), nullptr);
}

TEST(VertexLayoutRegistryTests, ClearRemovesAllLayouts)
{
    omega::VertexLayoutRegistry registry;

    const auto id = registry.register_layout(make_layout());

    registry.clear();

    EXPECT_EQ(registry.size(), 0);
    EXPECT_FALSE(registry.contains(id));
}