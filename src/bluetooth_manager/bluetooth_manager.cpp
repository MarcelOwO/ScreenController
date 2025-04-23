// Created by marce on 4/2/2025.
//

#include "bluetooth_manager.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <sys/socket.h>

BluetoothManager::BluetoothManager() :
    adapter_interface_name_(sdbus::InterfaceName("org.bluez.Adapter1")),
    adv_manager_interface_name_(sdbus::InterfaceName(
        "org.bluez.LEAdvertisingManager1")),
    gatt_mgr_interface_name_(
        sdbus::InterfaceName("org.bluez.GattManager1")),
    gatt_object_path_(
        sdbus::ObjectPath("/org/bluez/hci0/owo/service0001")),
    adapter_path_(sdbus::ObjectPath("/org/bluez/hci0")),
    app_path_(sdbus::ObjectPath("/org/bluez/hci0/owo")),
    service_path_(sdbus::ObjectPath("/org/bluez/hci0/owo/service0001")),
    char_path_(
        sdbus::ObjectPath("/org/bluez/hci0/owo/service0001/char0001")) {
    connection_ = sdbus::createSystemBusConnection();
}

void BluetoothManager::on_file_received(FileDataCallback callback) {
    file_data_callback_ = std::move(callback);
}

void BluetoothManager::on_command_received(CommandCallback callback) {
    command_callback_ = std::move(callback);
}


void BluetoothManager::init() {
    try {
        const auto adapter_proxy = sdbus::createProxy(*connection_, sdbus::ServiceName("org.bluez"),
                                                      adapter_path_);

        adapter_proxy->setProperty("Alias")
                     .onInterface(adapter_interface_name_)
                     .toValue("ScreenController");

        adapter_proxy->setProperty("Powered")
                     .onInterface(adapter_interface_name_)
                     .toValue(true);

        adapter_proxy->setProperty("Discoverable")
                     .onInterface(adapter_interface_name_)
                     .toValue(true);

        adapter_proxy->setProperty("Pairable")
                     .onInterface(adapter_interface_name_)
                     .toValue(true);

        const auto app_object = sdbus::createObject(*connection_, app_path_);


        app_object->addVTable(
                      sdbus::registerMethod("Release")
                      .withNoReply()
                      .implementedAs([=]() { std::cout << "Released" << std::endl; }),
                      sdbus::registerProperty("Type")
                      .withGetter([=]() { return std::string("peripheral"); }),
                      sdbus::registerProperty("ServiceUUIDs")
                      .withGetter([=]() {
                              return std::vector<std::string>{
                                  std::string(
                                      "69696969-6969-6969-6969-696969696969")
                              };
                          }
                      ),
                      sdbus::registerProperty("Discoverable")
                      .withGetter([=]() { return true; }))
                  .forInterface(sdbus::InterfaceName(
                      "org.bluez.LEAdvertisement1"));

        connection_->enterEventLoopAsync();

        adapter_proxy->callMethod("RegisterAdvertisement")
                     .onInterface(adv_manager_interface_name_)
                     .withArguments(app_path_, std::map<std::string, sdbus::Variant>{});
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}


void BluetoothManager::open_socket() {
    const int l2_cap_socket = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);

    if (l2_cap_socket < 0) {
        throw std::runtime_error("Failed to open l2cap socket");
        return;
    }

    sockaddr_l2 loc_addr = {};
    loc_addr.l2_family = AF_BLUETOOTH;
    loc_addr.l2_psm = 0x1010; // PSM
    loc_addr.l2_cid = 0;

    memset(&loc_addr.l2_bdaddr, 0, sizeof(loc_addr.l2_bdaddr));

    if (bind(l2_cap_socket, reinterpret_cast<sockaddr*>(&loc_addr), sizeof(loc_addr)) < 0) {
        (void)close(l2_cap_socket);
        throw std::runtime_error("Failed to bind l2cap socket");
        return;
    }

    if (listen(l2_cap_socket, 1) < 0) {
        (void)close(l2_cap_socket);
        throw std::runtime_error("Failed to listen on l2cap socket");
        return;
    }

    sockaddr_l2 rem_addr = {};
    socklen_t opt = sizeof(rem_addr);

    std::cout << "Waiting for connection..." << std::endl;

    const int client_socket = accept(l2_cap_socket, reinterpret_cast<sockaddr*>(&rem_addr),
                                     &opt);
    if (client_socket < 0) {
        (void)close(l2_cap_socket);
        throw std::runtime_error("Failed to accept connection");
        return;
    }

    std::array<char, 18> buffer{};
    (void)ba2str(&rem_addr.l2_bdaddr, buffer.data());
    std::cout << "Accepted connection from " << buffer.data() << std::endl;
    (void)close(l2_cap_socket);

    constexpr int buffer_size = 1024;
    std::array<char, buffer_size> file_buffer{};

    try {
        std::ofstream received_file("received_file.txt", std::ios::binary);

        if (!received_file.is_open()) {
            throw std::runtime_error("Failed to open received file");
        }

        ssize_t bytes_read{};
        while ((bytes_read = recv(client_socket, file_buffer.data(), buffer_size, 0))) {
            received_file.write(file_buffer.data(), bytes_read);
        }

        if (bytes_read < 0) {
            throw std::runtime_error("Failed to read received file");
        }

        std::cout << "File received." << std::endl;

        received_file.close();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        close(l2_cap_socket);
        return;
    }
}

BluetoothManager::~BluetoothManager() {
}
