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
        
        
)

find_package(Vulkan REQUIRED)

target_link_libraries(
    ogfx
    PRIVATE
        Vulkan::Vulkan
)