cmake_minimum_required(VERSION 3.25)
project(ScreenController VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(GNUInstallDirs)

add_subdirectory(res)
add_subdirectory(external)
add_subdirectory(libs)
add_subdirectory(src)

install(FILES packaging/com.owo.screen_controller.conf
        DESTINATION /usr/share/dbus-1/system.d/)

install(FILES packaging/owo-screen-controller.service
        DESTINATION /usr/lib/systemd/system/)

set(CPACK_PACKAGE_NAME "screencontroller")
set(CPACK_PACKAGE_VENDOR "MarcelOwO")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_CONTACT "MarcelOwO")

set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

include(CPack)
