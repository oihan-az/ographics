#include <oplatform/runtime.hpp>
#include <oplatform/window.hpp>

#include <ogfx/instance.hpp>
#include <ogfx/physical_device.hpp>
#include <ogfx/device.hpp>
#include <ogfx/surface.hpp>
#include <ogfx/swapchain.hpp>
#include <ogfx/queue.hpp>
#include <ogfx/command_pool.hpp>
#include <ogfx/command_buffer.hpp>
#include <ogfx/semaphore.hpp>
#include <ogfx/fence.hpp>
#include <ogfx/render_pass.hpp>
#include <ogfx/framebuffer.hpp>

int main()
{
    oplatform::Runtime runtime;

    oplatform::WindowDesc windowDesc;
    windowDesc.width = 1280;
    windowDesc.height = 720;
    windowDesc.title = "window example";
    windowDesc.resizable = true;
    windowDesc.fullscreen = false;

    oplatform::Window window(windowDesc);

    const auto extensions = runtime.required_instance_extensions();

    ogfx::InstanceDesc instanceDesc;
    instanceDesc.application_name = "ogfx Instance Test";
    instanceDesc.engine_name = "ographics";
    instanceDesc.debug = true;
    instanceDesc.extensions = extensions;

    ogfx::Instance instance(instanceDesc);

    ogfx::Surface surface = window.create_surface(instance);

    const std::vector<ogfx::PhysicalDevice> devices = instance.enumerate_physical_devices();
    ogfx::PhysicalDevice physicalDevice = instance.pick_physical_device(devices);

    physicalDevice.desc();

    ogfx::DeviceDesc deviceDesc;
    deviceDesc.enable_transfer = true;
    deviceDesc.enable_graphics = true;
    deviceDesc.enable_compute = true;
    deviceDesc.enable_present = true;

    ogfx::Device device(physicalDevice, deviceDesc, &surface);

    ogfx::SwapchainDesc swapchainDesc;
    swapchainDesc.width = window.width();
    swapchainDesc.height = window.height();
    swapchainDesc.format = ogfx::Format::BGRA8_UNORM;
    swapchainDesc.present_mode = ogfx::PresentMode::FIFO;
    swapchainDesc.image_count = 3;

    ogfx::Swapchain swapchain(physicalDevice, device, surface, swapchainDesc);



    // Render pass
    ogfx::RenderPassAttachmentDesc color_attachment;
    color_attachment.format = swapchainDesc.format;
    color_attachment.load_op = ogfx::LoadOp::Clear;
    color_attachment.store_op = ogfx::StoreOp::Store;
    color_attachment.initial_state = ogfx::ImageState::Undefined;
    color_attachment.final_state = ogfx::ImageState::Present;

    ogfx::RenderPassDesc renderPassDesc;
    renderPassDesc.attachments.push_back(color_attachment);

    ogfx::RenderPass renderPass(device, renderPassDesc);




    // Framebuffers
    std::vector<ogfx::Framebuffer> framebuffers;
    framebuffers.reserve(swapchain.image_count());

    for (uint32_t i = 0; i < swapchain.image_count(); ++i)
    {
        ogfx::FramebufferDesc framebuffer_desc;
        framebuffer_desc.render_pass = &renderPass;
        framebuffer_desc.attachments = {&swapchain.image_view(i)};
        framebuffer_desc.width = swapchainDesc.width;
        framebuffer_desc.height = swapchainDesc.height;
        framebuffer_desc.layers = 1;

        framebuffers.emplace_back(device, framebuffer_desc);
    }



    // Command pool
    ogfx::Queue& graphicsQueue = device.queue(ogfx::QueueType::Graphics);

    ogfx::CommandPoolDesc commandPoolDesc;
    commandPoolDesc.transient = false;
    commandPoolDesc.resettable = true;

    ogfx::CommandPool commandPool(device, graphicsQueue, commandPoolDesc);

    ogfx::CommandBuffer commandBuffer(commandPool);

    ogfx::Semaphore imageAvailable(device);
    ogfx::Semaphore renderFinished(device);

    const uint32_t imageIndex = swapchain.acquire_next_image(imageAvailable);

    ogfx::Fence inFlight(device);

    commandBuffer.reset();
    commandBuffer.begin();

    ogfx::RenderPassBeginInfo render_pass_begin_info;
    render_pass_begin_info.offset_x = 0;
    render_pass_begin_info.offset_y = 0;
    render_pass_begin_info.width = swapchainDesc.width;
    render_pass_begin_info.height = swapchainDesc.height;

    render_pass_begin_info.clear_color = {0.1f, 0.1f, 0.1f, 1.0f};

    commandBuffer.begin_render_pass(renderPass, framebuffers[imageIndex], render_pass_begin_info);

    commandBuffer.end_render_pass();

    commandBuffer.end();

    ogfx::SubmitDesc submitDesc;
    submitDesc.command_buffers.push_back(&commandBuffer);
    submitDesc.wait_semaphores.push_back({&imageAvailable, ogfx::PipelineStage::ColorAttachmentOutput});
    submitDesc.signal_semaphores.push_back(&renderFinished);
    submitDesc.fence = &inFlight;

    graphicsQueue.submit(submitDesc);

    inFlight.wait();

    ogfx::Queue& presentQueue = device.queue(ogfx::QueueType::Present);
    ogfx::PresentResult presentResult = presentQueue.present(swapchain, imageIndex, renderFinished);

    while (!window.should_close())
    {
        window.poll_events();
    }

    device.wait_idle();

    return 0;
}