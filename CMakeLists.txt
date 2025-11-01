cmake_minimum_required(VERSION 3.25)
project(ScreenController VERSION 1.0.0 )

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

add_subdirectory(external)
add_subdirectory(source)

add_executable(ScreenController source/main.cpp)

target_link_libraries(ScreenController
PRIVATE
App common ng-log)

install(TARGETS ScreenController
        RUNTIME DESTINATION bin
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/resources/
        DESTINATION share/screencontroller/assets
)

set(CPACK_PACKAGE_NAME "screencontroller")
set(CPACK_PACKAGE_VENDOR "MarcelOwO")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Screen Controller — A C++ desktop application for display management.")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "MarcelOwO")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})

set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")

set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}_${CMAKE_SYSTEM_PROCESSOR}")

set(CPACK_GENERATOR "DEB")

include(CPack)