#include <debug.hpp>

#include "vk_framebuffer.hpp"
#include "vk_device.hpp"
#include "vk_image_view.hpp"
#include "vk_render_pass.hpp"

#include <stdexcept>

namespace ogfx
{

Framebuffer::Framebuffer(const Device& device, const FramebufferDesc& desc) : m_impl(std::make_unique<Impl>())
{
    m_impl->m_device = device.m_impl->m_device;

    std::vector<VkImageView> attachments;
    attachments.reserve(desc.attachments.size());

    OGFX_LOG("Creating Vulkan framebuffer with " + std::to_string(desc.attachments.size()) + " attachment(s)");

    for (size_t i = 0; i < desc.attachments.size(); ++i)
    {
        const ImageView* image_view = desc.attachments[i];

        attachments.push_back(image_view->m_impl->m_image_view);
    }

    VkFramebufferCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

    create_info.renderPass = desc.render_pass->m_impl->m_render_pass;

    create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
    create_info.pAttachments = attachments.data();

    create_info.width = desc.width;
    create_info.height = desc.height;
    create_info.layers = desc.layers;

    if (vkCreateFramebuffer(m_impl->m_device, &create_info, nullptr, &m_impl->m_framebuffer) != VK_SUCCESS)
    {
        OGFX_LOG_ERROR("Failed to create Vulkan framebuffer");

        throw std::runtime_error("Failed to create Vulkan framebuffer.");
    }

    OGFX_LOG("Vulkan framebuffer created (" + std::to_string(desc.width) + "x" + std::to_string(desc.height) + ")");
}

Framebuffer::~Framebuffer()
{
    if (m_impl && m_impl->m_framebuffer != VK_NULL_HANDLE)
    {
        vkDestroyFramebuffer(m_impl->m_device, m_impl->m_framebuffer, nullptr);

        OGFX_LOG("Vulkan framebuffer destroyed");
    }
}

Framebuffer::Framebuffer(Framebuffer&&) noexcept = default;
Framebuffer& Framebuffer::operator=(Framebuffer&&) noexcept = default;

} // namespace ogfx