//
// Created by marce on 5/2/2025.
//

#include "bluetooth_device.h"

#include <iostream>
namespace screen_controller::dbus {

BluetoothDevice::BluetoothDevice(std::shared_ptr<sdbus::IConnection> connection,
                                 const sdbus::ObjectPath& device)
    : device_interface_("org.bluez.Device1") {
  device_proxy_ =
      sdbus::createProxy(*connection, sdbus::ServiceName("org.bluez"), device);
}

bool BluetoothDevice::Connect() const {
  try {
    (void)device_proxy_->callMethod("Connect").onInterface(device_interface_);
    return true;
  } catch (sdbus::Error& e) {
    std::cerr << e.what() << std::endl;
    return false;
  }
}
bool BluetoothDevice::Disconnect() const {
  try {
    (void)device_proxy_->callMethod("Disconnect")
        .onInterface(device_interface_);
    return true;
  } catch (sdbus::Error& e) {
    std::cerr << e.what() << std::endl;
    return false;
  }
}

bool BluetoothDevice::ConnectProfile(std::string_view uuid) const {
  try {
    device_proxy_->callMethod("ConnectProfile")
        .onInterface(device_interface_)
        .withArguments(uuid);
    return true;
  } catch (sdbus::Error& e) {
    std::cerr << e.what() << std::endl;
    return false;
  }
}

bool BluetoothDevice::DisconnectProfile(std::string_view uuid) const {
  try {
    device_proxy_->callMethod("DisconnectProfile")
        .onInterface(device_interface_)
        .withArguments(uuid);
    return true;
  } catch (sdbus::Error& e) {
    std::cerr << e.what() << std::endl;
    return false;
  }
}

bool BluetoothDevice::Pair() const {
  try {
    (void)device_proxy_->callMethod("Pair").onInterface(device_interface_);
    return true;
  } catch (sdbus::Error& e) {
    std::cerr << e.what() << std::endl;
    return false;
  }
}

bool BluetoothDevice::CancelPairing() const {
  try {
    (void)device_proxy_->callMethod("CancelPairing")
        .onInterface(device_interface_);
    return true;
  } catch (sdbus::Error& e) {
    std::cerr << e.what() << std::endl;
    return false;
  }
}

std::optional<std::vector<std::vector<uint8_t>>>
BluetoothDevice::GetServiceRecords() {
  try {
    std::vector<std::vector<uint8_t>> records{};
    (void)device_proxy_->callMethod("GetServiceRecords")
        .onInterface(device_interface_)
        .storeResultsTo(records);
    return records;
  } catch (sdbus::Error& e) {
    std::cerr << e.what() << std::endl;
    return std::nullopt;
  }
}

std::optional<std::string_view> BluetoothDevice::GetAddress() const {
  auto value = device_proxy_->getProperty("Address")
                   .onInterface(device_interface_)
                   .get<std::string>();
  if (value.empty()) {
    std::cerr << "Failed to get Address" << std::endl;
    return std::nullopt;
  }

  return value;
}

std::optional<std::string_view> BluetoothDevice::GetAddressType() const {
  auto value = device_proxy_->getProperty("AddressType")
                   .onInterface(device_interface_)
                   .get<std::string>();
  if (value.empty()) {
    std::cerr << "Failed to get AddressType" << std::endl;
    return std::nullopt;
  }

  return value;
}
std::optional<std::string_view> BluetoothDevice::GetName() const {
  auto value = device_proxy_->getProperty("Name")
                   .onInterface(device_interface_)
                   .get<std::string>();
  if (value.empty()) {
    std::cerr << "Failed to get AddressType" << std::endl;
    return std::nullopt;
  }

  return value;
}
std::optional<std::string_view> BluetoothDevice::GetIcon() const {
  auto value = device_proxy_->getProperty("Icon")
                   .onInterface(device_interface_)
                   .get<std::string>();
  if (value.empty()) {
    std::cerr << "Failed to get AddressType" << std::endl;
    return std::nullopt;
  }

  return value;
}

uint32_t BluetoothDevice::GetClass() const {
  auto value = device_proxy_->getProperty("Class")
                   .onInterface(device_interface_)
                   .get<uint32_t>();
  return value;
}

uint16_t BluetoothDevice::GetAppearance() {
  auto value = device_proxy_->getProperty("Appearance")
                   .onInterface(device_interface_)
                   .get<uint16_t>();
  return value;
}

// implement this later lol
std::vector<std::string> BluetoothDevice::GetUUIDs() {}
bool BluetoothDevice::GetPaired() {}
bool BluetoothDevice::GetBonded() {}
bool BluetoothDevice::GetConnected() {}
bool BluetoothDevice::GetTrusted() {}
void BluetoothDevice::SetTrusted(bool trusted) {}
bool BluetoothDevice::GetBlocked() {}
void BluetoothDevice::SetBlocked(bool blocked) {}
bool BluetoothDevice::GetWakeAllowed() {}
void BluetoothDevice::SetWakeAllowed(bool allowed) {}
std::string BluetoothDevice::GetAlias() {}
void BluetoothDevice::SetAlias(std::string_view alias) {}
bool BluetoothDevice::GetLegacyPairing() {}
bool BluetoothDevice::GetCablePairing() {}
std::string BluetoothDevice::GetModalias() {}
int16_t BluetoothDevice::GetRSSI() {}
int16_t BluetoothDevice::GetTxPower() {}
std::unordered_map<uint16_t, std::vector<uint8_t>>
BluetoothDevice::GetManufacturerData() {}
std::unordered_map<std::string_view, std::vector<uint8_t>>
BluetoothDevice::GetServiceData() {}
bool BluetoothDevice::GetServicesResolved() {}
std::vector<uint8_t> BluetoothDevice::GetAdvertisingFlags() {}
std::unordered_map<uint8_t, std::vector<uint8_t>>
BluetoothDevice::GetAdvertisingData() {}
std::unordered_map<sdbus::ObjectPath, std::vector<uint8_t>>
BluetoothDevice::GetSets() {}
std::string BluetoothDevice::GetPreferredBearer() {}
void BluetoothDevice::SetPreferredBearer(std::string bearer) {}

}  // namespace screen_controller::dbus