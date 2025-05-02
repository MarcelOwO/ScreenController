//
// Created by marce on 4/2/2025.
//

#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

#include <sdbus-c++/sdbus-c++.h>

#include <functional>
#include <string>
#include <vector>


namespace screen_controller {

class BluetoothManager {
 public:
  BluetoothManager();
  ~BluetoothManager();

  bool init();

  using file_data_callback = std::function<void(
      std::string file_name, std::vector<std::byte> file_data)>;
  using command_callback = std::function<void(
      std::string command_name, std::vector<std::byte> data_data)>;

  void on_file_received(file_data_callback callback);
  void on_command_received(command_callback callback);

  void open_socket();

 private:
  std::unique_ptr<sdbus::IConnection> connection_ =
      sdbus::createSystemBusConnection();

  sdbus::InterfaceName adapter_interface_name_;
  sdbus::InterfaceName adv_manager_interface_name_;
  sdbus::InterfaceName gatt_mgr_interface_name_;
  sdbus::InterfaceName agent_mgr_iface_;

  sdbus::ObjectPath gatt_object_path_;
  sdbus::ObjectPath adapter_path_;
  sdbus::ObjectPath advertisement_path_;
  sdbus::ObjectPath service_path_;
  sdbus::ObjectPath char_path_;
  sdbus::ObjectPath agent_path_;

  file_data_callback file_data_callback_;
  command_callback command_callback_;

  bool register_advertisement();
  bool setup_adapter(sdbus::IProxy& adapter_proxy);
  void unregister_agent();
  bool register_agent();
};
}  // namespace screen_controller

#endif  // BLUETOOTH_MANAGER_H
