add_executable(omega_editor)

target_compile_definitions(
    omega_editor 
    PRIVATE
        OMEGA_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
)

target_include_directories(
    omega_editor
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_sources(
    omega_editor
    PRIVATE
        "main.cpp"
        "src/editor.cpp"
        "src/rendering/ui_pipeline.cpp"
        "src/rendering/ui_feature.cpp"
        "src/layout/panel_slot.cpp"
        "src/layout/camera_viewport.cpp"
        "src/layout/viewport_toolbar.cpp"
        "src/panels/hierarchy_panel.cpp"
        "src/panels/inspection_panel.cpp"
        "src/panels/component_editor_registry.cpp"
        "src/icons/icons.cpp"
 )

find_package(imgui CONFIG REQUIRED)
find_package(imguizmo CONFIG REQUIRED)

target_link_libraries(
    omega_editor
    PRIVATE
        ographics::oplatform
        ographics::ogfx
        ographics::orendergraph
        ographics::orenderer
        ographics::omega_engine_core
        omega_engine
        imgui::imgui
        imguizmo::imguizmo
)

if(OGRAPHICS_API STREQUAL "VULKAN")

    add_subdirectory(src/backends/vulkan)

else()

    message(FATAL_ERROR "Unsupported OGRAPHICS_API : ${OGRAPHICS_API }")

endif()

include(
    "${CMAKE_SOURCE_DIR}/cmake/Shaders.cmake"
)

ogfx_compile_shaders(
    omega_editor
)