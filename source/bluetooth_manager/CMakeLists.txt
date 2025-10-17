add_library(Bluetooth
        bluetooth_manager.cpp
        dbus/dbus_manager.cpp
        dbus/dbus_implementations/bluetooth_device.cpp
        dbus/dbus_implementations/bluetooth_agent.cpp
        dbus/dbus_implementations/bluetooth_agent_manager.cpp
        dbus/dbus_implementations/bluetooth_le_advertisement.cpp
        dbus/dbus_implementations/bluetooth_le_advertising_manager.cpp
        dbus/dbus_implementations/bluetooth_adapter.cpp
        socket/l2cap_receiver.cpp
        unpacker/unpacker.cpp
        socket/socket_helper/socket_helper.cpp
        socket/socket_implementations/socket_options.cpp
        )

target_include_directories(Bluetooth
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/.
)

target_link_libraries(Bluetooth
        PUBLIC Logger common
        PRIVATE sdbus-c++ bluetooth  libzstd_static
)


