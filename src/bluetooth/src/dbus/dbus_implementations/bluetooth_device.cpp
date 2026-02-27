//
// Created by marce on 5/2/2025.
//

#include "bluetooth_device.h"

#include <iostream>

namespace screen_controller {
BluetoothDevice::BluetoothDevice(
    ILogger& logger, const std::shared_ptr<sdbus::IConnection>& connection,
    const sdbus::ObjectPath& device)
    : logger_(logger),
      device_proxy_(sdbus::createProxy(
          *connection, sdbus::ServiceName("org.bluez"), device)),
      device_interface_("org.bluez.Device1") {
  logger_.LogInfo("Creating bluetoothdevice");
}

std::expected<void, std::error_code> BluetoothDevice::Connect() const {
  try {
    (void)device_proxy_->callMethod("Connect").onInterface(device_interface_);
    return {};
  } catch (const sdbus::Error& e) {
    logger_.LogError(e.getMessage());
    return std::unexpected(
        std::make_error_code(std::errc::operation_not_permitted));
  }
}

std::expected<void, std::error_code> BluetoothDevice::Disconnect() const {
  try {
    (void)device_proxy_->callMethod("Disconnect")
        .onInterface(device_interface_);
    return {};
  } catch (const sdbus::Error& e) {
    logger_.LogError(e.getMessage());
    return std::unexpected(
        std::make_error_code(std::errc::operation_not_permitted));
  }
}

std::expected<void, std::error_code> BluetoothDevice::ConnectProfile(
    std::string_view uuid) const {
  try {
    device_proxy_->callMethod("ConnectProfile")
        .onInterface(device_interface_)
        .withArguments(uuid);
    return {};
  } catch (const sdbus::Error& e) {
    logger_.LogError(e.getMessage());
    return std::unexpected(
        std::make_error_code(std::errc::operation_not_permitted));
  }
}

std::expected<void, std::error_code> BluetoothDevice::DisconnectProfile(
    std::string_view uuid) const {
  try {
    device_proxy_->callMethod("DisconnectProfile")
        .onInterface(device_interface_)
        .withArguments(uuid);
    return {};
  } catch (const sdbus::Error& e) {
    logger_.LogError(e.getMessage());
    return std::unexpected(
        std::make_error_code(std::errc::operation_not_permitted));
  }
}

std::expected<void, std::error_code> BluetoothDevice::Pair() const {
  try {
    (void)device_proxy_->callMethod("Pair").onInterface(device_interface_);
    return {};
  } catch (const sdbus::Error& e) {
    logger_.LogError(e.getMessage());
    return std::unexpected(
        std::make_error_code(std::errc::operation_not_permitted));
  }
}

std::expected<void, std::error_code> BluetoothDevice::CancelPairing() const {
  try {
    (void)device_proxy_->callMethod("CancelPairing")
        .onInterface(device_interface_);
    return {};
  } catch (const sdbus::Error& e) {
    logger_.LogError(e.getMessage());
    return std::unexpected(
        std::make_error_code(std::errc::operation_not_permitted));
  }
}

std::optional<std::vector<std::vector<uint8_t>>>
BluetoothDevice::GetServiceRecords() const {
  try {
    std::vector<std::vector<uint8_t>> records{};

    device_proxy_->callMethod("GetServiceRecords")
        .onInterface(device_interface_)
        .storeResultsTo(records);

    return records;
  } catch (const sdbus::Error& e) {
    logger_.LogError(e.getMessage());
    return std::nullopt;
  }
}

std::optional<std::string_view> BluetoothDevice::GetAddress() const {
  auto value = device_proxy_->getProperty("Address")
                   .onInterface(device_interface_)
                   .get<std::string>();
  if (value.empty()) {
    logger_.LogError("Failed to get Address");
    return std::nullopt;
  }

  return value;
}

std::optional<std::string_view> BluetoothDevice::GetAddressType() const {
  auto value = device_proxy_->getProperty("AddressType")
                   .onInterface(device_interface_)
                   .get<std::string>();
  if (value.empty()) {
    logger_.LogError("Failed to get adress type");
    return std::nullopt;
  }

  return value;
}

std::optional<std::string> BluetoothDevice::GetName() const {
  auto value = device_proxy_->getProperty("Name")
                   .onInterface(device_interface_)
                   .get<std::string>();
  if (value.empty()) {
    logger_.LogError("Failed to get name");
    return std::nullopt;
  }

  return value;
}

std::optional<std::string_view> BluetoothDevice::GetIcon() const {
  auto value = device_proxy_->getProperty("Icon")
                   .onInterface(device_interface_)
                   .get<std::string>();
  if (value.empty()) {
    logger_.LogError("Failed to get Icon");
    return std::nullopt;
  }

  return value;
}

uint32_t BluetoothDevice::GetClass() const {
  const auto value = device_proxy_->getProperty("Class")
                         .onInterface(device_interface_)
                         .get<uint32_t>();
  return value;
}

uint16_t BluetoothDevice::GetAppearance() const {
  const auto value = device_proxy_->getProperty("Appearance")
                         .onInterface(device_interface_)
                         .get<uint16_t>();
  return value;
}
}  // namespace screen_controller
