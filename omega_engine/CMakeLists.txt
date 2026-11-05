add_library(omega_engine STATIC)

target_compile_features(
    omega_engine
    PUBLIC
        cxx_std_20
)

target_include_directories(
    omega_engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_sources(
    omega_engine
    PRIVATE
        src/scene/scene.cpp
        src/scene/entity_iterator.cpp

        src/core/component_manager.cpp
        src/core/event.cpp

        src/math/matrix_transform.cpp
        src/math/quaternion.cpp

        src/components/transform.cpp

 )

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

target_link_libraries(
    omega_engine
    PRIVATE
        ographics::oplatform
        ographics::ogfx
        ographics::orendergraph
        ographics::orenderer
)

find_package(glm CONFIG REQUIRED)

target_link_libraries(omega_engine PRIVATE glm::glm)

include(
    "${CMAKE_SOURCE_DIR}/cmake/Shaders.cmake"
)

ogfx_compile_shaders(
    omega_engine
)