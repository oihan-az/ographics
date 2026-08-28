set(OGRAPHICS_SHADER_DIR
    "${OGRAPHICS_ASSETS_DIR}/shaders"
)

set(OGRAPHICS_SHADER_OUTPUT_DIR
    "${CMAKE_BINARY_DIR}/shaders"
)

set(DXC_EXECUTABLE
    "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/directx-dxc/dxc.exe"
)


if(OGRAPHICS_API STREQUAL "VULKAN")

    set(OGRAPHICS_SHADER_FORMAT "spv")

elseif(OGRAPHICS_API STREQUAL "D3D12")

    set(OGRAPHICS_SHADER_FORMAT "dxil")

elseif(OGRAPHICS_API STREQUAL "METAL")

    message(FATAL_ERROR
        "Metal shader compilation is not supported yet."
    )

else()

    message(FATAL_ERROR
        "Unsupported graphics API: ${OGRAPHICS_API}"
    )

endif()


set(DXC_ARGUMENTS)

if(OGRAPHICS_API STREQUAL "VULKAN")

    list(APPEND DXC_ARGUMENTS
        -spirv
    )

elseif(OGRAPHICS_API STREQUAL "D3D12")

    # DXIL is the default DXC output.
    
endif()


if(NOT EXISTS "${DXC_EXECUTABLE}")
    message(FATAL_ERROR
        "DXC executable not found: ${DXC_EXECUTABLE}"
    )
endif()

message(STATUS "DXC executable: ${DXC_EXECUTABLE}")

file(GLOB_RECURSE OGRAPHICS_SHADER_SOURCES
    CONFIGURE_DEPENDS
    "${OGRAPHICS_SHADER_DIR}/*.hlsl"
)

set(OGRAPHICS_SHADER_OUTPUTS)


foreach(SHADER_SOURCE ${OGRAPHICS_SHADER_SOURCES})

    file(RELATIVE_PATH SHADER_RELATIVE
        "${OGRAPHICS_SHADER_DIR}"
        "${SHADER_SOURCE}"
    )

    string(REGEX MATCH
        "\\.(vert|frag|comp)\\.hlsl$"
        SHADER_STAGE_MATCH
        "${SHADER_RELATIVE}"
    )

    if(NOT SHADER_STAGE_MATCH)
        message(FATAL_ERROR
            "Unsupported shader file: ${SHADER_SOURCE}"
        )
    endif()

    if(SHADER_RELATIVE MATCHES "\\.vert\\.hlsl$")
        set(SHADER_TYPE "vertex")
        set(SHADER_PROFILE "vs_6_0")
        set(SHADER_ENTRY_POINT "VSMain")
    elseif(SHADER_RELATIVE MATCHES "\\.frag\\.hlsl$")
        set(SHADER_TYPE "fragment")
        set(SHADER_PROFILE "ps_6_0")
        set(SHADER_ENTRY_POINT "PSMain")
    elseif(SHADER_RELATIVE MATCHES "\\.comp\\.hlsl$")
        set(SHADER_TYPE "compute")
        set(SHADER_PROFILE "cs_6_0")
        set(SHADER_ENTRY_POINT "CSMain")
    endif()

    string(REGEX REPLACE
        "\\.hlsl$"
        ".${OGRAPHICS_SHADER_FORMAT}"
        SHADER_OUTPUT_RELATIVE
        "${SHADER_RELATIVE}"
    )

    set(SHADER_OUTPUT
        "${OGRAPHICS_SHADER_OUTPUT_DIR}/${SHADER_OUTPUT_RELATIVE}"
    )

    get_filename_component(
        SHADER_OUTPUT_DIR
        "${SHADER_OUTPUT}"
        DIRECTORY
    )

    add_custom_command(
        OUTPUT
            "${SHADER_OUTPUT}"

        COMMAND
            ${CMAKE_COMMAND} -E make_directory
            "${SHADER_OUTPUT_DIR}"

        COMMAND
            "${DXC_EXECUTABLE}"
            ${DXC_ARGUMENTS}
            -T "${SHADER_PROFILE}"
            -E "${SHADER_ENTRY_POINT}"
            -Fo "${SHADER_OUTPUT}"
            "${SHADER_SOURCE}"

        DEPENDS
            "${SHADER_SOURCE}"

        COMMENT
            "Compiling ${SHADER_TYPE} HLSL shader: ${SHADER_RELATIVE}"

        VERBATIM
    )

    list(APPEND
        OGRAPHICS_SHADER_OUTPUTS
        "${SHADER_OUTPUT}"
    )

endforeach()


add_custom_target(
    ogfx_shaders
    DEPENDS
        ${OGRAPHICS_SHADER_OUTPUTS}
)