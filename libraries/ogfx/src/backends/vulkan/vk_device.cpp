#include <debug.hpp>

#include "vk_device.hpp"
#include "vk_device_functions.hpp"
#include "vk_physical_device.hpp"
#include "vk_queue.hpp"

#include <stdexcept>
#include <vector>
#include <set>

namespace ogfx
{

void check_device_features_support(VkPhysicalDevice physical_device, ogfx::DeviceFeature features)
{
    const bool request_multi_draw_indirect = ogfx::has_flag(features, ogfx::DeviceFeature::MultiDrawIndirect);
    const bool request_draw_indirect_first_instance = ogfx::has_flag(features, ogfx::DeviceFeature::DrawIndirectFirstInstance);
    const bool request_sampler_anisotropy = ogfx::has_flag(features, ogfx::DeviceFeature::SamplerAnisotropy);
    const bool request_ray_tracing = ogfx::has_flag(features, ogfx::DeviceFeature::RayTracing);

    // Check core features only when at least one is requested.
    if (request_multi_draw_indirect || request_draw_indirect_first_instance || request_sampler_anisotropy)
    {
        VkPhysicalDeviceFeatures supported{};
        vkGetPhysicalDeviceFeatures(physical_device, &supported);

        if (request_multi_draw_indirect && !supported.multiDrawIndirect)
        {
            throw std::runtime_error("Multi draw indirect is not supported.");
        }

        if (request_draw_indirect_first_instance && !supported.drawIndirectFirstInstance)
        {
            throw std::runtime_error("Draw indirect first instance is not supported.");
        }

        if (request_sampler_anisotropy && !supported.samplerAnisotropy)
        {
            throw std::runtime_error("Sampler Anisotropy is not supported.");
        }
    }

    // Check ray-tracing features only when ray tracing is requested.
    if (request_ray_tracing)
    {
        VkPhysicalDeviceRayTracingPipelineFeaturesKHR ray_tracing{};
        ray_tracing.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;

        VkPhysicalDeviceAccelerationStructureFeaturesKHR acceleration_structure{};
        acceleration_structure.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;

        VkPhysicalDeviceBufferDeviceAddressFeatures buffer_device_address{};
        buffer_device_address.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;

        ray_tracing.pNext = &acceleration_structure;
        acceleration_structure.pNext = &buffer_device_address;

        VkPhysicalDeviceFeatures2 supported{};
        supported.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        supported.pNext = &ray_tracing;

        vkGetPhysicalDeviceFeatures2(physical_device, &supported);

        if (!ray_tracing.rayTracingPipeline || 
            !acceleration_structure.accelerationStructure ||
            !buffer_device_address.bufferDeviceAddress)
        {
            throw std::runtime_error("Ray tracing is not supported.");
        }
    }
}

Device::Device(const PhysicalDevice& physical_device, const DeviceDesc& desc, const Surface* surface)
    : m_impl(std::make_unique<Impl>())
{
    OGFX_LOG("Creating Vulkan logical device");

    ogfx::QueueFamilyIndices indices = physical_device.m_impl->FindQueueFamilies(surface);

    std::set<uint32_t> uniqueQueueFamilies;

    const bool request_graphics = ogfx::has_flag(desc.queues, ogfx::DeviceQueue::Graphics);
    const bool request_compute = ogfx::has_flag(desc.queues, ogfx::DeviceQueue::Compute);
    const bool request_transfer = ogfx::has_flag(desc.queues, ogfx::DeviceQueue::Transfer);
    const bool request_present = ogfx::has_flag(desc.queues, ogfx::DeviceQueue::Present);

    if (request_graphics)
    {
        if (!indices.graphics_family.has_value())
        {
            throw std::runtime_error("Graphics queue requested but not available.");
        }

        uniqueQueueFamilies.insert(indices.graphics_family.value());
    }

    if (request_compute)
    {
        if (!indices.compute_family.has_value())
        {
            throw std::runtime_error("Compute queue requested but not available.");
        }

        uniqueQueueFamilies.insert(indices.compute_family.value());
    }

    if (request_transfer)
    {
        if (!indices.transfer_family.has_value())
        {
            throw std::runtime_error("Transfer queue requested but not available.");
        }

        uniqueQueueFamilies.insert(indices.transfer_family.value());
    }

    if (request_present)
    {
        if (!indices.present_family.has_value())
        {
            throw std::runtime_error("Present queue requested but not available.");
        }

        uniqueQueueFamilies.insert(indices.present_family.value());
    }

    // Queues
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Requested features
    const bool request_multi_draw_indirect = ogfx::has_flag(desc.features, ogfx::DeviceFeature::MultiDrawIndirect);
    const bool request_draw_indirect_first_instance = ogfx::has_flag(desc.features, ogfx::DeviceFeature::DrawIndirectFirstInstance);
    const bool request_sampler_anisotropy = ogfx::has_flag(desc.features, ogfx::DeviceFeature::SamplerAnisotropy);
    const bool request_ray_tracing = ogfx::has_flag(desc.features, ogfx::DeviceFeature::RayTracing);

    OGFX_LOG("Requested device features:");

    if (request_multi_draw_indirect)
    {
        OGFX_LOG("  Multi draw indirect");
    }

    if (request_draw_indirect_first_instance)
    {
        OGFX_LOG("  Draw indirect first instance");
    }

    if (request_sampler_anisotropy)
    {
        OGFX_LOG("  Sampler anisotropy");
    }

    if (request_ray_tracing)
    {
        OGFX_LOG("  Ray tracing");
    }

    check_device_features_support(physical_device.m_impl->m_physical_device, desc.features);

    // Enable features
    VkPhysicalDeviceFeatures device_features{};
    device_features.multiDrawIndirect = request_multi_draw_indirect ? VK_TRUE : VK_FALSE;
    device_features.drawIndirectFirstInstance = request_draw_indirect_first_instance ? VK_TRUE : VK_FALSE;
    device_features.samplerAnisotropy = request_sampler_anisotropy ? VK_TRUE : VK_FALSE;

    // Ray tracing support
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR ray_tracing_features = {};
    ray_tracing_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
    ray_tracing_features.rayTracingPipeline = request_ray_tracing ? VK_TRUE : VK_FALSE;

    VkPhysicalDeviceAccelerationStructureFeaturesKHR acceleration_structure_features = {};
    acceleration_structure_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
    acceleration_structure_features.accelerationStructure = request_ray_tracing ? VK_TRUE : VK_FALSE;

    VkPhysicalDeviceBufferDeviceAddressFeatures buffer_device_address_features = {};
    buffer_device_address_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
    buffer_device_address_features.bufferDeviceAddress = request_ray_tracing ? VK_TRUE : VK_FALSE;

    ray_tracing_features.pNext = &acceleration_structure_features;
    acceleration_structure_features.pNext = &buffer_device_address_features;

    // Extensions
    std::vector<const char*> extensions;

    if (request_present)
    {
        extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

    if (request_ray_tracing)
    {
        extensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
        extensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
        extensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
        extensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
        extensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);

        extensions.push_back(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
        extensions.push_back(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);
    }

    // Device
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = &device_features;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount = 0;

    if (request_ray_tracing)
    {
        createInfo.pNext = &ray_tracing_features;
    }

    if (vkCreateDevice(physical_device.m_impl->m_physical_device, &createInfo, nullptr, &m_impl->m_device) != VK_SUCCESS)
    {
        OGFX_LOG_ERROR("Failed to create logical device");
        throw std::runtime_error("Failed to create logical device!");
    }

    OGFX_LOG("Vulkan logical device created");

    // Extension function pointers
    {
        if (request_ray_tracing)
        {
            vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(
                vkGetDeviceProcAddr(m_impl->m_device, "vkGetBufferDeviceAddressKHR"));

            vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(
                vkGetDeviceProcAddr(m_impl->m_device, "vkGetAccelerationStructureDeviceAddressKHR"));

            vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(
                vkGetDeviceProcAddr(m_impl->m_device, "vkCreateAccelerationStructureKHR"));

            vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(
                vkGetDeviceProcAddr(m_impl->m_device, "vkDestroyAccelerationStructureKHR"));

            vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(
                vkGetDeviceProcAddr(m_impl->m_device, "vkGetAccelerationStructureBuildSizesKHR"));

            vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(
                vkGetDeviceProcAddr(m_impl->m_device, "vkCmdBuildAccelerationStructuresKHR"));

            vkCreateRayTracingPipelinesKHR = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(
                vkGetDeviceProcAddr(m_impl->m_device, "vkCreateRayTracingPipelinesKHR"));

            vkCmdTraceRaysKHR = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(
                vkGetDeviceProcAddr(m_impl->m_device, "vkCmdTraceRaysKHR"));

            vkGetRayTracingShaderGroupHandlesKHR = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(
                vkGetDeviceProcAddr(m_impl->m_device, "vkGetRayTracingShaderGroupHandlesKHR"));
        }
    }

    // Get the queues
    {
        if (indices.transfer_family.has_value())
        {
            Queue& queue = this->queue(QueueType::Transfer);
            queue.m_impl->m_family_index = indices.transfer_family.value();

            vkGetDeviceQueue(m_impl->m_device, queue.m_impl->m_family_index, 0, &queue.m_impl->m_queue);
            OGFX_LOG("Retrieved transfer queue (family " + std::to_string(indices.transfer_family.value()) + ")");
        }

        if (indices.graphics_family.has_value())
        {
            Queue& queue = this->queue(QueueType::Graphics);
            queue.m_impl->m_family_index = indices.graphics_family.value();

            vkGetDeviceQueue(m_impl->m_device, queue.m_impl->m_family_index, 0, &queue.m_impl->m_queue);
            OGFX_LOG("Retrieved graphics queue (family " + std::to_string(indices.graphics_family.value()) + ")");
        }

        if (indices.compute_family.has_value())
        {
            Queue& queue = this->queue(QueueType::Compute);
            queue.m_impl->m_family_index = indices.compute_family.value();

            vkGetDeviceQueue(m_impl->m_device, queue.m_impl->m_family_index, 0, &queue.m_impl->m_queue);
            OGFX_LOG("Retrieved compute queue (family " + std::to_string(indices.compute_family.value()) + ")");
        }

        if (indices.present_family.has_value())
        {
            Queue& queue = this->queue(QueueType::Present);
            queue.m_impl->m_family_index = indices.present_family.value();

            vkGetDeviceQueue(m_impl->m_device, queue.m_impl->m_family_index, 0, &queue.m_impl->m_queue);
            OGFX_LOG("Retrieved present queue (family " + std::to_string(indices.present_family.value()) + ")");
        }
    }
}

Device::~Device() 
{
    if (m_impl && m_impl->m_device != VK_NULL_HANDLE)
    {
        vkDestroyDevice(m_impl->m_device, nullptr);

        OGFX_LOG("Vulkan logical device destroyed");
    }
}

Device::Device(Device&&) noexcept = default;
Device& Device::operator=(Device&&) noexcept = default;

void Device::wait_idle()
{
    OGFX_LOG("Waiting for Vulkan device to become idle");

    if (vkDeviceWaitIdle(m_impl->m_device) != VK_SUCCESS)
    {
        OGFX_LOG_ERROR("Failed to wait for Vulkan device idle");
        throw std::runtime_error("Failed to wait for Vulkan device idle.");
    }

    OGFX_LOG("Vulkan device is idle");
}

} // namespace ogfx