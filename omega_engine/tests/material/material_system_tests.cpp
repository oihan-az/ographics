#include <gtest/gtest.h>

#include <omega/material/material_system.hpp>

namespace
{

struct TestMaterial
{
    float roughness = 1.0f;
    float metallic = 0.0f;
};

struct OtherMaterial
{
    int value = 0;
};

TEST(MaterialSystemTests, CreatesMaterial)
{
    omega::MaterialSystem system;

    const auto handle = system.create(TestMaterial{.roughness = 0.4f, .metallic = 0.8f});

    EXPECT_TRUE(handle.is_valid());
    EXPECT_TRUE(system.has(handle));

    const TestMaterial* material = system.get(handle);

    ASSERT_NE(material, nullptr);
    EXPECT_FLOAT_EQ(material->roughness, 0.4f);
    EXPECT_FLOAT_EQ(material->metallic, 0.8f);
}

TEST(MaterialSystemTests, CreatesMultipleMaterials)
{
    omega::MaterialSystem system;

    const auto first = system.create(TestMaterial{.roughness = 0.2f, .metallic = 0.1f});

    const auto second = system.create(TestMaterial{.roughness = 0.9f, .metallic = 0.7f});

    EXPECT_NE(first, second);

    const TestMaterial* first_material = system.get(first);
    const TestMaterial* second_material = system.get(second);

    ASSERT_NE(first_material, nullptr);
    ASSERT_NE(second_material, nullptr);

    EXPECT_FLOAT_EQ(first_material->roughness, 0.2f);
    EXPECT_FLOAT_EQ(second_material->roughness, 0.9f);
}

TEST(MaterialSystemTests, PreservesMaterialData)
{
    omega::MaterialSystem system;

    const TestMaterial original{.roughness = 0.35f, .metallic = 0.65f};

    const auto handle = system.create(original);

    const TestMaterial* material = system.get(handle);

    ASSERT_NE(material, nullptr);

    EXPECT_FLOAT_EQ(material->roughness, original.roughness);
    EXPECT_FLOAT_EQ(material->metallic, original.metallic);
}

TEST(MaterialSystemTests, AllowsMaterialModification)
{
    omega::MaterialSystem system;

    const auto handle = system.create(TestMaterial{});

    TestMaterial* material = system.get(handle);

    ASSERT_NE(material, nullptr);

    material->roughness = 0.25f;
    material->metallic = 1.0f;

    const TestMaterial* updated = system.get(handle);

    ASSERT_NE(updated, nullptr);

    EXPECT_FLOAT_EQ(updated->roughness, 0.25f);
    EXPECT_FLOAT_EQ(updated->metallic, 1.0f);
}

TEST(MaterialSystemTests, SupportsMultipleMaterialTypes)
{
    omega::MaterialSystem system;

    const auto first = system.create(TestMaterial{.roughness = 0.5f, .metallic = 0.2f});

    const auto second = system.create(OtherMaterial{.value = 42});

    EXPECT_TRUE(system.has(first));
    EXPECT_TRUE(system.has(second));

    const TestMaterial* first_material = system.get(first);
    const OtherMaterial* second_material = system.get(second);

    ASSERT_NE(first_material, nullptr);
    ASSERT_NE(second_material, nullptr);

    EXPECT_FLOAT_EQ(first_material->roughness, 0.5f);
    EXPECT_EQ(second_material->value, 42);
}

TEST(MaterialSystemTests, MaterialTypesHaveIndependentStorage)
{
    omega::MaterialSystem system;

    const auto first = system.create(TestMaterial{});
    const auto second = system.create(OtherMaterial{});

    // Each material type maintains its own pool.
    EXPECT_EQ(first.index, 0);
    EXPECT_EQ(second.index, 0);

    EXPECT_EQ(first.generation, 0);
    EXPECT_EQ(second.generation, 0);

    EXPECT_TRUE(system.has(first));
    EXPECT_TRUE(system.has(second));

    ASSERT_TRUE(system.destroy(first));

    EXPECT_FALSE(system.has(first));
    EXPECT_TRUE(system.has(second));
}

TEST(MaterialSystemTests, DestroysMaterial)
{
    omega::MaterialSystem system;

    const auto handle = system.create(TestMaterial{});

    ASSERT_TRUE(system.has(handle));

    EXPECT_TRUE(system.destroy(handle));

    EXPECT_FALSE(system.has(handle));
    EXPECT_EQ(system.get(handle), nullptr);
}

TEST(MaterialSystemTests, ReusesMaterialSlotWithNewGeneration)
{
    omega::MaterialSystem system;

    const auto first = system.create(TestMaterial{.roughness = 0.2f});

    ASSERT_TRUE(system.destroy(first));

    const auto second = system.create(TestMaterial{.roughness = 0.8f});

    EXPECT_EQ(first.index, second.index);
    EXPECT_NE(first.generation, second.generation);

    const TestMaterial* material = system.get(second);

    ASSERT_NE(material, nullptr);
    EXPECT_FLOAT_EQ(material->roughness, 0.8f);
}

TEST(MaterialSystemTests, RejectsDoubleDestroy)
{
    omega::MaterialSystem system;

    const auto handle = system.create(TestMaterial{});

    EXPECT_TRUE(system.destroy(handle));
    EXPECT_FALSE(system.destroy(handle));
    EXPECT_FALSE(system.has(handle));
}

TEST(MaterialSystemTests, RejectsStaleHandle)
{
    omega::MaterialSystem system;

    const auto first = system.create(TestMaterial{});

    ASSERT_TRUE(system.destroy(first));

    const auto second = system.create(TestMaterial{});

    EXPECT_EQ(first.index, second.index);
    EXPECT_NE(first.generation, second.generation);

    EXPECT_EQ(system.get(first), nullptr);
    EXPECT_FALSE(system.has(first));
    EXPECT_FALSE(system.destroy(first));

    EXPECT_NE(system.get(second), nullptr);
    EXPECT_TRUE(system.has(second));
}

TEST(MaterialSystemTests, RejectsInvalidHandle)
{
    omega::MaterialSystem system;

    omega::MaterialHandle<TestMaterial> invalid;

    EXPECT_FALSE(invalid.is_valid());

    EXPECT_EQ(system.get(invalid), nullptr);
    EXPECT_FALSE(system.has(invalid));
    EXPECT_FALSE(system.destroy(invalid));
}

TEST(MaterialSystemTests, RejectsOutOfRangeHandle)
{
    omega::MaterialSystem system;

    const omega::MaterialHandle<TestMaterial> invalid{100, 0};

    EXPECT_EQ(system.get(invalid), nullptr);
    EXPECT_FALSE(system.has(invalid));
    EXPECT_FALSE(system.destroy(invalid));
}

TEST(MaterialSystemTests, DoesNotCreatePoolWhenLookingUpMaterial)
{
    omega::MaterialSystem system;

    omega::MaterialHandle<TestMaterial> handle{0, 0};

    // No material of this type has been created
    EXPECT_EQ(system.get(handle), nullptr);
    EXPECT_FALSE(system.has(handle));
    EXPECT_FALSE(system.destroy(handle));

    // Creating a material should still use the first slot
    const auto created = system.create(TestMaterial{});

    EXPECT_EQ(created.index, 0);
    EXPECT_EQ(created.generation, 0);
}

TEST(MaterialSystemTests, SupportsConstAccess)
{
    omega::MaterialSystem system;

    const auto handle = system.create(TestMaterial{.roughness = 0.6f});

    const omega::MaterialSystem& const_system = system;

    const TestMaterial* material = const_system.get(handle);

    ASSERT_NE(material, nullptr);
    EXPECT_FLOAT_EQ(material->roughness, 0.6f);

    EXPECT_TRUE(const_system.has(handle));
}

TEST(MaterialSystemTests, ReusesSlotsInLastInFirstOutOrder)
{
    omega::MaterialSystem system;

    const auto first = system.create(TestMaterial{});
    const auto second = system.create(TestMaterial{});
    const auto third = system.create(TestMaterial{});

    ASSERT_TRUE(system.destroy(first));
    ASSERT_TRUE(system.destroy(third));

    const auto reused_first = system.create(TestMaterial{});
    const auto reused_second = system.create(TestMaterial{});

    EXPECT_EQ(reused_first.index, third.index);
    EXPECT_EQ(reused_second.index, first.index);

    EXPECT_NE(reused_first.generation, third.generation);
    EXPECT_NE(reused_second.generation, first.generation);

    EXPECT_TRUE(system.has(second));
}

} // namespace