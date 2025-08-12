cmake_minimum_required(VERSION 3.25)
project(ScreenController VERSION 1.0.0 )

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

add_subdirectory(external)
add_subdirectory(src/common)
add_subdirectory(src/app)
add_subdirectory(src/window_manager)
add_subdirectory(src/graphics_renderer)
add_subdirectory(src/bluetooth_manager)
add_subdirectory(src/storage_manager)
add_subdirectory(src/file_processor)
add_subdirectory(src/updater)

add_executable(ScreenController src/main.cpp)

target_link_libraries(ScreenController PRIVATE App common ng-log)

install(TARGETS ScreenController RUNTIME DESTINATION bin)
install (DIRECTORY "${CMAKE_SOURCE_DIR}/resources/"
DESTINATION "share/${PROJECT_NAME}"
FILES_MATCHING PATTERN "*")

configure_file(
        "${CMAKE_SOURCE_DIR}/packaging/screen_controller.service.in"
        "${CMAKE_CURRENT_BINARY_DIR}/screen_controller.service"
        @ONLY
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/screen_controller.service"
DESTINATION lib/systemd/system)

install(CODE "execute_process(COMMAND systemctl enable screen_controller.service")

include(CPack)

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "screen_controller")
set(CPACK_PACKAGE_VENDOR "MarcelOwO")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "MarcelOwO")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "armhf")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "bluetooth (>= 5.55)")


add_custom_command(
        TARGET ScreenController POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/resources"
        "${CMAKE_BINARY_DIR}/assets"
)