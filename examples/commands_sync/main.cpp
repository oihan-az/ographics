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

    ogfx::Queue& graphicsQueue = device.queue(ogfx::QueueType::Graphics);

    ogfx::CommandPoolDesc commandPoolDesc;
    commandPoolDesc.transient = false;
    commandPoolDesc.resettable = true;

    ogfx::CommandPool command_pool(device, graphicsQueue, commandPoolDesc);

    ogfx::CommandBuffer command_buffer(command_pool);

    ogfx::Semaphore image_available(device);
    ogfx::Semaphore render_finished(device);

    const uint32_t image_index = swapchain.acquire_next_image(image_available);

    ogfx::Fence in_flight(device, true);

    command_buffer.reset();
    command_buffer.begin();
    command_buffer.end();

    // Wait for the fence before using the command buffer.
    in_flight.wait();
    in_flight.reset();

    ogfx::SubmitDesc submit_desc;
    submit_desc.command_buffers.push_back(&command_buffer);
    submit_desc.wait_semaphores.push_back({&image_available, ogfx::PipelineStage::ColorAttachmentOutput});
    submit_desc.signal_semaphores.push_back(&render_finished);
    submit_desc.fence = &in_flight;

    graphicsQueue.submit(submit_desc);

    // Wait until the GPU has completed the submission.
    in_flight.wait();

    //while (!window.should_close())
    //{
    //    window.poll_events();
    //}

    return 0;
}