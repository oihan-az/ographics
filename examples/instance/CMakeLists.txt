add_executable(instance_test)

target_sources(
    instance_test
    PRIVATE
        main.cpp
 )

target_link_libraries(
    instance_test
    PRIVATE
        ographics::ogfx
)