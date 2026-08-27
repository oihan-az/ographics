#include <debug.hpp>

#include "vk_physical_device.hpp"
#include "vk_surface.hpp"

#include <cstring>
#include <stdexcept>
#include <vector>

namespace ogfx
{

PhysicalDevice::PhysicalDevice(const PhysicalDeviceDesc& desc, std::unique_ptr<Impl> impl)
    : m_desc(desc), m_impl(std::make_unique<Impl>())
{
    m_impl->m_physical_device = impl->m_physical_device;
}

PhysicalDevice::~PhysicalDevice() = default;

PhysicalDevice::PhysicalDevice(PhysicalDevice&&) noexcept = default;
PhysicalDevice& PhysicalDevice::operator=(PhysicalDevice&&) noexcept = default;

const PhysicalDeviceDesc& PhysicalDevice::desc() const
{
    const auto format_version = [](uint32_t version)
    {
        return std::to_string(VK_VERSION_MAJOR(version)) + "." + 
               std::to_string(VK_VERSION_MINOR(version)) + "." +
               std::to_string(VK_VERSION_PATCH(version));
    };

    const auto format_hex = [](uint32_t value)
    {
        std::stringstream stream;
        stream << std::hex << std::uppercase << value;
        return stream.str();
    };

    const auto vendor_name = [](uint32_t vendor_id) -> std::string
    {
        switch (vendor_id)
        {
        case 0x10DE:
            return "NVIDIA";

        case 0x1002:
            return "AMD";

        case 0x8086:
            return "Intel";

        default:
            return "Unknown";
        }
    };

    OGFX_LOG("Physical device:");
    OGFX_LOG("  Name: " + m_desc.name);
    OGFX_LOG("  Vendor: " + vendor_name(m_desc.vendor_id) + 
             " (0x" + format_hex(m_desc.vendor_id) + ")");
    OGFX_LOG("  Device ID: 0x" + format_hex(m_desc.device_id));
    OGFX_LOG("  API Version: " + format_version(m_desc.api_version));
    OGFX_LOG("  Driver Version: " + format_version(m_desc.driver_version));

    return m_desc;
}

QueueFamilyIndices PhysicalDevice::Impl::FindQueueFamilies(const Surface* surface) const
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physical_device, &queueFamilyCount, queueFamilies.data());

    // Get queue family indeces
    // Prefer dedicated over shared queues
    for (uint32_t i = 0; i < queueFamilyCount; ++i)
    {
        const VkQueueFlags flags = queueFamilies[i].queueFlags;
        const bool graphics = (flags & VK_QUEUE_GRAPHICS_BIT) != 0;
        const bool compute = (flags & VK_QUEUE_COMPUTE_BIT) != 0;
        const bool transfer = (flags & VK_QUEUE_TRANSFER_BIT) != 0;

        // Graphics
        if (graphics && !indices.graphics_family.has_value())
        {
            indices.graphics_family = i;
        }

        // Compute
        if (compute)
        {
            const bool dedicated_compute = !graphics;

            if (!indices.compute_family.has_value() || dedicated_compute)
            {
                indices.compute_family = i;
            }
        }

        // Transfer
        if (transfer)
        {
            const bool dedicated_transfer = !graphics && !compute;

            if (!indices.transfer_family.has_value() || dedicated_transfer)
            {
                indices.transfer_family = i;
            }
        }

        // Present
        if (surface != nullptr)
        {
            VkBool32 present_support = VK_FALSE;
            
            vkGetPhysicalDeviceSurfaceSupportKHR(m_physical_device, i, surface->m_impl->m_surface, &present_support);

            if (present_support && !indices.present_family.has_value())
            {
                indices.present_family = i;
            }
        }
    }

    return indices;
}

} // namespace ogfx