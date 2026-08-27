#include <debug.hpp>

#include "vk_swapchain.hpp"
#include "vk_device.hpp"
#include "vk_physical_device.hpp"
#include "vk_surface.hpp"
#include "vk_format.hpp"
#include "vk_present_mode.hpp"
#include "vk_image.hpp"
#include "vk_image_view.hpp"
#include "vk_image_usage.hpp"
#include "vk_semaphore.hpp"

#include <stdexcept>
#include <algorithm>
#include <vector>

namespace ogfx
{

struct SwapchainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities{};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
};

SwapchainSupportDetails query_swapchain_support(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
{
    SwapchainSupportDetails details;

    // Capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &details.capabilities);

    // Formats
    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);

    if (format_count > 0)
    {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, details.formats.data());
    }

    // Present mode
    uint32_t present_mode_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);

    if (present_mode_count > 0)
    {
        details.present_modes.resize(present_mode_count);

        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count,
                                                  details.present_modes.data());
    }

    return details;
}

VkSurfaceFormatKHR choose_surface_format(const std::vector<VkSurfaceFormatKHR>& formats, Format requested_format)
{
    const VkFormat requested = to_vk_format(requested_format);

    for (const auto& format : formats)
    {
        const Format available_format = from_vk_format(format.format);

        if (format.format == requested && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            OGFX_LOG("Surface format: requested " + std::string(to_string(requested_format)) + ", selected " +
                     std::string(to_string(available_format)));

            return format;
        }
    }

    const VkSurfaceFormatKHR available_surface_format = formats.front();
    const VkFormat available = available_surface_format.format;
    const Format available_format = from_vk_format(available);

    OGFX_LOG("Surface format: requested " + std::string(to_string(requested_format)) + ", selected fallback " +
             std::string(to_string(available_format)));

    return available_surface_format;
}

VkPresentModeKHR choose_present_mode(const std::vector<VkPresentModeKHR>& modes, PresentMode requested_mode)
{
    const VkPresentModeKHR requested = to_vk_present_mode(requested_mode);

    for (VkPresentModeKHR mode : modes)
    {
        const PresentMode available_mode = from_vk_present_mode(mode);

        if (mode == requested)
        {
            OGFX_LOG("Present mode: requested " + std::string(to_string(requested_mode)) + ", selected mode " +
                     std::string(to_string(available_mode)));

            return mode;
        }
    }

    VkPresentModeKHR available = VK_PRESENT_MODE_FIFO_KHR;
    const PresentMode available_mode = from_vk_present_mode(available);

    OGFX_LOG("Present mode: requested " + std::string(to_string(requested_mode)) + ", selected fallback " +
             std::string(to_string(available_mode)));

    return available;
}

VkExtent2D choose_extent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        OGFX_LOG("Swapchain extent: requested " + std::to_string(width) + "x" + std::to_string(height) + ", selected " +
                 std::to_string(capabilities.currentExtent.width) +
                 "x" + std::to_string(capabilities.currentExtent.height));

        return capabilities.currentExtent;
    }

    VkExtent2D extent{width, height};
    extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    OGFX_LOG("Swapchain extent: requested " + std::to_string(width) + "x" + std::to_string(height) +
             ", selected " + std::to_string(extent.width) + "x" + std::to_string(extent.height));

    return extent;
}

Swapchain::Swapchain(const PhysicalDevice& physical_device, const Device& device, const Surface& surface,
                     const SwapchainDesc& desc)
    : m_impl(std::make_unique<Impl>())
{
    const VkPhysicalDevice vk_physical_device = physical_device.m_impl->m_physical_device;
    const VkDevice vk_device = device.m_impl->m_device;
    const VkSurfaceKHR vk_surface = surface.m_impl->m_surface;

    OGFX_LOG("Creating Vulkan swapchain");

    const SwapchainSupportDetails support = query_swapchain_support(vk_physical_device, vk_surface);
    const VkSurfaceFormatKHR surface_format = choose_surface_format(support.formats, desc.format);
    const VkPresentModeKHR present_mode = choose_present_mode(support.present_modes, desc.present_mode);
    const VkExtent2D extent = choose_extent(support.capabilities, desc.width, desc.height);

    uint32_t image_count = std::max(desc.image_count, support.capabilities.minImageCount);
    if (support.capabilities.maxImageCount > 0)
    {
        image_count = std::min(image_count, support.capabilities.maxImageCount);
    }

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = vk_surface;
    create_info.minImageCount = image_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    create_info.preTransform = support.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(vk_device, &create_info, nullptr, &m_impl->m_swapchain) != VK_SUCCESS)
    {
        OGFX_LOG_ERROR("Failed to create Vulkan swapchain");
        throw std::runtime_error("Failed to create Vulkan swapchain.");
    }

    m_impl->m_device = vk_device;

    // Images
    uint32_t actual_image_count = 0;
    {
        if (vkGetSwapchainImagesKHR(vk_device, m_impl->m_swapchain, &actual_image_count, nullptr) != VK_SUCCESS)
        {
            OGFX_LOG_ERROR("Failed to query Vulkan swapchain images");
            throw std::runtime_error("Failed to query Vulkan swapchain images.");
        }

        std::vector<VkImage> vk_images(actual_image_count);

        if (vkGetSwapchainImagesKHR(vk_device, m_impl->m_swapchain, &actual_image_count, vk_images.data()) !=
            VK_SUCCESS)
        {
            OGFX_LOG_ERROR("Failed to retrieve Vulkan swapchain images");
            throw std::runtime_error("Failed to retrieve Vulkan swapchain images.");
        }

        m_images.reserve(actual_image_count);

        for (VkImage vk_image : vk_images)
        {
            // Swapchain images are externally created and owned by the swapchain.
            // Image only wraps the native handle and stores its resource properties.
            ImageDesc desc;
            desc.width = extent.width;
            desc.height = extent.height;
            desc.mip_levels = 1;
            desc.array_layers = 1;
            desc.format = from_vk_format(surface_format.format);
            desc.usage = from_vk_image_usage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
            desc.owned = false; // The VkImage is owned by the swapchain.

            Image image(device, desc);
            image.m_impl->m_image = vk_image;
            image.m_impl->m_device = device.m_impl->m_device;

            m_images.emplace_back(std::move(image));
        }
    }

    // Image views
    {
        for (Image& image : m_images)
        {
            ImageView image_view(image);
            m_image_views.emplace_back(std::move(image_view));
        }
    }

    OGFX_LOG("Vulkan swapchain created with " + std::to_string(actual_image_count) + " image(s)");
}

Swapchain::~Swapchain()
{
    if (m_impl && m_impl->m_swapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(m_impl->m_device, m_impl->m_swapchain, nullptr);
        OGFX_LOG("Vulkan swapchain destroyed");
    }
}

Swapchain::Swapchain(Swapchain&&) noexcept = default;
Swapchain& Swapchain::operator=(Swapchain&&) noexcept = default;

uint32_t Swapchain::acquire_next_image(Semaphore& signal_semaphore)
{
    uint32_t image_index = 0;

    VkResult result = vkAcquireNextImageKHR(m_impl->m_device, m_impl->m_swapchain, UINT64_MAX,
                                            signal_semaphore.m_impl->m_semaphore, VK_NULL_HANDLE, &image_index);

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        OGFX_LOG_ERROR("Failed to acquire Vulkan swapchain image");

        throw std::runtime_error("Failed to acquire Vulkan swapchain image.");
    }

    OGFX_LOG("Acquired Vulkan swapchain image [" + std::to_string(image_index) + "]");

    return image_index;
}

} // namespace ogfx