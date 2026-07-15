if(LINUX)

find_package(OpenSSL 3 REQUIRED COMPONENTS Crypto)

add_library(
  Bluetooth
  src/linux/bluetooth_manager.cpp
  src/linux/bluetooth_factory.cpp
  src/linux/dbus/dbus_manager.cpp
  src/linux/dbus/dbus_implementations/bluetooth_device.cpp
  src/linux/dbus/dbus_implementations/bluetooth_agent.cpp
  src/linux/dbus/dbus_implementations/bluetooth_agent_manager.cpp
  src/linux/dbus/dbus_implementations/bluetooth_le_advertisement.cpp
  src/linux/dbus/dbus_implementations/bluetooth_le_advertising_manager.cpp
  src/linux/dbus/dbus_implementations/bluetooth_adapter.cpp
  src/linux/dbus/dbus_implementations/gatt_application.cpp
  src/linux/dbus/dbus_implementations/object_manager.cpp
  src/linux/unpacker/unpacker.cpp
  src/linux/socket/l2cap_receiver.cpp
  src/linux/socket/socket_helper.cpp
  src/linux/socket/socket_options.cpp
  )

target_include_directories(Bluetooth PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(
  Bluetooth
  PUBLIC Logger common Events
  PRIVATE sdbus-c++ bluetooth libzstd_static OpenSSL::Crypto)
else()

  add_library(Bluetooth src/dummy/dummy_mgr.cpp src/dummy/dummy_factory.cpp )

  target_include_directories(Bluetooth PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

  target_link_libraries(
    Bluetooth
    PUBLIC Logger common Events
  )


endif()
