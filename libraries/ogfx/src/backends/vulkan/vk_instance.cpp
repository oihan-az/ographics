#include <ogfx/instance.hpp>

#include <debug.hpp>

#include <vulkan/vulkan.h>

#include "vk_instance.hpp"
#include "vk_physical_device.hpp"

#include <cstring>
#include <stdexcept>
#include <vector>

namespace ogfx
{

Instance::Instance(const InstanceDesc& desc) : m_impl(std::make_unique<Impl>())
{
    OGFX_LOG(std::string("Creating Vulkan instance for app \"") + desc.application_name + "\"");

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = desc.application_name;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = desc.engine_name;
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // Extensions
    std::vector<const char*> extensions = desc.extensions;

    if (desc.debug)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    // Validation layers
    const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    if (desc.debug)
    {
        {
            // Get available validation layers
            uint32_t layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

            // Check validation layer support
            for (const char* layerName : validationLayers)
            {
                bool layerFound = false;

                for (const auto& layerProperties : availableLayers)
                {
                    if (strcmp(layerName, layerProperties.layerName) == 0)
                    {
                        layerFound = true;
                        break;
                    }
                }

                if (!layerFound)
                {
                    throw std::runtime_error("Validation layers requested, but not available!");
                }
            }
        }

        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateInstance(&createInfo, nullptr, &m_impl->m_instance) != VK_SUCCESS)
    {
        OGFX_LOG_ERROR("Failed to create Vulkan instance");
        throw std::runtime_error("Failed to create instance!");
    }

    OGFX_LOG("Vulkan instance created");
}

Instance::~Instance()
{
    if (m_impl->m_instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(m_impl->m_instance, nullptr);

        OGFX_LOG("Vulkan instance destroyed");
    }
}

Instance::Instance(Instance&&) noexcept = default;

Instance& Instance::operator=(Instance&&) noexcept = default;

[[nodiscard]] NativeInstanceHandle Instance::native_handle() const
{
    return reinterpret_cast<void*>(m_impl->m_instance);
}

std::vector<PhysicalDevice> Instance::enumerate_physical_devices() const 
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_impl->m_instance, &deviceCount, nullptr);

    std::vector<VkPhysicalDevice> vk_devices(deviceCount);
    vkEnumeratePhysicalDevices(m_impl->m_instance, &deviceCount, vk_devices.data());

    OGFX_LOG("Found " + std::to_string(deviceCount) + " physical device(s)");

    std::vector<PhysicalDevice> devices;
    devices.reserve(deviceCount);
    for (VkPhysicalDevice& vk_device : vk_devices)
    {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(vk_device, &properties);

        // Store the properties
        PhysicalDeviceDesc desc;
        desc.name = properties.deviceName;
        desc.vendor_id = properties.vendorID;
        desc.device_id = properties.deviceID;
        desc.api_version = properties.apiVersion;
        desc.driver_version = properties.driverVersion;

        // Create a physical device abstraction
        std::unique_ptr<PhysicalDevice::Impl> impl = std::make_unique<PhysicalDevice::Impl>();
        impl->m_physical_device = vk_device;
        devices.emplace_back(desc, std::move(impl));

        OGFX_LOG("Physical device [" + std::to_string(devices.size() - 1) + "]: " + devices.back().m_desc.name);
    }

    return devices;
}

PhysicalDevice Instance::pick_physical_device(const std::vector<PhysicalDevice>& physicalDevice) const
{
    if (physicalDevice.empty())
    {
        OGFX_LOG_ERROR("Cannot select a physical device: none available");
        throw std::runtime_error("No physical devices available.");
    }

    // TODO: Pick based on requirements
    const PhysicalDevice& selectedDevice = physicalDevice.front();

    OGFX_LOG("Selected physical device: " + selectedDevice.m_desc.name);

    auto impl = std::make_unique<PhysicalDevice::Impl>();
    impl->m_physical_device = selectedDevice.m_impl->m_physical_device;

    return PhysicalDevice(selectedDevice.m_desc, std::move(impl));
}

} // namespace ogfx