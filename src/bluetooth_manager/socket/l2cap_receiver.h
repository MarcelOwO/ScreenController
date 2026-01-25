//
// Created by marce on 5/8/2025.
//

#ifndef L_2_CAP_RECEIVER_H
#define L_2_CAP_RECEIVER_H

#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>

#include <fstream>
#include <functional>
#include <memory>
#include <span>
#include <vector>

#include "../models/packet.h"
#include "logging/logger.h"
#include "socket_implementations/socket_options.h"

namespace screen_controller {
class L2CapReceiver {
 public:
  explicit L2CapReceiver(const std::shared_ptr<Logger>& logger);
  ~L2CapReceiver();

  L2CapReceiver(const L2CapReceiver&) = delete;
  L2CapReceiver& operator=(const L2CapReceiver&) = delete;
  L2CapReceiver(L2CapReceiver&&) = delete;
  L2CapReceiver& operator=(L2CapReceiver&&) = delete;

  bool init();

  void poll_socket();

  void OnReceived(
      const std::function<void(const std::span<std::byte>& data)>& callback);

  void OnPacket(std::function<void(const packet&)> callback);

  void OnError(
      const std::function<void(int code, std::string_view message)>& callback);
  bool SendPacket(uint8_t type, std::string_view name,
                  std::span<const std::byte> payload = {});

  bool SendError(uint32_t err_code, std::string_view message,
                 uint8_t type = 0xFF);

 private:
  void CheckClient();
  void ReadAllAvailable();
  bool ExtractOnePacket();

  void TryEnable2MDefaultPhy();

  int l2_cap_socket_{-1};
  int client_socket_{-1};

  SocketOptions socket_options_;

  int imtu_{672};
  int omtu_{672};
  std::shared_ptr<Logger> logger_;

  std::vector<uint8_t> received_buffer_;
  std::vector<uint8_t> temp_record_;
  std::vector<uint8_t> received_data_;
  std::vector<uint8_t> current_payload_;

  std::function<void(const std::span<std::byte>&)> on_received_;
  std::function<void(const packet&)> on_packet_;
  std::function<void(int, std::string_view)> on_error_;
};

}  // namespace screen_controller

#endif  // L_2_CAP_RECEIVER_H
