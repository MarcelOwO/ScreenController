//
// Created by marce on 4/2/2025.
//

#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H
#include <functional>
#include <string>
#include <vector>

#include <sdbus-c++/sdbus-c++.h>

class BluetoothManager {
public:
    BluetoothManager();
    ~BluetoothManager();

    void init();
    void run();
    void cleanup();

    using FileDataCallback = std::function<void(std::string file_name,
                                                std::vector<std::byte> file_data)>;
    using CommandCallback = std::function<void(std::string command_name,
                                               std::vector<std::byte> data_data)>;

    void on_file_received(FileDataCallback callback);
    void on_command_received(CommandCallback callback);

    void open_socket();

private:
    std::unique_ptr<sdbus::IConnection> connection_ = sdbus::createSystemBusConnection();

    sdbus::InterfaceName adapter_interface_name_;
    sdbus::InterfaceName adv_manager_interface_name_;
    sdbus::InterfaceName gatt_mgr_interface_name_;

    sdbus::ObjectPath gatt_object_path_;
    sdbus::ObjectPath adapter_path_;
    sdbus::ObjectPath app_path_;
    sdbus::ObjectPath service_path_;
    sdbus::ObjectPath char_path_;


    FileDataCallback file_data_callback_;
    CommandCallback command_callback_;
};


#endif //BLUETOOTH_MANAGER_H
