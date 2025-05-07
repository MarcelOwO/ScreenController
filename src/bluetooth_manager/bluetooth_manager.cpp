// Created by marce on 4/2/2025.
//

#include "bluetooth_manager.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <sys/socket.h>

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "dbus_implementations/bluetooth_adapter.h"
#include "dbus_implementations/bluetooth_le_advertising_manager.h"

namespace screen_controller {

BluetoothManager::BluetoothManager()
    : adapter_interface_name_(sdbus::InterfaceName("org.bluez.Adapter1")),
      adv_manager_interface_name_(
          sdbus::InterfaceName("org.bluez.LEAdvertisingManager1")),
      gatt_mgr_interface_name_(sdbus::InterfaceName("org.bluez.GattManager1")),
      agent_mgr_iface_(sdbus::InterfaceName("org.bluez.AgentManager1")),
      gatt_object_path_(sdbus::ObjectPath("/org/bluez/hci0/owo/service0001")),
      adapter_path_(sdbus::ObjectPath("/org/bluez/hci0")),
      advertisement_path_(sdbus::ObjectPath("/org/bluez/hci0/owo")),
      service_path_(sdbus::ObjectPath("/org/bluez/hci0/owo/service0001")),
      char_path_(sdbus::ObjectPath("/org/bluez/hci0/owo/service0001/char0001")),
      agent_path_(sdbus::ObjectPath("/org/bluez/hci0/owo/agent1")) {}

BluetoothManager::~BluetoothManager() = default;
void BluetoothManager::on_file_received(file_data_callback callback) {
  file_data_callback_ = std::move(callback);
}

void BluetoothManager::on_command_received(command_callback callback) {
  command_callback_ = std::move(callback);
}

bool BluetoothManager::init() {
  connection_ = sdbus::createSystemBusConnection();

  if (!connection_) {
    std::cerr << "Failed to connect to system bus." << std::endl;
    return false;
  }

  adapter_proxy_ = sdbus::createProxy(
      *connection_, sdbus::ServiceName("org.bluez"), adapter_path_);

  if (!adapter_proxy_) {
    std::cerr << "Failed to create adapter proxy" << std::endl;
    return false;
  }

  if (!setup_adapter()) {
    std::cerr << "Failed to setup adapter" << std::endl;
    return false;
  }

  if (!register_agent()) {
    std::cerr << "Failed to register adapter." << std::endl;
    return false;
  }

  if (!register_advertisement()) {
    std::cerr << "Failed to register advertisement." << std::endl;
    return false;
  };
  try {
    std::string capabilities = "NoInputNoOutput";

    const auto bluez_proxy =
        sdbus::createProxy(*connection_, sdbus::ServiceName("org.bluez"),
                           sdbus::ObjectPath("/org/bluez"));

    (void)bluez_proxy->callMethod(sdbus::MethodName("RegisterAgent"))
        .onInterface(agent_mgr_iface_)
        .withArguments(agent_path_, capabilities);

    (void)bluez_proxy->callMethod(sdbus::MethodName("RequestDefaultAgent"))
        .onInterface(agent_mgr_iface_)
        .withArguments(agent_path_);

    dbus::BluetoothLeAdvertisingManager bluetooth_le_advertising_manager(
        adapter_proxy_);

    std::unordered_map<std::string, sdbus::Variant> options{};

    if (!bluetooth_le_advertising_manager.RegisterAdvertisement(
            advertisement_path_, options)) {
      std::cerr << "Failed to register advertisement." << std::endl;
    }

  } catch (sdbus::Error& e) {
    std::cerr << e.what() << std::endl;
    return false;
  }

  connection_->enterEventLoopAsync();

  return true;
}

bool BluetoothManager::register_advertisement() {
  const auto advertisement_obj =
      sdbus::createObject(*connection_, advertisement_path_);

  if (!advertisement_obj) {
    return false;
  }

  advertisement_obj
      ->addVTable(sdbus::registerMethod("Release").withNoReply().implementedAs(
                      [=] { std::cout << "Released" << std::endl; }),
                  sdbus::registerProperty("Type").withGetter(
                      [=] { return std::string("peripheral"); }),
                  sdbus::registerProperty("ServiceUUIDs").withGetter([=] {
                    return std::vector<std::string>{
                        std::string("69696969-6969-6969-6969-696969696969")};
                  }))
      .forInterface(sdbus::InterfaceName("org.bluez.LEAdvertisement1"));

  return true;
}
void BluetoothManager::open_socket() {
  const int l2_cap_socket = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);

