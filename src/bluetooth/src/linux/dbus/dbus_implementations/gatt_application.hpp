#pragma once

#include <atomic>
#include <chrono>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include <logging/logger.hpp>

#include "../../auth/authenticator.hpp"
#include "../../models/packet.hpp"
#include "object_manager.hpp"
#include "sdbus-c++/IConnection.h"
#include "sdbus-c++/IObject.h"
#include "sdbus-c++/IProxy.h"
#include "sdbus-c++/Types.h"

namespace screen_controller::dbus {

class GattApplication {
public:
  GattApplication(ILogger& logger, const std::shared_ptr<sdbus::IConnection>& connection,
                  const std::shared_ptr<sdbus::IProxy>& adapter_proxy);
  ~GattApplication();

  GattApplication(const GattApplication&) = delete;
  GattApplication& operator=(const GattApplication&) = delete;
  GattApplication(GattApplication&&) = delete;
  GattApplication& operator=(GattApplication&&) = delete;

  bool Register();
  void Unregister();

  void OnPacket(std::function<void(const Packet&)> callback);
  void OnConnected(std::function<void()> callback);
  void OnAuthenticated(std::function<void()> callback);
  void OnDisconnected(std::function<void()> callback);

  [[nodiscard]] bool SendPacket(uint8_t type, std::string_view name,
                                std::span<const std::byte> payload = {});
  [[nodiscard]] bool SendError(uint32_t code, std::string_view message);
  [[nodiscard]] bool IsAuthenticated() const;
  [[nodiscard]] bool IsCommissioned() const;
  [[nodiscard]] bool IsRegistered() const;

private:
  enum class AuthState { kWaiting, kChallengeSent, kAuthenticated };

  void HandleWrite(const std::vector<uint8_t>& value,
                   const std::map<std::string, sdbus::Variant>& options);
  void StartSession(const std::map<std::string, sdbus::Variant>& options);
  void ResetSession(bool notify_disconnect);
  void ValidateAuth(std::span<const uint8_t> payload);
  bool ExtractOnePacket();
  bool Notify(std::span<const std::byte> bytes);
  std::string ResolveController(const sdbus::ObjectPath& device_path) const;
  void LoadAuthorizedController();
  bool PersistController();

  ILogger& logger_;
  std::shared_ptr<sdbus::IConnection> connection_;
  std::shared_ptr<sdbus::IProxy> adapter_proxy_;
  sdbus::ObjectPath root_path_;
  sdbus::ObjectPath service_path_;
  sdbus::ObjectPath write_path_;
  sdbus::ObjectPath notify_path_;
  ObjectManager object_manager_;
  std::unique_ptr<sdbus::IObject> service_object_;
  std::unique_ptr<sdbus::IObject> write_object_;
  std::unique_ptr<sdbus::IObject> notify_object_;
  std::atomic_bool registered_{false};
  sdbus::Slot registration_slot_;
  std::atomic_bool notifying_{false};
  std::mutex notification_mutex_;
  std::vector<uint8_t> notification_value_;

  auth::Key auth_key_{};
  auth::Nonce nonce_{};
  std::atomic<AuthState> auth_state_{AuthState::kWaiting};
  std::atomic_bool commissioned_{false};
  std::chrono::steady_clock::time_point challenge_deadline_{};
  std::filesystem::path controller_id_path_;
  std::optional<std::string> authorized_controller_;
  std::string connected_controller_;
  std::vector<uint8_t> received_data_;
  std::vector<uint8_t> current_payload_;

  std::function<void(const Packet&)> on_packet_;
  std::function<void()> on_connected_;
  std::function<void()> on_authenticated_;
  std::function<void()> on_disconnected_;
};

}  // namespace screen_controller::dbus
