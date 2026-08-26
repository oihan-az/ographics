add_executable(set_up_test)

target_sources(
    set_up_test
    PRIVATE
        main.cpp
 )

target_link_libraries(
    set_up_test
    PRIVATE
        ographics::ogfx
)