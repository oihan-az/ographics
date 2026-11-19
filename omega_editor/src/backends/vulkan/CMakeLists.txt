target_sources(
    omega_editor
    PRIVATE
        ui/vk_imgui_layer.cpp
        
)

find_package(Vulkan REQUIRED)

target_link_libraries(
    omega_editor
    PRIVATE
        Vulkan::Vulkan
)