//
// Created by marce on 4/2/2025.
//

#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

#include <chrono>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <sdbus-c++/sdbus-c++.h>
#include <sys/poll.h>
#include <sys/socket.h>

class BluetoothManager {
public:
    BluetoothManager();
    ~BluetoothManager();

    void init();
    void open_socket();

private:
    sdbus::ServiceName bluez_service_;
    sdbus::InterfaceName le_adv_manager_iface_;
    sdbus::InterfaceName le_adv_iface_;
    sdbus::InterfaceName adapter_iface_;

    sdbus::ObjectPath device0_path_;
    sdbus::ObjectPath adv_object_path_;
    sdbus::ObjectPath app_object_path_;

    std::string my_service_uuid_;
    std::string my_local_name_;
    std::string adv_type_;

    std::unique_ptr<sdbus::IProxy> adv_manager_proxy_;
    std::unique_ptr<sdbus::IObject> advertisement_object_;

    std::shared_ptr<sdbus::IConnection> connection_;

};


#endif //BLUETOOTH_MANAGER_H
