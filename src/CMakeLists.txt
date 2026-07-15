add_subdirectory(app)
add_subdirectory(bluetooth)
add_subdirectory(graphics)
add_subdirectory(logging)
add_subdirectory(processor)
add_subdirectory(storage)
add_subdirectory(window)
add_subdirectory(events)

add_executable(ScreenController main.cpp)
set_target_properties(ScreenController PROPERTIES OUTPUT_NAME screen_controller)

target_link_libraries(ScreenController PRIVATE App common)

add_dependencies(ScreenController copy_assets)

install(DIRECTORY
        "${CMAKE_SOURCE_DIR}/res/shader_files"
        "${CMAKE_SOURCE_DIR}/res/startup_files"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/screencontroller/assets")

install(TARGETS ScreenController RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
