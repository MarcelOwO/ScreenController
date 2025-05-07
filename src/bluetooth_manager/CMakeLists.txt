add_library(Bluetooth STATIC bluetooth_manager.cpp bluetooth_manager.h
        dbus_implementations/bluetooth_device.cpp
        dbus_implementations/bluetooth_device.h
        dbus_implementations/bluetooth_agent.cpp
        dbus_implementations/bluetooth_agent.h
        dbus_implementations/bluetooth_agent_manager.cpp
        dbus_implementations/bluetooth_agent_manager.h
        dbus_implementations/bluetooth_le_advertisement.cpp
        dbus_implementations/bluetooth_le_advertisement.h
        dbus_implementations/bluetooth_le_advertising_manager.cpp
        dbus_implementations/bluetooth_le_advertising_manager.h
        dbus_implementations/bluetooth_adapter.cpp
        dbus_implementations/bluetooth_adapter.h)

target_include_directories(Bluetooth PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(Bluetooth PUBLIC sdbus-c++ bluetooth)


