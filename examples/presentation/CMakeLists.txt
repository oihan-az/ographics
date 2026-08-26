add_executable(presentation_test)

target_sources(
    presentation_test
    PRIVATE
        main.cpp
 )

target_link_libraries(
    presentation_test
    PRIVATE
        ographics::oplatform
        ographics::ogfx
)