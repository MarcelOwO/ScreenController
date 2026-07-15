//
// Created by marce on 5/8/2025.
//

#pragma once

#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>

#include <array>
#include <chrono>
#include <filesystem>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include <logging/logger.hpp>
#include <models/app_settings.hpp>
#include "../auth/authenticator.hpp"
#include "../models/packet.hpp"

namespace screen_controller::socket {

class L2CapReceiver {
public:
  explicit L2CapReceiver(ILogger& logger, const AppSettings& settings);
  ~L2CapReceiver();

  L2CapReceiver(const L2CapReceiver&) = delete;
  L2CapReceiver& operator=(const L2CapReceiver&) = delete;
  L2CapReceiver(L2CapReceiver&&) = delete;
  L2CapReceiver& operator=(L2CapReceiver&&) = delete;

  // Poll for new data or connections. Call this every frame from the main loop.
  void PollSocket();

  // Callbacks — set before PollSocket is first called.
  void OnReceived(std::function<void(const std::span<std::byte>&)> callback);
  void OnPacket(std::function<void(const Packet&)> callback);
  void OnError(std::function<void(int code, std::string_view message)> callback);
  void OnConnected(std::function<void()> callback);
  void OnAuthenticated(std::function<void()> callback);
  void OnDisconnected(std::function<void()> callback);

  // Send a packet to the connected, authenticated client.
  bool SendPacket(uint8_t type, std::string_view name, std::span<const std::byte> payload = {});
  bool SendError(uint32_t err_code, std::string_view message, uint8_t type = 0xCF);

  [[nodiscard]] bool IsConnected() const {
    return client_socket_ >= 0;
  }
  [[nodiscard]] bool IsAuthenticated() const {
    return auth_state_ == AuthState::kAuthenticated;
  }
  [[nodiscard]] bool IsCommissioned() const {
    return authorized_controller_.has_value();
  }

private:
  enum class AuthState { kWaiting, kChallengeSent, kAuthenticated };

  void CheckClient();
  void ReadAllAvailable();
  bool ExtractOnePacket();
  void ValidateAuth(std::span<const uint8_t> payload);
  void CloseClient(bool notify = true);
  [[nodiscard]] bool PersistController();
  void LoadAuthorizedController();

  void TryEnable2MDefaultPhy();

  int l2_cap_socket_{-1};
  int client_socket_{-1};

  const AppSettings& settings_;
  int imtu_;
  int omtu_;
  ILogger& logger_;

  AuthState auth_state_{AuthState::kWaiting};
  auth::Key auth_key_{};
  auth::Nonce nonce_{};
  std::chrono::steady_clock::time_point challenge_deadline_{};
  std::filesystem::path controller_id_path_;
  std::optional<std::string> authorized_controller_;
  std::string connected_controller_;

  std::vector<uint8_t> temp_record_;
  std::vector<uint8_t> received_data_;
  std::vector<uint8_t> current_payload_;

  std::function<void(const std::span<std::byte>&)> on_received_;
  std::function<void(const Packet&)> on_packet_;
  std::function<void(int, std::string_view)> on_error_;
  std::function<void()> on_connected_;
  std::function<void()> on_authenticated_;
  std::function<void()> on_disconnected_;
};

}  // namespace screen_controller::socket
