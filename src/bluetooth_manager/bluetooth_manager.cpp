// Created by marce on 4/2/2025.
//

#include "bluetooth_manager.h"

#include "../../../external/bluez-dbus-cpp/example/SerialCharacteristic.h"

BluetoothManager::BluetoothManager() {
    bluez_service_ = sdbus::ServiceName("org.bluez");
    device0_path_ = sdbus::ObjectPath("/org/bluez/hci0");
    adapter_iface_ = sdbus::InterfaceName("org.bluez.Adapter1");
    le_adv_manager_iface_ = sdbus::InterfaceName("org.bluez.LEAdvertisingManager1");
    le_adv_iface_ = sdbus::InterfaceName("org.bluez.LEAdvertisement1");
    app_object_path_ = sdbus::ObjectPath("/org/bluez/owo");
    adv_object_path_ = sdbus::ObjectPath("/org/bluez/owo/adv");
    adv_type_ = "peripheral";
    my_local_name_ = "Backpack";
    my_service_uuid_ = "69696969-6969-6969-6969-696969696969";
    connection_ = nullptr;
    advertisement_object_ = nullptr;
}

#include <bluez-dbus-cpp/bluez.h>
#include <bluez-dbus-cpp/GenericCharacteristic.h>
#include <bluez-dbus-cpp/ReadOnlyCharacteristic.h>

#include <iostream>
#include <signal.h>

void BluetoothManager::init() {
    connection_ = std::move(sdbus::createSystemBusConnection());
    {
        org::bluez::Adapter1 adapter1{*connection_, bluez_service_, device0_path_};

        adapter1.Powered(true);
        adapter1.Discoverable(true);
        adapter1.Pairable(true);
        adapter1.Alias(my_local_name_);

        std::cout << "Found adapter '" << device0_path_ << "'" << std::endl;
        std::cout << "  Name: " << adapter1.Name() << std::endl;
        std::cout << "  Address: " << adapter1.Address() << " type: " << adapter1.AddressType() <<
        std::endl;
        std::cout << "  Powered: " << adapter1.Powered() << std::endl;
        std::cout << "  Discoverable: " << adapter1.Discoverable() << std::endl;
        std::cout << "  Pairable: " << adapter1.Pairable() << std::endl;
    }

    // ---- Services ---------------------------------------------------------------------------------------------------

    org::bluez::GattManager1 gattMgr{connection_, bluez_service_, device0_path_};

    auto app = std::make_shared<org::bluez::GattApplication1>(connection_, app_object_path_);

    auto srv1 = std::make_shared<org::bluez::GattService1>(app, "deviceinfo", "180A");

    org::bluez::ReadOnlyCharacteristic::createFinal(srv1, "2A24", my_local_name_); // model name
    org::bluez::ReadOnlyCharacteristic::createFinal(srv1, "2A25", "333-12345678-888");
    // serial number
    org::bluez::ReadOnlyCharacteristic::createFinal(srv1, "2A26", "1.0.1"); // fw rev
    org::bluez::ReadOnlyCharacteristic::createFinal(srv1, "2A27", "rev A"); // hw rev
    org::bluez::ReadOnlyCharacteristic::createFinal(srv1, "2A28", "5.0"); // sw rev
    org::bluez::ReadOnlyCharacteristic::createFinal(srv1, "2A29", "ACME Inc."); // manufacturer

    auto srv2 = std::make_shared<org::bluez::GattService1>(app, "serial",
                                                           "368a3edf-514e-4f70-ba8f-2d0a5a62bc8c");
    org::bluez::SerialCharacteristic::create(srv2, connection_, "de0a7b0c-358f-4cef-b778-8fe9abf09d53")
    .finalize();

    auto register_app_callback = [](const sdbus::Error* error) {
        if (error == nullptr) {
            std::cout << "Application registered." << std::endl;
        }
        else {
            std::cerr << "Error registering application " << error->getName() << " with message " <<
            error->getMessage() << std::endl;
        }
    };

    gattMgr.RegisterApplicationAsync(sdbus::ObjectPath(app->getPath()), {})
           .uponReplyInvoke(register_app_callback);



    auto mgr = std::make_shared<org::bluez::LEAdvertisingManager1>(
        *connection_, bluez_service_, device0_path_);
    std::cout << "LEAdvertisingManager1" << std::endl;
    std::cout << "  ActiveInstances: " << mgr->ActiveInstances() << std::endl;
    std::cout << "  SupportedInstances: " << mgr->SupportedInstances() << std::endl;
    {
        std::cout << "  SupportedIncludes: ";
        auto includes = mgr->SupportedIncludes();
        for (auto include : includes) {
            std::cout << "\"" << include << "\",";
        }
        std::cout << std::endl;
    }

    auto register_adv_callback = [](const sdbus::Error* error) -> void {
        if (error == nullptr) {
            std::cout << "Advertisement registered." << std::endl;
        }
        else {
            std::cerr << "Error registering advertisment " << error->getName() << " with message "
            << error->getMessage() << std::endl;
        }
    };

    connection_->enterEventLoopAsync();

    try {
        open_socket();
    }
    catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}

#include <fstream>
#include <iostream>
#include <stdexcept>

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

    const int client_socket = accept(l2_cap_socket, reinterpret_cast<sockaddr*>(&rem_addr), &opt);
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
    adv_manager_proxy_->callMethod("UnregisterAdvertisement")
                      .onInterface(le_adv_manager_iface_)
                      .withArguments(adv_object_path_);
}
