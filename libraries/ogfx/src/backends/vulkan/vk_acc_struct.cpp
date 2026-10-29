#include <debug.hpp>
#include <ogfx/enum_string.hpp>

#include "vk_acc_struct.hpp"
#include "vk_acc_struct_type.hpp"
#include "vk_acc_struct_geometry.hpp"
#include "vk_acc_struct_build_preference.hpp"
#include "vk_device_functions.hpp"
#include "vk_buffer.hpp"
#include "vk_device.hpp"
#include "vk_physical_device.hpp"

#include <stdexcept>

namespace ogfx
{

AccelerationStructure::AccelerationStructure(const PhysicalDevice& physical_device, const Device& device,
                                             const AccelerationStructureDesc& desc)
    : m_impl(std::make_unique<Impl>())
{
    m_impl->m_device = device.m_impl->m_device;
    m_impl->m_build_flags = to_vk_acceleration_structure_build_preference(desc.build_preference);
}

AccelerationStructure::~AccelerationStructure()
{
    destroy();
}

AccelerationStructure::AccelerationStructure(AccelerationStructure&&) noexcept = default;

AccelerationStructure& AccelerationStructure::operator=(AccelerationStructure&&) noexcept = default;

uint64_t AccelerationStructure::device_address() const
{
    VkAccelerationStructureDeviceAddressInfoKHR address_info{};
    address_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
    address_info.pNext = nullptr;
    address_info.accelerationStructure = m_impl->m_acceleration_structure;

    VkDeviceAddress address = vkGetAccelerationStructureDeviceAddressKHR(m_impl->m_device, &address_info);
    return static_cast<uint64_t>(address);
}

void AccelerationStructure::create(const PhysicalDevice& physical_device, const Device& device)
{
    if (m_impl->m_geometries.empty() || m_impl->m_geometries.size() != m_impl->m_primitive_counts.size())
    {
        OGFX_LOG_ERROR("Invalid input for acceleration structure build size query");
        return;
    }

    VkAccelerationStructureTypeKHR type = to_vk_acceleration_structure_type(this->type());

    VkAccelerationStructureBuildSizesInfoKHR sizes{};
    sizes.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

    VkAccelerationStructureBuildGeometryInfoKHR build_info{};
    build_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    build_info.type = type;
    build_info.flags = m_impl->m_build_flags;
    build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    build_info.geometryCount = static_cast<uint32_t>(m_impl->m_geometries.size());
    build_info.pGeometries = m_impl->m_geometries.data();

    vkGetAccelerationStructureBuildSizesKHR(m_impl->m_device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                                            &build_info, m_impl->m_primitive_counts.data(), &sizes);

    if (sizes.accelerationStructureSize == 0)
    {
        OGFX_LOG_ERROR("Failed to query acceleration structure build sizes");
        return;
    }

    // Storage buffer
    BufferDesc buffer_desc{};
    buffer_desc.size = sizes.accelerationStructureSize;
    buffer_desc.usage = BufferUsage::AccelerationStructureStorage | BufferUsage::ShaderDeviceAddress;
    buffer_desc.memory_properties = MemoryProperty::DeviceLocal;

    m_impl->m_storage_buffer = std::make_unique<Buffer>(physical_device, device, buffer_desc);
    m_impl->m_storage_buffer->allocate_memory();
    m_impl->m_storage_buffer->bind_memory();

    // Scratch buffer
    BufferDesc scratch_desc{};
    scratch_desc.size = sizes.buildScratchSize;
    scratch_desc.usage = BufferUsage::Storage | BufferUsage::ShaderDeviceAddress;
    scratch_desc.memory_properties = MemoryProperty::DeviceLocal; // Corregido: antes asignabas a buffer_desc por error

    m_impl->m_scratch_buffer = std::make_unique<Buffer>(physical_device, device, scratch_desc);
    m_impl->m_scratch_buffer->allocate_memory();
    m_impl->m_scratch_buffer->bind_memory();

    // Acceleration structure handle
    VkAccelerationStructureCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    create_info.buffer = m_impl->m_storage_buffer->m_impl->m_buffer;
    create_info.size = sizes.accelerationStructureSize;
    create_info.type = type;

    const VkResult result = vkCreateAccelerationStructureKHR(m_impl->m_device, &create_info, nullptr, &m_impl->m_acceleration_structure);

    if (result != VK_SUCCESS)
    {
        OGFX_LOG_ERROR("Failed to create Vulkan acceleration structure");
        m_impl->m_acceleration_structure = VK_NULL_HANDLE;
        return;
    }

    OGFX_LOG("Acceleration structure created successfully");
}

void AccelerationStructure::destroy()
{
    if (m_impl && m_impl->m_acceleration_structure != VK_NULL_HANDLE)
    {
        vkDestroyAccelerationStructureKHR(m_impl->m_device, m_impl->m_acceleration_structure, nullptr);

        m_impl->m_acceleration_structure = VK_NULL_HANDLE;
    }

    m_impl->m_storage_buffer.reset();
    m_impl->m_scratch_buffer.reset();
}

BLAS::BLAS(const PhysicalDevice& physical_device, const Device& device, const BLASDesc& desc)
    : AccelerationStructure(physical_device, device, desc)
{
    OGFX_LOG("Creating BLAS:");
    OGFX_LOG("  Build preference: " + to_string_enum(desc.build_preference));
    OGFX_LOG("  Triangle geometries: " + std::to_string(desc.triangle_geometries.size()));

    if (desc.triangle_geometries.empty())
    {
        OGFX_LOG_ERROR("Cannot create bottom-level acceleration structure without triangle geometry");
        return;
    }

    m_impl->m_geometries.reserve(desc.triangle_geometries.size());
    m_impl->m_primitive_counts.reserve(desc.triangle_geometries.size());

    for (const auto& geometry : desc.triangle_geometries)
    {
        if (geometry.primitive_count == 0)
        {
            OGFX_LOG_ERROR("Triangle geometry primitive count must be greater than zero");
            return;
        }

        m_impl->m_geometries.push_back(to_vk_acceleration_structure_triangle_geometry(geometry));
        m_impl->m_primitive_counts.push_back(geometry.primitive_count);
    }

    create(physical_device, device);
}

TLAS::TLAS(const PhysicalDevice& physical_device, const Device& device, const TLASDesc& desc)
    : AccelerationStructure(physical_device, device, desc)
{
    OGFX_LOG("Creating TLAS:");
    OGFX_LOG("  Build preference: " + to_string_enum(desc.build_preference));
    OGFX_LOG("  Instance geometries: " + std::to_string(desc.instance_geometries.size()));

    if (desc.instance_geometries.empty())
    {
        OGFX_LOG_ERROR("Cannot create top-level acceleration structure without instance geometry");
        return;
    }

    m_impl->m_geometries.reserve(desc.instance_geometries.size());
    m_impl->m_primitive_counts.reserve(desc.instance_geometries.size());

    for (const auto& geometry : desc.instance_geometries)
    {
        if (geometry.instance_count == 0)
        {
            OGFX_LOG_ERROR("Instance geometry count must be greater than zero");
            return;
        }

        m_impl->m_geometries.push_back(to_vk_acceleration_structure_instance_geometry(geometry));
        m_impl->m_primitive_counts.push_back(geometry.instance_count);
    }

    create(physical_device, device);
}

uint64_t TLAS::instance_size()
{
    return sizeof(VkAccelerationStructureInstanceKHR);
}

void TLAS::write_instances_to_buffer(const std::vector<AccelerationStructureInstance>& instances,
                                                      Buffer& output_buffer)
{
    if (instances.empty())
    {
        OGFX_LOG_ERROR("Cannot write empty instances list to buffer");
        return;
    }

    // Convert the container
    std::vector<VkAccelerationStructureInstanceKHR> vk_instances;
    vk_instances.reserve(instances.size());

    for (const auto& inst : instances)
    {
        VkAccelerationStructureInstanceKHR vk_inst{};

        std::memcpy(&vk_inst.transform, &inst.transform, sizeof(float) * 12);
        vk_inst.instanceCustomIndex = inst.custom_index;
        vk_inst.mask = inst.mask;
        vk_inst.instanceShaderBindingTableRecordOffset = inst.shader_binding_table_record_offset;

        vk_inst.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;

        if (inst.acceleration_structure)
        {
            vk_inst.accelerationStructureReference = inst.acceleration_structure->device_address();
        }
        else
        {
            vk_inst.accelerationStructureReference = 0;
        }

        vk_instances.push_back(vk_inst);
    }

    BufferWriteInfo instance_write_info{};
    instance_write_info.data = vk_instances.data();
    instance_write_info.size = sizeof(VkAccelerationStructureInstanceKHR) * vk_instances.size();
    instance_write_info.offset = 0;

    output_buffer.write(instance_write_info);
}

} // namespace ogfx