//
// Created by marce on 5/3/2025.
//

#include "bluetooth_le_advertising_manager.h"

#include <iostream>
namespace screen_controller::dbus {
BluetoothLeAdvertisingManager::BluetoothLeAdvertisingManager(
    const std::shared_ptr<sdbus::IProxy>& adapter_proxy)
    : adapter_proxy_(adapter_proxy),
      adv_manager_interface_name(
          sdbus::InterfaceName("org.bluez.LEAdvertisingManager1")) {}

BluetoothLeAdvertisingManager::~BluetoothLeAdvertisingManager() = default;

bool BluetoothLeAdvertisingManager::RegisterAdvertisement(
    sdbus::ObjectPath advertisement,
    std::unordered_map<std::string, sdbus::Variant> options) const {
  try {
    (void)adapter_proxy_->callMethod(sdbus::MethodName("RegisterAdvertisement"))
        .onInterface(adv_manager_interface_name)
        .withArguments(advertisement, options)
        .dontExpectReply();
    return true;
  } catch (sdbus::Error& e) {
    std::cerr << e.what() << std::endl;
    return false;
  }
}

bool BluetoothLeAdvertisingManager::UnregisterAdvertisement() const {
  try {
    (void)adapter_proxy_
        ->callMethod(sdbus::MethodName("UnregisterAdvertisement"))
        .onInterface(adv_manager_interface_name);
    return true;
  } catch (sdbus::Error& e) {
    std::cerr << e.what() << std::endl;
    return false;
  }
}

std::optional<uint8_t> BluetoothLeAdvertisingManager::GetActiveInstances()
    const {
  try {
    const auto value = adapter_proxy_->getProperty("ActiveInstances")
                           .onInterface(adv_manager_interface_name)
                           .get<uint8_t>();
    return value;
  } catch (sdbus::Error& e) {
    std::cerr << e.what() << std::endl;
    return std::nullopt;
  }
}

std::optional<uint8_t> BluetoothLeAdvertisingManager::GetSupportedInstances()
    const {
  try {
    const auto value = adapter_proxy_->getProperty("SupportedInstances")
                           .onInterface(adv_manager_interface_name)
                           .get<uint8_t>();
    return value;
  } catch (sdbus::Error& e) {
    std::cerr << e.what() << std::endl;
    return std::nullopt;
  }
}

std::optional<std::vector<std::string>>
BluetoothLeAdvertisingManager::GetSupportedIncludes() const {
  try {
    const auto value = adapter_proxy_->getProperty("SupportedIncludes")
                           .onInterface(adv_manager_interface_name)
                           .get<std::vector<std::string>>();
    return value;
  } catch (sdbus::Error& e) {
    std::cerr << e.what() << std::endl;
    return std::nullopt;
  }
}
std::optional<std::vector<std::string>> screen_controller::dbus::
    BluetoothLeAdvertisingManager::GetSupportedSecondaryChannels() const {
  try {
    const auto value = adapter_proxy_->getProperty("SupportedSecondaryChannels")
                           .onInterface(adv_manager_interface_name)
                           .get<std::vector<std::string>>();
    return value;
  } catch (sdbus::Error& e) {
    std::cerr << e.what() << std::endl;
    return std::nullopt;
  }
}
std::optional<std::unordered_map<std::string, sdbus::Variant>>
BluetoothLeAdvertisingManager::GetSupportedCapabilities() const {
  try {
    const auto value =
        adapter_proxy_->getProperty("SupportedCapabilities")
            .onInterface(adv_manager_interface_name)
            .get<std::unordered_map<std::string, sdbus::Variant>>();
    return value;
  } catch (sdbus::Error& e) {
    std::cerr << e.what() << std::endl;
    return std::nullopt;
  }
}
std::optional<std::vector<std::string>>
BluetoothLeAdvertisingManager::GetSupportedFeatures() const {
  try {
    const auto value = adapter_proxy_->getProperty("SupportedFeatures")
                           .onInterface(adv_manager_interface_name)
                           .get<std::vector<std::string>>();
    return value;
  } catch (sdbus::Error& e) {
    std::cerr << e.what() << std::endl;
    return std::nullopt;
  }
}
}  // namespace screen_controller::dbus