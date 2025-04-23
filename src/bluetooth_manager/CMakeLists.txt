add_library(Bluetooth STATIC bluetooth_manager.cpp bluetooth_manager.h)

include_directories(${GLIB_INCLUDE_DIRS})

target_include_directories(Bluetooth PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Bluetooth PUBLIC sdbus-c++ bluetooth dbus_gatt)


