add_library(omega_engine_core STATIC)

add_library(ographics::omega_engine_core ALIAS omega_engine_core)

target_compile_features(
    omega_engine_core
    PUBLIC
        cxx_std_20
)

target_include_directories(
    omega_engine_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_definitions(
    omega_engine_core
    PRIVATE
        omega_engine_core_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

target_compile_definitions(
    omega_engine_core
    PRIVATE
        omega_engine_core_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    PUBLIC
        OMEGA_CLIP_DEPTH_ZERO_TO_ONE=${OMEGA_CLIP_DEPTH_ZERO_TO_ONE}
        OMEGA_PROJECTION_LEFT_HANDED=${OMEGA_PROJECTION_LEFT_HANDED}
)

target_sources(
    omega_engine_core
    PRIVATE
        "src/math/quaternion.cpp"
        "src/math/matrix_transform.cpp"
        "src/math/matrix_projection.cpp"
        "src/entity/entity_iterator.cpp"
        "src/event/event_system.cpp"
        "src/component/component_manager.cpp"
        "src/scene/scene.cpp"
)

find_package(glm CONFIG REQUIRED)
target_link_libraries(omega_engine_core PUBLIC glm::glm)