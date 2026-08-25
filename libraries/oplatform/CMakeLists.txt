add_library(oplatform STATIC)

add_library(ographics::platform ALIAS oplatform)

target_compile_features(
    oplatform
    PUBLIC
        cxx_std_20
)

target_include_directories(
    oplatform
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_sources(
    oplatform
    PRIVATE
        src/runtime.cpp
        src/window.cpp
)

find_package(glfw3 CONFIG REQUIRED)

target_link_libraries(
    oplatform
    PRIVATE
        glfw
)