#include <debug.hpp>

#include "vk_render_pass.hpp"
#include "vk_device.hpp"
#include "vk_format.hpp"
#include "vk_image_state.hpp"
#include "vk_load_store_op.hpp"

#include <stdexcept>

namespace ogfx
{

RenderPass::RenderPass(const Device& device, const RenderPassDesc& desc) : m_impl(std::make_unique<Impl>())
{
    m_impl->m_device = device.m_impl->m_device;

    OGFX_LOG("Creating Vulkan render pass with " + std::to_string(desc.attachments.size()) + " attachment(s)");

    std::vector<VkAttachmentDescription> attachments;
    attachments.reserve(desc.attachments.size());

    for (size_t i = 0; i < desc.attachments.size(); ++i)
    {
        const RenderPassAttachmentDesc& attachment = desc.attachments[i];

        OGFX_LOG("Attachment " + std::to_string(i) + ": format=" + std::string(to_string(attachment.format)) +
                 ", load_op=" + std::string(to_string(attachment.load_op)) +
                 ", store_op=" + std::string(to_string(attachment.store_op)) +
                 ", initial_state=" + std::string(to_string(attachment.initial_state)) +
                 ", final_state=" + std::string(to_string(attachment.final_state)));

        VkAttachmentDescription vk_attachment{};
        vk_attachment.format = to_vk_format(attachment.format);
        vk_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        vk_attachment.loadOp = to_vk_load_op(attachment.load_op);
        vk_attachment.storeOp = to_vk_store_op(attachment.store_op);
        vk_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        vk_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        vk_attachment.initialLayout = to_vk_image_state(attachment.initial_state);
        vk_attachment.finalLayout = to_vk_image_state(attachment.final_state);

        attachments.push_back(vk_attachment);
    }

    VkAttachmentReference color_reference{};
    color_reference.attachment = 0;
    color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_reference;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;

    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

    create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
    create_info.pAttachments = attachments.data();

    create_info.subpassCount = 1;
    create_info.pSubpasses = &subpass;

    create_info.dependencyCount = 1;
    create_info.pDependencies = &dependency;

    if (vkCreateRenderPass(m_impl->m_device, &create_info, nullptr, &m_impl->m_render_pass) != VK_SUCCESS)
    {
        OGFX_LOG_ERROR("Failed to create Vulkan render pass");
        throw std::runtime_error("Failed to create Vulkan render pass.");
    }

    OGFX_LOG("Vulkan render pass created");
}

RenderPass::~RenderPass()
{
    if (m_impl && m_impl->m_render_pass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(m_impl->m_device, m_impl->m_render_pass, nullptr);

        OGFX_LOG("Vulkan render pass destroyed");
    }
}

RenderPass::RenderPass(RenderPass&&) noexcept = default;
RenderPass& RenderPass::operator=(RenderPass&&) noexcept = default;

} // namespace ogfx