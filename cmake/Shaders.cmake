function(ogfx_compile_shaders TARGET)

    set(SHADER_DIR
        "${CMAKE_SOURCE_DIR}/assets/shaders"
    )

    set(SHADER_OUTPUT_DIR
        "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/shaders"
    )

    set(DXC_EXECUTABLE
        "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/directx-dxc/dxc.exe"
    )

    if(NOT EXISTS "${DXC_EXECUTABLE}")
        message(FATAL_ERROR
            "DXC executable not found: ${DXC_EXECUTABLE}"
        )
    endif()

    message(STATUS "DXC executable: ${DXC_EXECUTABLE}")


    if(OGRAPHICS_API STREQUAL "VULKAN")

        set(SHADER_FORMAT "spv")

    elseif(OGRAPHICS_API STREQUAL "D3D12")

        set(SHADER_FORMAT "dxil")

    elseif(OGRAPHICS_API STREQUAL "METAL")

        message(FATAL_ERROR
            "Metal shader compilation is not supported yet."
        )

    else()

        message(FATAL_ERROR
            "Unsupported graphics API: ${OGRAPHICS_API}"
        )

    endif()


    file(GLOB_RECURSE SHADER_SOURCES
        CONFIGURE_DEPENDS
        "${SHADER_DIR}/*.hlsl"
    )


    set(SHADER_OUTPUTS)


    foreach(SHADER_SOURCE ${SHADER_SOURCES})

        file(RELATIVE_PATH SHADER_RELATIVE
            "${SHADER_DIR}"
            "${SHADER_SOURCE}"
        )


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

        else()

            message(FATAL_ERROR
                "Unsupported shader file: ${SHADER_SOURCE}"
            )

        endif()


        string(REGEX REPLACE
            "\\.hlsl$"
            ".${SHADER_FORMAT}"
            SHADER_OUTPUT_RELATIVE
            "${SHADER_RELATIVE}"
        )


        set(SHADER_OUTPUT
            "${SHADER_OUTPUT_DIR}/${SHADER_OUTPUT_RELATIVE}"
        )


        get_filename_component(
            SHADER_OUTPUT_DIR_ABSOLUTE
            "${SHADER_OUTPUT}"
            DIRECTORY
        )


        set(DXC_ARGUMENTS)

        if(OGRAPHICS_API STREQUAL "VULKAN")

            list(APPEND DXC_ARGUMENTS
                -spirv
            )

        endif()


        add_custom_command(
            OUTPUT
                "${SHADER_OUTPUT}"

            COMMAND
                ${CMAKE_COMMAND} -E make_directory
                "${SHADER_OUTPUT_DIR_ABSOLUTE}"

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
            SHADER_OUTPUTS
            "${SHADER_OUTPUT}"
        )

    endforeach()


    add_custom_target(
        "${TARGET}_shaders"
        DEPENDS
            ${SHADER_OUTPUTS}
    )


    add_dependencies(
        "${TARGET}"
        "${TARGET}_shaders"
    )


    target_compile_definitions(
        "${TARGET}"
        PRIVATE
            OGRAPHICS_SHADER_FORMAT="${SHADER_FORMAT}"
    )

endfunction()