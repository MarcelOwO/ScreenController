//
// Created by marce on 5/2/2025.
//

#include "bluetooth_agent.h"

#include <ng-log/logging.h>

#include <iostream>
#include <utility>

#include "bluetooth_device.h"
namespace screen_controller::bluetooth::dbus {

BluetoothAgent::BluetoothAgent(
    const std::shared_ptr<sdbus::IConnection>& connection,
    const sdbus::ObjectPath& device)
    : agent_path_(device),
      agent_object_(sdbus::createObject(*connection, agent_path_)),
      connection_(connection) {
  LOG(INFO) << "Creating BluetoothAgent";
}

void BluetoothAgent::init() const {
  agent_object_
      ->addVTable(
          sdbus::registerMethod("Release").withNoReply().implementedAs(
              [this] { PLOG(INFO) << "BluetoothAgent released"; }),
          sdbus::registerMethod("RequestPinCode")
              .implementedAs([this](const sdbus::ObjectPath& device_path) {
                PLOG(INFO) << "RequestPinCode";
                BluetoothDevice device_object(connection_, device_path);
                PLOG(INFO) << "Device name: " << device_object.GetAlias();
                return std::string("0000");
              }),
          sdbus::registerMethod("DisplayPinCode")
              .implementedAs([this](const sdbus::ObjectPath& device,
                                 const std::string& pincode) {
                PLOG(INFO) << "DisplayPinCode: " << pincode;
                BluetoothDevice device_object(connection_, device);
                PLOG(INFO) << "Device name: " << device_object.GetAlias();
              }),
          sdbus::registerMethod("RequestPasskey")
              .implementedAs([=](const sdbus::ObjectPath& device) {
                PLOG(INFO) << "RequestPasskey";
                BluetoothDevice device_object(connection_, device);
                PLOG(INFO) << "Device name: " << device_object.GetAlias();
                return 1234;
              }),
          sdbus::registerMethod("DisplayPasskey")
              .implementedAs([=](const sdbus::ObjectPath& device,
                                 const uint32_t passkey,
                                 const uint16_t entered) {
                PLOG(INFO) << "DisplayPasskey: " << passkey
                           << " Entered: " << entered;
                BluetoothDevice device_object(connection_, device);
                PLOG(INFO) << "Device name: " << device_object.GetAlias();
              }),
          sdbus::registerMethod("RequestConfirmation")
              .implementedAs(
                  [=](const sdbus::ObjectPath& device, const uint32_t passkey) {
                    PLOG(INFO) << "RequestConfirmation: " << passkey;
                    BluetoothDevice device_object(connection_, device);
                    PLOG(INFO) << "Device name: " << device_object.GetAlias();
                  }),
          sdbus::registerMethod("RequestAuthorization")
              .implementedAs([this](const sdbus::ObjectPath& device) {
                BluetoothDevice device_object(connection_, device);
                PLOG(INFO) << "Device name: " << device_object.GetAlias();
                PLOG(INFO) << "RequestAuthorization";
              }),
          sdbus::registerMethod("AuthorizeService")
              .implementedAs([this](const sdbus::ObjectPath& device,
                                 const std::string& uuid) {
                BluetoothDevice device_object(connection_, device);
                PLOG(INFO) << "Device name: " << device_object.GetAlias();
                PLOG(INFO) << "AuthorizeService: " << uuid;
              }),
          sdbus::registerMethod("Cancel").implementedAs(
              [this] { PLOG(INFO) << "Agent Cancel"; }))
      .forInterface(sdbus::InterfaceName("org.bluez.Agent1"));
}
}  // namespace screen_controller::bluetooth::dbus
