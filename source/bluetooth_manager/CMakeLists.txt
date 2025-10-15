add_library(Bluetooth
        bluetooth_manager.cpp
        bluetooth_manager.h
        dbus/dbus_manager.cpp
        dbus/dbus_manager.h
        dbus/dbus_implementations/bluetooth_device.cpp
        dbus/dbus_implementations/bluetooth_device.h
        dbus/dbus_implementations/bluetooth_agent.cpp
        dbus/dbus_implementations/bluetooth_agent.h
        dbus/dbus_implementations/bluetooth_agent_manager.cpp
        dbus/dbus_implementations/bluetooth_agent_manager.h
        dbus/dbus_implementations/bluetooth_le_advertisement.cpp
        dbus/dbus_implementations/bluetooth_le_advertisement.h
        dbus/dbus_implementations/bluetooth_le_advertising_manager.cpp
        dbus/dbus_implementations/bluetooth_le_advertising_manager.h
        dbus/dbus_implementations/bluetooth_adapter.cpp
        dbus/dbus_implementations/bluetooth_adapter.h
        socket/l2cap_receiver.cpp
        socket/l2cap_receiver.h
        unpacker/unpacker.cpp
        unpacker/unpacker.h
        models/packet.h
        socket/socket_helper/socket_helper.cpp
        socket/socket_helper/socket_helper.h
        socket/socket_implementations/socket_options.cpp
        socket/socket_implementations/socket_options.h)


target_include_directories(Bluetooth PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Bluetooth PRIVATE sdbus-c++ bluetooth ng-log libzstd_static)


