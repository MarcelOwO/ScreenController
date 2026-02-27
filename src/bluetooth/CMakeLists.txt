add_library(
  Bluetooth
  src/bluetooth_manager.cpp
  src/bluetooth_factory.cpp
  src/dbus/dbus_manager.cpp
  src/dbus/dbus_implementations/bluetooth_device.cpp
  src/dbus/dbus_implementations/bluetooth_agent.cpp
  src/dbus/dbus_implementations/bluetooth_agent_manager.cpp
  src/dbus/dbus_implementations/bluetooth_le_advertisement.cpp
  src/dbus/dbus_implementations/bluetooth_le_advertising_manager.cpp
  src/dbus/dbus_implementations/bluetooth_adapter.cpp
  src/socket/l2cap_receiver.cpp
  src/unpacker/unpacker.cpp
  src/socket/socket_helper/socket_helper.cpp
  src/socket/socket_implementations/socket_options.cpp
  src/models/connection_state.h)

target_include_directories(Bluetooth PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(
  Bluetooth
  PUBLIC Logger common
  PRIVATE sdbus-c++ bluetooth libzstd_static)
