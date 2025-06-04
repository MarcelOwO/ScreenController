//
// Created by marce on 5/8/2025.
//

#ifndef L_2_CAP_RECEIVER_H
#define L_2_CAP_RECEIVER_H

#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>

#include <fstream>
#include <functional>
#include <span>
#include <vector>

namespace screen_controller::bluetooth {
class L2CapReceiver {
 public:
  L2CapReceiver();
  ~L2CapReceiver();

  L2CapReceiver(const L2CapReceiver&) = delete;
  L2CapReceiver& operator=(const L2CapReceiver&) = delete;
  L2CapReceiver(L2CapReceiver&&) = delete;
  L2CapReceiver& operator=(L2CapReceiver&&) = delete;

  void OnReceived(std::function<void(const std::span<std::byte>)> callback);

  bool init();
  void run();

 private:
  std::vector<uint8_t> received_data_;
  bool transfer_complete_ = false;
  int l2_cap_socket_;
  int client_socket_;

  std::function<void(const std::span<std::byte>)> on_received_;
  std::vector<std::byte> received_bytes_;

  bool SetSocketOption(int level, int option, int value) const;
  void SetReuseAddress(int value);
  void SetReceiveBufferSize(int value);
  void SetSendBufferSize(int value);
  void SetFlushable(int value);
  void SetDefer(int value);
  void SetNonBlocking();
};

}  // namespace screen_controller::bluetooth

#endif  // L_2_CAP_RECEIVER_H
