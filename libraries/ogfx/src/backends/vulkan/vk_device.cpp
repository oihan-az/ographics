#include <ogfx/device.hpp>
#include <ogfx/physical_device.hpp>

#include <debug.hpp>

#include <vulkan/vulkan.h>

#include "vk_physical_device.hpp"

#include <stdexcept>
#include <vector>
#include <set>

namespace ogfx
{

struct Device::Impl
{
    VkDevice m_device = VK_NULL_HANDLE;

    VkQueue m_transfer_queue = VK_NULL_HANDLE;
    VkQueue m_graphics_queue = VK_NULL_HANDLE;
    VkQueue m_compute_queue = VK_NULL_HANDLE;
};

Device::Device(const PhysicalDevice& physical_device, const DeviceDesc& desc) : m_impl(std::make_unique<Impl>())
{
    OGFX_LOG("Creating Vulkan logical device");

    ogfx::QueueFamilyIndices indices = physical_device.m_impl->FindQueueFamilies();

    std::set<uint32_t> uniqueQueueFamilies;

    if (desc.enable_graphics)
    {
        if (!indices.graphics_family.has_value())
        {
            throw std::runtime_error("Graphics queue requested but not available.");
        }

        uniqueQueueFamilies.insert(indices.graphics_family.value());
    }

    if (desc.enable_compute)
    {
        if (!indices.compute_family.has_value())
        {
            throw std::runtime_error("Compute queue requested but not available.");
        }

        uniqueQueueFamilies.insert(indices.compute_family.value());
    }

    if (desc.enable_transfer)
    {
        if (!indices.transfer_family.has_value())
        {
            throw std::runtime_error("Transfer queue requested but not available.");
        }

        uniqueQueueFamilies.insert(indices.transfer_family.value());
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

    // Device features
    VkPhysicalDeviceFeatures deviceFeatures{};

    // Device
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = 0;
    createInfo.enabledLayerCount = 0;

    if (vkCreateDevice(physical_device.m_impl->m_physical_device, &createInfo, nullptr, &m_impl->m_device) != VK_SUCCESS)
    {
        OGFX_LOG_ERROR("Failed to create logical device");
        throw std::runtime_error("Failed to create logical device!");
    }

    OGFX_LOG("Vulkan logical device created");

    // Get the queues
    vkGetDeviceQueue(m_impl->m_device, indices.transfer_family.value(), 0, &m_impl->m_transfer_queue);
    vkGetDeviceQueue(m_impl->m_device, indices.graphics_family.value(), 0, &m_impl->m_graphics_queue);
    vkGetDeviceQueue(m_impl->m_device, indices.compute_family.value(), 0, &m_impl->m_compute_queue);

    OGFX_LOG("Retrieved transfer queue (family " + std::to_string(indices.transfer_family.value()) + ")");
    OGFX_LOG("Retrieved graphics queue (family " + std::to_string(indices.graphics_family.value()) + ")");
    OGFX_LOG("Retrieved compute queue (family " + std::to_string(indices.compute_family.value()) + ")");
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

} // namespace ogfx