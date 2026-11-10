
#include <gtest/gtest.h>

#include <omega/mesh/gpu_mesh_system.hpp>
#include <omega/mesh/mesh_asset.hpp>
#include <omega/mesh/mesh_event.hpp>
#include <omega/mesh/mesh_system.hpp>

#include <omega/core/event/event_system.hpp>

#include <ogfx/device.hpp>
#include <ogfx/instance.hpp>
#include <ogfx/physical_device.hpp>

#include <orendergraph/resource_pool_registry.hpp>

#include <cstddef>
#include <cstdint>
#include <vector>

namespace
{

class GpuMeshSystemTests : public ::testing::Test
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

    inline static std::unique_ptr<ogfx::Instance> s_instance;
    inline static std::unique_ptr<ogfx::PhysicalDevice> s_physical_device;
    inline static std::unique_ptr<ogfx::Device> s_device;

  protected:
    void SetUp() override
    {
        m_mesh_system = std::make_unique<omega::MeshSystem>(m_events, omega::MeshSystemDesc{});

        m_gpu_mesh_system = std::make_unique<omega::GpuMeshSystem>(*s_physical_device, *s_device, m_resource_registry,
                                                                   *m_mesh_system, m_events, omega::MeshSystemDesc{});
    }

    void TearDown() override
    {
        m_gpu_mesh_system.reset();
        m_mesh_system.reset();
    }

    omega::MeshAsset create_triangle_asset()
    {
        struct Vertex
        {
            float position[3];
        };

        const Vertex vertices[] = {{{0.0f, 0.0f, 0.0f}}, {{1.0f, 0.0f, 0.0f}}, {{0.0f, 1.0f, 0.0f}}};

        omega::MeshAsset asset;

        asset.vertex_input.bindings.push_back({0, sizeof(Vertex), ogfx::VertexInputRate::Vertex});

        asset.vertex_input.attributes.push_back({0, 0, ogfx::Format::RGB32_FLOAT, 0});

        asset.vertex_count = 3;

        const auto* bytes = reinterpret_cast<const std::byte*>(vertices);

        asset.vertex_data.emplace_back(bytes, bytes + sizeof(vertices));

        asset.indices = {0, 1, 2};

        return asset;
    }
    
