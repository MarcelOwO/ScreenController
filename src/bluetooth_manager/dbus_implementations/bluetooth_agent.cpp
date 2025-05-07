//
// Created by marce on 5/2/2025.
//

#include "bluetooth_agent.h"

#include <utility>
namespace screen_controller::dbus {

BluetoothAgent::BluetoothAgent(
    const std::shared_ptr<sdbus::IConnection>& connection,
    sdbus::ObjectPath device)
    : agent_path_(std::move(device)),
      agent_object_(sdbus::createObject(*connection, agent_path_)) {}

void BluetoothAgent::Setup(
    const std::function<void()>& release,
    const std::function<std::string(sdbus::ObjectPath)>& request_pin_code,
    const std::function<void(sdbus::ObjectPath, std::string)>& display_pin_code,
    const std::function<uint32_t(sdbus::ObjectPath)>& request_passkey,
    const std::function<void(sdbus::ObjectPath, uint32_t, uint16_t)>&
        display_passkey,
    const std::function<void(sdbus::ObjectPath, uint32_t)>&
        request_confirmation,
    const std::function<void(sdbus::ObjectPath)>& request_authorization,
    const std::function<void(sdbus::ObjectPath a, std::string)>&
        authorize_service,
    const std::function<void()>& cancel) const {
  agent_object_
      ->addVTable(
          sdbus::registerMethod("Release").withNoReply().implementedAs(
              [=] { release(); }),
          sdbus::registerMethod("RequestPinCode")
              .implementedAs([=](const sdbus::ObjectPath& device) {
                return request_pin_code(device);
              }),
          sdbus::registerMethod("DisplayPinCode")
              .implementedAs([=](const sdbus::ObjectPath& device,
                                 const std::string& pincode) {
                display_pin_code(device, pincode);
              }),
          sdbus::registerMethod("RequestPasskey")
              .implementedAs([=](const sdbus::ObjectPath& device) {
                return request_passkey(device);
              }),
          sdbus::registerMethod("DisplayPasskey")
              .implementedAs([=](const sdbus::ObjectPath& device,
                                 const uint32_t passkey,
                                 const uint16_t entered) {
                display_passkey(device, passkey, entered);
              }),
          sdbus::registerMethod("RequestConfirmation")
              .implementedAs(
                  [=](const sdbus::ObjectPath& device, const uint32_t passkey) {
                    request_confirmation(device, passkey);
                  }),
          sdbus::registerMethod("RequestAuthorization")
              .implementedAs([=](const sdbus::ObjectPath& device) {
                request_authorization(device);
              }),
          sdbus::registerMethod("AuthorizeService")
              .implementedAs([=](const sdbus::ObjectPath& device,
                                 const std::string& uuid) {
                authorize_service(device, uuid);
              }),
          sdbus::registerMethod("Cancel").implementedAs([=]() { cancel(); }))
      .forInterface(sdbus::InterfaceName("org.bluez.Agent1"));
}
}  // namespace screen_controller::dbus
