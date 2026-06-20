//
// Created by marce on 5/2/2025.
//

#include "bluetooth_agent.hpp"

#include "bluetooth_device.hpp"
namespace screen_controller::dbus {

BluetoothAgent::BluetoothAgent(ILogger& logger,
                               const std::shared_ptr<sdbus::IConnection>& connection,
                               const sdbus::ObjectPath& device)
    : logger_(logger),
      agent_path_(device),
      agent_object_(sdbus::createObject(*connection, agent_path_)),
      connection_(connection) {
  logger_.LogInfo("Creating BluetoothAgent");

  agent_object_
      ->addVTable(
          sdbus::registerMethod("Release").withNoReply().implementedAs(
              [this] { logger_.LogInfo("BluetoothAgent released"); }),
          sdbus::registerMethod("RequestPinCode")
              .implementedAs([this](const sdbus::ObjectPath& device_path) {
                logger_.LogInfo("RequestPinCode");
                const BluetoothDevice device_object(logger_, connection_, device_path);

                if (const auto res = device_object.GetName(); res.has_value()) {
                  logger_.LogInfo("Device name: " + std::string(res.value()));
                }

                return std::string("0000");
              }),
          sdbus::registerMethod("DisplayPinCode")
              .implementedAs([this](const sdbus::ObjectPath& device, const std::string& pincode) {
                logger_.LogInfo("DisplayPinCode: " + pincode);
                const BluetoothDevice device_object(logger_, connection_, device);
                if (const auto res = device_object.GetName()) {
                  logger_.LogInfo("Device name: " + res.value());
                }
              }),
          sdbus::registerMethod("RequestPasskey")
              .implementedAs([this](const sdbus::ObjectPath& device) {
                logger_.LogInfo("RequestPasskey");
                const BluetoothDevice device_object(logger_, connection_, device);
                if (const auto res = device_object.GetName()) {
                  logger_.LogInfo("Device name: " + res.value());
                }

                return 1234;
              }),
          sdbus::registerMethod("DisplayPasskey")
              .implementedAs([this](const sdbus::ObjectPath& device, const uint32_t passkey,
                                    const uint16_t entered) {
                logger_.LogInfo("DisplayPasskey: " + std::to_string(passkey) +
                                " Entered: " + std::to_string(entered));
                const BluetoothDevice device_object(logger_, connection_, device);
                const auto res = device_object.GetName();
                logger_.LogInfo("Device name: " + res.value());
              }),
          sdbus::registerMethod("RequestConfirmation")
              .implementedAs([this](const sdbus::ObjectPath& device, const uint32_t passkey) {
                logger_.LogInfo("RequestConfirmation: " + std::to_string(passkey));

                const BluetoothDevice device_object(logger_, connection_, device);
                if (const auto res = device_object.GetName()) {
                  logger_.LogInfo("Device name: " + res.value());
                }
              }),
          sdbus::registerMethod("RequestAuthorization")
              .implementedAs([this](const sdbus::ObjectPath& device) {
                logger_.LogInfo("RequestAuthorization");
                const BluetoothDevice device_object(logger_, connection_, device);
                const auto res = device_object.GetName();
                logger_.LogInfo("Device name: " + res.value());
              }),
          sdbus::registerMethod("AuthorizeService")
              .implementedAs([this](const sdbus::ObjectPath& device, const std::string& uuid) {
                logger_.LogInfo("AuthorizeService: " + uuid);
                const BluetoothDevice device_object(logger_, connection_, device);
                if (const auto res = device_object.GetName()) {
                  logger_.LogInfo("Device name: " + res.value());
                }
              }),
          sdbus::registerMethod("Cancel").implementedAs(
              [this] { logger_.LogInfo("Agent Cancel"); }))
      .forInterface(sdbus::InterfaceName("org.bluez.Agent1"));
}

}  // namespace screen_controller::dbus
