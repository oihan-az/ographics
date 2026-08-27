target_sources(
    ogfx
    PRIVATE
        vk_instance.cpp
        vk_physical_device.cpp
        vk_device.cpp
        vk_surface.cpp
        vk_swapchain.cpp
        vk_format.cpp
        vk_present_mode.cpp
        vk_image.cpp
        vk_image_view.cpp
        vk_image_usage.cpp
        vk_queue.cpp
        vk_command_pool.cpp
        vk_command_buffer.cpp
        vk_semaphore.cpp
        vk_pipeline_stage.cpp
        vk_fence.cpp
        
        
)

find_package(Vulkan REQUIRED)

target_link_libraries(
    ogfx
    PRIVATE
        Vulkan::Vulkan
)