    omega::MeshAsset create_position_normal_asset()
    {
        struct Vertex
        {
            float position[3];
            float normal[3];
        };

        const Vertex vertices[] = {{{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
                                   {{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
                                   {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}};

        omega::MeshAsset asset;

        asset.vertex_input.bindings.push_back({0, sizeof(Vertex), ogfx::VertexInputRate::Vertex});

        asset.vertex_input.attributes.push_back({0, 0, ogfx::Format::RGB32_FLOAT, 0});

        asset.vertex_input.attributes.push_back(
            {1, 0, ogfx::Format::RGB32_FLOAT, static_cast<uint32_t>(offsetof(Vertex, normal))});

        asset.vertex_count = 3;

        const auto* bytes = reinterpret_cast<const std::byte*>(vertices);

        asset.vertex_data.emplace_back(bytes, bytes + sizeof(vertices));

        asset.indices = {0, 1, 2};

        return asset;
    }

    omega::MeshAsset create_non_indexed_asset()
    {
        auto asset = create_triangle_asset();
        asset.indices.clear();
        return asset;
    }

    omega::MeshHandle create_mesh(const omega::MeshAsset& asset)
    {
        omega::MeshHandle handle;

        if (!m_mesh_system->create_mesh(asset, handle))
            return {};

        return handle;
    }

    void synchronize()
    {
        m_gpu_mesh_system->synchronize();
    }

    omega::EventSystem m_events;
    orendergraph::ResourcePoolRegistry m_resource_registry;

    std::unique_ptr<omega::MeshSystem> m_mesh_system;
    std::unique_ptr<omega::GpuMeshSystem> m_gpu_mesh_system;
};


TEST_F(GpuMeshSystemTests, MeshIsNotResidentByDefault)
{
    const auto asset = create_triangle_asset();
    const auto handle = create_mesh(asset);

    ASSERT_TRUE(handle.is_valid());

    synchronize();

    EXPECT_FALSE(m_gpu_mesh_system->is_resident(handle));
    EXPECT_EQ(m_gpu_mesh_system->resident_mesh_count(), 0);
}

TEST_F(GpuMeshSystemTests, MakeResidentUploadsMesh)
{
    const auto asset = create_triangle_asset();
    const auto handle = create_mesh(asset);

    ASSERT_TRUE(handle.is_valid());

    ASSERT_TRUE(m_gpu_mesh_system->make_resident(handle));
    synchronize();

    EXPECT_TRUE(m_gpu_mesh_system->is_resident(handle));
    EXPECT_FALSE(m_gpu_mesh_system->is_pending(handle));
    EXPECT_EQ(m_gpu_mesh_system->resident_mesh_count(), 1);

    const omega::MeshData* gpu_mesh = m_gpu_mesh_system->get(handle);

    ASSERT_NE(gpu_mesh, nullptr);
    EXPECT_EQ(gpu_mesh->vertex_count, 3);
    EXPECT_EQ(gpu_mesh->index_count, 3);
    EXPECT_TRUE(gpu_mesh->layout.is_valid());
}

TEST_F(GpuMeshSystemTests, MakeNonResidentReleasesMesh)
{
    const auto asset = create_triangle_asset();
    const auto handle = create_mesh(asset);

    ASSERT_TRUE(handle.is_valid());

    ASSERT_TRUE(m_gpu_mesh_system->make_resident(handle));
    synchronize();

    ASSERT_TRUE(m_gpu_mesh_system->is_resident(handle));

    ASSERT_TRUE(m_gpu_mesh_system->make_non_resident(handle));
    synchronize();

    EXPECT_FALSE(m_gpu_mesh_system->is_resident(handle));
    EXPECT_EQ(m_gpu_mesh_system->resident_mesh_count(), 0);
    EXPECT_EQ(m_gpu_mesh_system->get(handle), nullptr);
}


TEST_F(GpuMeshSystemTests, MeshesWithSameLayoutShareGpuLayoutGroup)
{
    const auto asset = create_triangle_asset();

    const auto first = create_mesh(asset);
    const auto second = create_mesh(asset);

    ASSERT_TRUE(first.is_valid());
    ASSERT_TRUE(second.is_valid());

    ASSERT_TRUE(m_gpu_mesh_system->make_resident(first));
    ASSERT_TRUE(m_gpu_mesh_system->make_resident(second));

    synchronize();

    ASSERT_TRUE(m_gpu_mesh_system->is_resident(first));
    ASSERT_TRUE(m_gpu_mesh_system->is_resident(second));

    EXPECT_EQ(m_gpu_mesh_system->layout_group_count(), 1);
    EXPECT_EQ(m_gpu_mesh_system->resident_mesh_count(), 2);

    const auto* first_data = m_gpu_mesh_system->get(first);
    const auto* second_data = m_gpu_mesh_system->get(second);

    ASSERT_NE(first_data, nullptr);
    ASSERT_NE(second_data, nullptr);

    EXPECT_EQ(first_data->layout, second_data->layout);
}

TEST_F(GpuMeshSystemTests, DifferentLayoutsCreateDifferentGpuLayoutGroups)
{
    const auto first = create_mesh(create_triangle_asset());
    const auto second = create_mesh(create_position_normal_asset());

    ASSERT_TRUE(first.is_valid());
    ASSERT_TRUE(second.is_valid());

    ASSERT_TRUE(m_gpu_mesh_system->make_resident(first));
    ASSERT_TRUE(m_gpu_mesh_system->make_resident(second));

    synchronize();

    ASSERT_TRUE(m_gpu_mesh_system->is_resident(first));
    ASSERT_TRUE(m_gpu_mesh_system->is_resident(second));

    EXPECT_EQ(m_gpu_mesh_system->layout_group_count(), 2);

    EXPECT_NE(m_gpu_mesh_system->get(first)->layout, m_gpu_mesh_system->get(second)->layout);
}

TEST_F(GpuMeshSystemTests, GpuLayoutRegistryIsIndependentFromCpuRegistry)
{
    // Register a layout in the CPU registry without making it resident
    const auto first = create_mesh(create_triangle_asset());
    const auto second = create_mesh(create_position_normal_asset());

    ASSERT_TRUE(first.is_valid());
    ASSERT_TRUE(second.is_valid());

    // Register the second CPU layout first by creating its mesh.
    // Only the first mesh is made GPU resident
    ASSERT_TRUE(m_gpu_mesh_system->make_resident(second));
    synchronize();

    ASSERT_TRUE(m_gpu_mesh_system->is_resident(second));
    ASSERT_EQ(m_gpu_mesh_system->layout_group_count(), 1);

    const auto* gpu_data = m_gpu_mesh_system->get(second);
    ASSERT_NE(gpu_data, nullptr);

    // The GPU registry assigns its own first ID, independent of the
    // CPU registry's ID for this layout
    EXPECT_EQ(gpu_data->layout.value, 0);
}


TEST_F(GpuMeshSystemTests, UploadsIndexedMesh)
{
    const auto handle = create_mesh(create_triangle_asset());

    ASSERT_TRUE(handle.is_valid());
    ASSERT_TRUE(m_gpu_mesh_system->make_resident(handle));

    synchronize();

    const auto* gpu_mesh = m_gpu_mesh_system->get(handle);

    ASSERT_NE(gpu_mesh, nullptr);
    EXPECT_TRUE(gpu_mesh->is_indexed());
    EXPECT_EQ(gpu_mesh->vertex_count, 3);
    EXPECT_EQ(gpu_mesh->index_count, 3);
}

TEST_F(GpuMeshSystemTests, UploadsNonIndexedMesh)
{
    const auto handle = create_mesh(create_non_indexed_asset());

    ASSERT_TRUE(handle.is_valid());
    ASSERT_TRUE(m_gpu_mesh_system->make_resident(handle));

    synchronize();

    const auto* gpu_mesh = m_gpu_mesh_system->get(handle);

    ASSERT_NE(gpu_mesh, nullptr);
    EXPECT_FALSE(gpu_mesh->is_indexed());
    EXPECT_EQ(gpu_mesh->vertex_count, 3);
    EXPECT_EQ(gpu_mesh->index_count, 0);
}

TEST_F(GpuMeshSystemTests, RepeatedMakeResidentDoesNotDuplicateAllocation)
{
    const auto handle = create_mesh(create_triangle_asset());

    ASSERT_TRUE(handle.is_valid());

    ASSERT_TRUE(m_gpu_mesh_system->make_resident(handle));
    synchronize();

    ASSERT_TRUE(m_gpu_mesh_system->is_resident(handle));

    const auto first_allocation = m_gpu_mesh_system->get(handle)->allocation;

    ASSERT_TRUE(m_gpu_mesh_system->make_resident(handle));
    synchronize();

    ASSERT_TRUE(m_gpu_mesh_system->is_resident(handle));
    EXPECT_EQ(m_gpu_mesh_system->resident_mesh_count(), 1);
    EXPECT_EQ(m_gpu_mesh_system->get(handle)->allocation, first_allocation);
}

TEST_F(GpuMeshSystemTests, RepeatedMakeNonResidentIsRejected)
{
    const auto handle = create_mesh(create_triangle_asset());

    ASSERT_TRUE(handle.is_valid());

    EXPECT_FALSE(m_gpu_mesh_system->make_non_resident(handle));
}

TEST_F(GpuMeshSystemTests, MakeResidentAndNonResidentBeforeSynchronization)
{
    const auto handle = create_mesh(create_triangle_asset());

    ASSERT_TRUE(handle.is_valid());

    ASSERT_TRUE(m_gpu_mesh_system->make_resident(handle));
    ASSERT_TRUE(m_gpu_mesh_system->make_non_resident(handle));

    synchronize();

    EXPECT_FALSE(m_gpu_mesh_system->is_resident(handle));
    EXPECT_EQ(m_gpu_mesh_system->resident_mesh_count(), 0);
}

TEST_F(GpuMeshSystemTests, CanMakeMeshResidentAgainAfterRelease)
{
    const auto handle = create_mesh(create_triangle_asset());

    ASSERT_TRUE(handle.is_valid());

    ASSERT_TRUE(m_gpu_mesh_system->make_resident(handle));
    synchronize();

    ASSERT_TRUE(m_gpu_mesh_system->make_non_resident(handle));
    synchronize();

    ASSERT_FALSE(m_gpu_mesh_system->is_resident(handle));

    ASSERT_TRUE(m_gpu_mesh_system->make_resident(handle));
    synchronize();

    EXPECT_TRUE(m_gpu_mesh_system->is_resident(handle));
    EXPECT_EQ(m_gpu_mesh_system->resident_mesh_count(), 1);
}

TEST_F(GpuMeshSystemTests, StressTestManyMeshes)
{
    constexpr uint32_t mesh_count = 1000;

    std::vector<omega::MeshHandle> handles;
    handles.reserve(mesh_count);

    // Create many meshes
    for (uint32_t i = 0; i < mesh_count; ++i)
    {
        omega::MeshHandle handle;

        ASSERT_TRUE(m_mesh_system->create_mesh(create_triangle_asset(), handle));

        handles.push_back(handle);
    }

    EXPECT_EQ(m_mesh_system->mesh_count(), mesh_count);
    EXPECT_EQ(m_gpu_mesh_system->resident_mesh_count(), 0);

    // Request residency for every mesh
    for (const auto handle : handles)
    {
        ASSERT_TRUE(m_gpu_mesh_system->make_resident(handle));
    }

    synchronize();

    EXPECT_EQ(m_gpu_mesh_system->resident_mesh_count(), mesh_count);

    // Repeated residency requests must not duplicate allocations
    for (const auto handle : handles)
    {
        ASSERT_TRUE(m_gpu_mesh_system->make_resident(handle));
    }

    synchronize();

    EXPECT_EQ(m_gpu_mesh_system->resident_mesh_count(), mesh_count);

    // Release half of the GPU meshes
    for (uint32_t i = 0; i < mesh_count / 2; ++i)
    {
        ASSERT_TRUE(m_gpu_mesh_system->make_non_resident(handles[i]));
    }

    synchronize();

    EXPECT_EQ(m_gpu_mesh_system->resident_mesh_count(), mesh_count - mesh_count / 2);

    // Make the released GPU meshes resident again
    for (uint32_t i = 0; i < mesh_count / 2; ++i)
    {
        ASSERT_TRUE(m_gpu_mesh_system->make_resident(handles[i]));
    }

    synchronize();

    EXPECT_EQ(m_gpu_mesh_system->resident_mesh_count(), mesh_count);

    // Release all GPU allocations
    for (const auto handle : handles)
    {
        ASSERT_TRUE(m_gpu_mesh_system->make_non_resident(handle));
    }

    synchronize();

    EXPECT_EQ(m_gpu_mesh_system->resident_mesh_count(), 0);

    // Release CPU meshes independently
    for (const auto handle : handles)
    {
        ASSERT_TRUE(m_mesh_system->release_mesh(handle));
    }

    EXPECT_EQ(m_mesh_system->mesh_count(), 0);
}

} // namespace
