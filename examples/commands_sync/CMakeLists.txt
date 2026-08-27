add_executable(commands_sync_test)

target_sources(
    commands_sync_test
    PRIVATE
        main.cpp
 )

target_link_libraries(
    commands_sync_test
    PRIVATE
        ographics::oplatform
        ographics::ogfx
)