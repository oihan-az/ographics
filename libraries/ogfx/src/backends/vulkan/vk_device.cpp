#include <debug.hpp>

#include "vk_device.hpp"
#include "vk_physical_device.hpp"
#include "vk_queue.hpp"

#include <stdexcept>
#include <vector>
#include <set>

namespace ogfx
{

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

    // Device features
    VkPhysicalDeviceFeatures supported_features{};
    vkGetPhysicalDeviceFeatures(physical_device.m_impl->m_physical_device, &supported_features);

    const bool request_multi_draw_indirect = ogfx::has_flag(desc.features, ogfx::DeviceFeature::MultiDrawIndirect);
    const bool request_draw_indirect_first_instance = ogfx::has_flag(desc.features, ogfx::DeviceFeature::DrawIndirectFirstInstance);
    const bool request_sampler_anisotropy = ogfx::has_flag(desc.features, ogfx::DeviceFeature::SamplerAnisotropy);

    if (request_multi_draw_indirect && !supported_features.multiDrawIndirect)
    {
        throw std::runtime_error("Multi draw indirect is not supported.");
    }

    if (request_draw_indirect_first_instance && !supported_features.drawIndirectFirstInstance)
    {
        throw std::runtime_error("Draw indirect first instance is not supported.");
    }

    if (request_sampler_anisotropy && !supported_features.samplerAnisotropy)
    {
        throw std::runtime_error("Sampler Anisotropy is not supported.");
    }

    VkPhysicalDeviceFeatures device_features{};
    device_features.multiDrawIndirect = request_multi_draw_indirect ? VK_TRUE : VK_FALSE;
    device_features.drawIndirectFirstInstance = request_draw_indirect_first_instance ? VK_TRUE : VK_FALSE;
    device_features.samplerAnisotropy = request_sampler_anisotropy ? VK_TRUE : VK_FALSE;

    // Extensions
    std::vector<const char*> extensions;

    if (request_present)
    {
        extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
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

    if (vkCreateDevice(physical_device.m_impl->m_physical_device, &createInfo, nullptr, &m_impl->m_device) != VK_SUCCESS)
    {
        OGFX_LOG_ERROR("Failed to create logical device");
        throw std::runtime_error("Failed to create logical device!");
    }

    OGFX_LOG("Vulkan logical device created");

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