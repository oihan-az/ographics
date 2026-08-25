target_sources(
    ogfx
    PRIVATE
        vk_instance.cpp
        vk_physical_device.cpp
        vk_device.cpp
        
)

find_package(Vulkan REQUIRED)

target_link_libraries(
    ogfx
    PRIVATE
        Vulkan::Vulkan
)