add_executable(oplatform_test)

target_sources(
    oplatform_test
    PRIVATE
        main.cpp
 )

target_link_libraries(
    oplatform_test
    PRIVATE
        ographics::platform
)