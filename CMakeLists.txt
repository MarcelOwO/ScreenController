cmake_minimum_required(VERSION 3.25)

project(ScreenController VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 23)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(ScreenController src/main.cpp)

add_subdirectory(external)
add_subdirectory(include)
add_subdirectory(src)

set(RES_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res")

file(
  GLOB ALL_FILES
  RELATIVE "${RES_SRC_DIR}"
  "${RES_SRC_DIR}/*")

foreach(ITEM ${ALL_FILES})
  if(IS_DIRECTORY "${RES_SRC_DIR}/${ITEM}")
    add_custom_command(
      TARGET ScreenController
      POST_BUILD
      # This command prints to the terminal during the build phase
      COMMAND ${CMAKE_COMMAND} -E echo
              "Build step: Copying directory ${ITEM} to assets..."
      COMMAND ${CMAKE_COMMAND} -E make_directory
              "$<TARGET_FILE_DIR:ScreenController>/assets"
      COMMAND ${CMAKE_COMMAND} -E copy_directory "${RES_SRC_DIR}/${ITEM}"
              "$<TARGET_FILE_DIR:ScreenController>/assets/${ITEM}"
      # This shows up as a header in some IDEs/build tools
      COMMENT "Syncing ${ITEM} to build assets folder"
      VERBATIM)
  endif()
endforeach()

target_link_libraries(ScreenController PRIVATE App common)

include(GNUInstallDirs)

install(TARGETS ScreenController RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY ${CMAKE_SOURCE_DIR}/res/
        DESTINATION ${CMAKE_INSTALL_DATADIR}/screencontroller/assets)

if(UNIX AND NOT APPLE)
  cmake_host_system_information(RESULT DISTRO_ID QUERY DISTRIB_ID)

  find_program(DPKG_PROGRAM dpkg)
  if(DPKG_PROGRAM AND NOT (DISTRO_ID STREQUAL "arch"))

    set(CPACK_PACKAGE_NAME "screencontroller")
    set(CPACK_PACKAGE_VENDOR "MarcelOwO")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
        "Screen Controller — A C++ desktop application for display management.")
    set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")

    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "MarcelOwO")
    set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})

    set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")

    set(CPACK_PACKAGE_FILE_NAME
        "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}_${CMAKE_SYSTEM_PROCESSOR}"
    )

    set(CPACK_GENERATOR "DEB")

    include(CPack)
  endif()
endif()
