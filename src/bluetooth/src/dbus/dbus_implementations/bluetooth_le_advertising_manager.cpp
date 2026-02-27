//
// Created by marce on 5/3/2025.
//

#include "bluetooth_le_advertising_manager.h"

namespace screen_controller {

BluetoothLeAdvertisingManager::BluetoothLeAdvertisingManager(
    ILogger& logger, const std::shared_ptr<sdbus::IProxy>& adapter_proxy)
    : logger_(logger),
      adapter_proxy_(adapter_proxy),
      adv_manager_interface_name(
          sdbus::InterfaceName("org.bluez.LEAdvertisingManager1")) {}

BluetoothLeAdvertisingManager::~BluetoothLeAdvertisingManager() = default;

std::expected<void, std::error_code>
BluetoothLeAdvertisingManager::RegisterAdvertisement(
    const sdbus::ObjectPath& advertisement,
    const std::unordered_map<std::string, sdbus::Variant>& options) const {
  try {
    adapter_proxy_->callMethod(sdbus::MethodName("RegisterAdvertisement"))
        .onInterface(adv_manager_interface_name)
        .withArguments(advertisement, options)
        .dontExpectReply();
    return {};
  } catch (const sdbus::Error& e) {
    logger_.LogError(e.getMessage());
    return std::unexpected(
        std::make_error_code(std::errc::operation_not_permitted));
  }
}

std::expected<void, std::error_code>
BluetoothLeAdvertisingManager::UnregisterAdvertisement() const {
  try {
    adapter_proxy_->callMethod(sdbus::MethodName("UnregisterAdvertisement"))
        .onInterface(adv_manager_interface_name);
    return {};
  } catch (const sdbus::Error& e) {
    logger_.LogError(e.getMessage());
    return std::unexpected(
        std::make_error_code(std::errc::operation_not_permitted));
  }
}

std::optional<uint8_t> BluetoothLeAdvertisingManager::GetActiveInstances()
    const {
  try {
    const auto value = adapter_proxy_->getProperty("ActiveInstances")
                           .onInterface(adv_manager_interface_name)
                           .get<uint8_t>();
    return value;
  } catch (const sdbus::Error& e) {
    logger_.LogError(e.getMessage());
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
  } catch (const sdbus::Error& e) {
    logger_.LogError(e.getMessage());
    return std::nullopt;
  }
}

std::optional<std::vector<std::string>>
BluetoothLeAdvertisingManager::GetSupportedIncludes() const {
  try {
    return adapter_proxy_->getProperty("SupportedIncludes")
        .onInterface(adv_manager_interface_name)
        .get<std::vector<std::string>>();
  } catch (const sdbus::Error& e) {
    logger_.LogError(e.getMessage());
    return std::nullopt;
  }
}

std::optional<std::vector<std::string>>
BluetoothLeAdvertisingManager::GetSupportedSecondaryChannels() const {
  try {
    const auto value = adapter_proxy_->getProperty("SupportedSecondaryChannels")
                           .onInterface(adv_manager_interface_name)
                           .get<std::vector<std::string>>();
    return value;
  } catch (const sdbus::Error& e) {
    logger_.LogError(e.getMessage());
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
  } catch (const sdbus::Error& e) {
    logger_.LogError(e.getMessage());
    return std::nullopt;
  }
}

std::optional<std::vector<std::string>>
BluetoothLeAdvertisingManager::GetSupportedFeatures() const {
  try {
    return adapter_proxy_->getProperty("SupportedFeatures")
        .onInterface(adv_manager_interface_name)
        .get<std::vector<std::string>>();
  } catch (const sdbus::Error& e) {
    logger_.LogError(e.getMessage());
    return std::nullopt;
  }
}
}  // namespace screen_controller