  if (l2_cap_socket < 0) {
    throw std::runtime_error("Failed to open l2cap socket");
    return;
  }

  sockaddr_l2 loc_addr = {};

  loc_addr.l2_family = AF_BLUETOOTH;
  loc_addr.l2_psm = 0x1010;
  loc_addr.l2_cid = 0;

  (void)memset(&loc_addr.l2_bdaddr, 0, sizeof(loc_addr.l2_bdaddr));

  if (bind(l2_cap_socket, reinterpret_cast<sockaddr*>(&loc_addr),
           sizeof(loc_addr)) < 0) {
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

  const int client_socket =
      accept(l2_cap_socket, reinterpret_cast<sockaddr*>(&rem_addr), &opt);
  if (client_socket < 0) {
    (void)close(l2_cap_socket);
    throw std::runtime_error("Failed to accept connection");
    return;
  }

  std::array<char, 18> buffer{};
  (void)ba2str(&rem_addr.l2_bdaddr, buffer.data());
  (void)close(l2_cap_socket);
}
bool BluetoothManager::setup_adapter() {
  dbus::BluetoothAdapter adapter(adapter_proxy_);
  if (!adapter.SetAlias("ScreenController")) {
    return false;
  };

  if (!adapter.SetDiscoverable(true)) {
    return false;
  }
  if (!adapter.SetPowered(true)) {
    return false;
  }
  if (!adapter.SetPairable(true)) {
    return false;
  }

  return true;
}
void BluetoothManager::unregister_agent() {
  const auto agent_mgr_iface = sdbus::InterfaceName("org.bluez.AgentManager1");

  adapter_proxy_->callMethod("UnregisterAgent")
      .onInterface(agent_mgr_iface)
      .withArguments(sdbus::ObjectPath("/org/bluez/hci0/owo/agent"));
  connection_->leaveEventLoop();
}

bool BluetoothManager::register_agent() {
  const auto agent_object = sdbus::createObject(*connection_, agent_path_);

  if (!agent_object) {
    std::cerr << "Failed to create agent object" << std::endl;
    return false;
  }

  agent_object
      ->addVTable(
          sdbus::registerMethod("Release").withNoReply().implementedAs(
              []() { std::cout << "Agent released" << std::endl; }),
          sdbus::registerMethod("RequestPinCode")
              .implementedAs(
                  [](sdbus::ObjectPath device) { return std::string("0000"); }),
          sdbus::registerMethod("DisplayPinCode")
              .implementedAs([](sdbus::ObjectPath device, std::string pincode) {
                std::cout << pincode << std::endl;
              }),
          sdbus::registerMethod("RequestPasskey")
              .implementedAs([](sdbus::ObjectPath device) { return 1234; }),
          sdbus::registerMethod("DisplayPasskey")
              .implementedAs(
                  [](sdbus::ObjectPath device, uint32_t passkey,
                     uint16_t entered) { std::cout << entered << std::endl; }),

          sdbus::registerMethod("RequestConfirmation")
              .implementedAs([](sdbus::ObjectPath device, uint32_t passkey) {
                std::cout << "RequestConfirmation" << std::endl;
              }),
          sdbus::registerMethod("RequestAuthorization")
              .implementedAs([](sdbus::ObjectPath device) {
                std::cout << "RequestAuthorization" << std::endl;
              }),
          sdbus::registerMethod("AuthorizeService")
              .implementedAs([](sdbus::ObjectPath device, std::string uuid) {
                std::cout << "AuthorizeService" << std::endl;
              }),
          sdbus::registerMethod("Cancel").implementedAs(
              []() { std::cout << "Cancel" << std::endl; })

              )
      .forInterface(sdbus::InterfaceName("org.bluez.Agent1"));
  return true;
}

}  // namespace screen_controller
