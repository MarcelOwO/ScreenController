add_subdirectory(app)
<<<<<<< HEAD:source/CMakeLists.txt
add_subdirectory(bluetooth_manager)
add_subdirectory(common)
add_subdirectory(file_processor)
add_subdirectory(graphics_renderer)
add_subdirectory(logging)
add_subdirectory(storage_manager)
add_subdirectory(window_manager)

=======
add_subdirectory(bluetooth)
add_subdirectory(graphics)
add_subdirectory(logging)
add_subdirectory(processor)
add_subdirectory(storage)
add_subdirectory(window)
add_subdirectory(events)

add_executable(ScreenController main.cpp)

target_link_libraries(ScreenController PRIVATE App common)

add_dependencies(ScreenController copy_assets)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/res/"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/screencontroller/assets")

install(TARGETS ScreenController RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
>>>>>>> origin/dev:src/CMakeLists.txt
