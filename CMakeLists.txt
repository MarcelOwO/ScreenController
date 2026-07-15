cmake_minimum_required(VERSION 3.25)
project(ScreenController VERSION 2.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CTest)

include(GNUInstallDirs)

add_subdirectory(res)
add_subdirectory(external)
add_subdirectory(libs)
add_subdirectory(src)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

install(FILES packaging/owo-screen-controller.service
        DESTINATION /usr/lib/systemd/system/)

install(FILES packaging/screencontroller.env.example
        DESTINATION /usr/share/doc/screencontroller/)

install(FILES packaging/copyright
        DESTINATION /usr/share/doc/screencontroller/)

install(PROGRAMS
        scripts/screencontroller-reset-controller
        scripts/screencontroller-show-key
        DESTINATION ${CMAKE_INSTALL_SBINDIR})

set(CPACK_PACKAGE_NAME "screencontroller")
set(CPACK_PACKAGE_VENDOR "MarcelOwO")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_CONTACT "MarcelOwO")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "Secure Bluetooth-controlled fullscreen display for Raspberry Pi")
set(CPACK_PACKAGE_DESCRIPTION
    "ScreenController turns a Raspberry Pi into a dedicated fullscreen media display. It accepts one commissioned controller over encrypted Bluetooth LE L2CAP or GATT and supports authenticated media upload and display control.")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/MarcelOwO/ScreenController")

set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")

if(LINUX)
  set(CPACK_GENERATOR "DEB")
  set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
  set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
  # GLFW and EGL load these at runtime, so dpkg-shlibdeps cannot discover them
  # from ELF NEEDED entries. Keep the explicit list alongside its generated
  # FFmpeg/OpenSSL/systemd dependencies.
  set(CPACK_DEBIAN_PACKAGE_DEPENDS
      "bluez, openssl, libegl1, libgles2, libx11-6, libxcursor1, libxext6, libxfixes3, libxi6, libxinerama1, libxrandr2, libxrender1")
  set(CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION TRUE)
  set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
      "${CMAKE_SOURCE_DIR}/packaging/debian/postinst;${CMAKE_SOURCE_DIR}/packaging/debian/prerm;${CMAKE_SOURCE_DIR}/packaging/debian/postrm")
else()
  set(CPACK_GENERATOR "TGZ")
endif()

include(CPack)
