target_sources(
    oplatform
    PRIVATE
        glfw_vk_runtime.cpp
        glfw_vk_window.cpp
        
)

find_package(Vulkan REQUIRED)

target_link_libraries(
    oplatform
    PRIVATE
        Vulkan::Vulkan
)