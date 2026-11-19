#include "vk_imgui_layer.hpp"

#include <ogfx/backends/vulkan/vulkan_access.hpp>

#include <oplatform/backends/glfw/glfw_access.hpp>

#include <omega/platform/platform_system.hpp>
#include <omega/graphics/graphics_system.hpp>

#include <omega_editor/rendering/ui_feature.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

namespace omega_editor
{

ImGuiLayer::ImGuiLayer() : m_impl(std::make_unique<Impl>()) {}

ImGuiLayer::~ImGuiLayer()
{
    shutdown();
}

void ImGuiLayer::initialize(omega::PlatformSystem& platform, omega::GraphicsSystem& graphics, UIFeature& ui_render_feature)
{
    if (m_initialized)
        return;

    // Create ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Initialize GLFW backend
    ImGui_ImplGlfw_InitForVulkan(oplatform::glfw::GLFWAccess::window_handle(platform.window()), true);

    // Create Vulkan descriptor pool

    // Initialize Vulkan backend
    ImGui_ImplVulkan_InitInfo init_info{};

    ogfx::Device& device = graphics.device();
    ogfx::Queue& graphics_queue = device.queue(ogfx::QueueType::Graphics);

    init_info.ApiVersion = VK_API_VERSION_1_3;
    init_info.Instance = ogfx::vulkan::VulkanAccess::instance(graphics.instance());
    init_info.PhysicalDevice = ogfx::vulkan::VulkanAccess::physical_device(graphics.physical_device());
    init_info.Device = ogfx::vulkan::VulkanAccess::device(device);
    init_info.QueueFamily = ogfx::vulkan::VulkanAccess::queue_family_index(graphics_queue);
    init_info.Queue = ogfx::vulkan::VulkanAccess::queue(graphics_queue);

    // TODO: Don't hardcode values
    init_info.DescriptorPoolSize = 1000;
    init_info.MinImageCount = 2;
    init_info.ImageCount = graphics.swapchain().image_count();

    auto node = ui_render_feature.GetNode();

    init_info.PipelineInfoMain.RenderPass = ogfx::vulkan::VulkanAccess::render_pass(*node->context().render_pass);
    init_info.PipelineInfoMain.Subpass = 0;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&init_info);

    m_initialized = true;
}

void ImGuiLayer::begin_frame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::end_frame()
{
    ImGui::Render();
}

void ImGuiLayer::render(ogfx::CommandBuffer& command_buffer)
{
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), ogfx::vulkan::VulkanAccess::command_buffer(command_buffer));
}

void ImGuiLayer::shutdown()
{
    if (!m_impl || !m_initialized)
        return;

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    m_initialized = false;
}

} // namespace omega_editor