#include "gatt_application.hpp"

#include <sys/random.h>

#include <algorithm>
#include <cstdlib>
#include <format>
#include <fstream>
#include <thread>

#include "../../socket/socket_helper.hpp"
#include "../../socket/socket_variables.hpp"
#include "sdbus-c++/Error.h"
#include "sdbus-c++/sdbus-c++.h"

namespace screen_controller::dbus {
namespace {

constexpr std::string_view kServiceUuid = "8e7f1a10-6e40-4d5f-8d7c-9b5a9f88a001";
constexpr std::string_view kWriteUuid = "8e7f1a10-6e40-4d5f-8d7c-9b5a9f88a002";
constexpr std::string_view kNotifyUuid = "8e7f1a10-6e40-4d5f-8d7c-9b5a9f88a003";
constexpr const char* kServiceInterface = "org.bluez.GattService1";
constexpr const char* kCharacteristicInterface = "org.bluez.GattCharacteristic1";
constexpr std::size_t kNotificationChunk = 180;

std::filesystem::path ControllerIdPath() {
  if (const char* configured = std::getenv("SCREEN_CONTROLLER_CONTROLLER_ID_PATH")) {
    return configured;
  }
  if (const char* state_directory = std::getenv("SCREEN_CONTROLLER_STATE_DIR")) {
    return std::filesystem::path{state_directory}.parent_path() / "controller.id";
  }
  return "controller.id";
}

}  // namespace

GattApplication::GattApplication(ILogger& logger,
                                 const std::shared_ptr<sdbus::IConnection>& connection,
                                 const std::shared_ptr<sdbus::IProxy>& adapter_proxy)
    : logger_(logger),
      connection_(connection),
      adapter_proxy_(adapter_proxy),
      root_path_(sdbus::ObjectPath("/owo/gatt")),
      service_path_(sdbus::ObjectPath("/owo/gatt/service0")),
      write_path_(sdbus::ObjectPath("/owo/gatt/service0/write")),
      notify_path_(sdbus::ObjectPath("/owo/gatt/service0/notify")),
      object_manager_(logger, *connection_, root_path_),
      service_object_(sdbus::createObject(*connection_, service_path_)),
      write_object_(sdbus::createObject(*connection_, write_path_)),
      notify_object_(sdbus::createObject(*connection_, notify_path_)),
      controller_id_path_(ControllerIdPath()) {
  const auto key = auth::LoadKeyFromEnvironment();
  if (!key) {
    throw std::runtime_error(key.error());
  }
  auth_key_ = *key;
  LoadAuthorizedController();

  service_object_
      ->addVTable(
          sdbus::registerProperty("UUID").withGetter([] { return std::string(kServiceUuid); }),
          sdbus::registerProperty("Primary").withGetter([] { return true; }))
      .forInterface(sdbus::InterfaceName(kServiceInterface));

  write_object_
      ->addVTable(
          sdbus::registerMethod("WriteValue")
              .implementedAs([this](const std::vector<uint8_t>& value,
                                    const std::map<std::string, sdbus::Variant>& options) {
                HandleWrite(value, options);
              }),
          sdbus::registerProperty("UUID").withGetter([] { return std::string(kWriteUuid); }),
          sdbus::registerProperty("Service").withGetter([this] { return service_path_; }),
          sdbus::registerProperty("Flags").withGetter([] {
            return std::vector<std::string>{"write", "write-without-response", "encrypt-write"};
          }))
      .forInterface(sdbus::InterfaceName(kCharacteristicInterface));

  notify_object_
      ->addVTable(
          sdbus::registerMethod("StartNotify").implementedAs([this] { notifying_.store(true); }),
          sdbus::registerMethod("StopNotify").implementedAs([this] {
            notifying_.store(false);
            ResetSession(true);
          }),
          sdbus::registerProperty("UUID").withGetter([] { return std::string(kNotifyUuid); }),
          sdbus::registerProperty("Service").withGetter([this] { return service_path_; }),
          sdbus::registerProperty("Flags").withGetter(
              [] { return std::vector<std::string>{"notify", "encrypt-notify"}; }),
          sdbus::registerProperty("Notifying").withGetter([this] { return notifying_.load(); }),
          sdbus::registerProperty("Value").withGetter([this] {
            const std::lock_guard lock(notification_mutex_);
            return notification_value_;
          }))
      .forInterface(sdbus::InterfaceName(kCharacteristicInterface));

  logger_.LogInfo("Created the Windows-compatible BLE GATT transport");
}

GattApplication::~GattApplication() {
  Unregister();
}

bool GattApplication::Register() {
  if (registered_.load()) {
    return true;
  }
  try {
    registration_slot_ =
        adapter_proxy_->callMethodAsync(sdbus::MethodName("RegisterApplication"))
            .onInterface(sdbus::InterfaceName("org.bluez.GattManager1"))
            .withArguments(root_path_, std::map<std::string, sdbus::Variant>{})
            .uponReplyInvoke(
                [this](const std::optional<sdbus::Error> error) {
                  if (error) {
                    logger_.LogFmt(LogLevel::ERROR, "Could not register BLE GATT application: {}",
                                   error->getMessage());
                    return;
                  }
                  registered_.store(true);
                  logger_.LogInfo("Registered BLE GATT application");
                },
                sdbus::return_slot);
    return true;
  } catch (const sdbus::Error& error) {
    logger_.LogFmt(LogLevel::ERROR, "Could not register BLE GATT application: {}",
                   error.getMessage());
    return false;
  }
}

void GattApplication::Unregister() {
  registration_slot_.reset();
  if (!registered_.load()) {
    return;
  }
  try {
    adapter_proxy_->callMethod(sdbus::MethodName("UnregisterApplication"))
        .onInterface(sdbus::InterfaceName("org.bluez.GattManager1"))
        .withArguments(root_path_);
  } catch (const sdbus::Error& error) {
    logger_.LogFmt(LogLevel::WARN, "Could not unregister BLE GATT application: {}",
                   error.getMessage());
  }
  registered_.store(false);
}

void GattApplication::OnPacket(std::function<void(const Packet&)> callback) {
  on_packet_ = std::move(callback);
}
void GattApplication::OnConnected(std::function<void()> callback) {
  on_connected_ = std::move(callback);
}
void GattApplication::OnAuthenticated(std::function<void()> callback) {
  on_authenticated_ = std::move(callback);
}
void GattApplication::OnDisconnected(std::function<void()> callback) {
  on_disconnected_ = std::move(callback);
}

bool GattApplication::IsAuthenticated() const {
  return auth_state_.load() == AuthState::kAuthenticated;
}
bool GattApplication::IsCommissioned() const {
  return commissioned_.load();
}
bool GattApplication::IsRegistered() const {
  return registered_.load();
}

void GattApplication::HandleWrite(const std::vector<uint8_t>& value,
                                  const std::map<std::string, sdbus::Variant>& options) {
  if (auth_state_.load() == AuthState::kWaiting) {
    if (value != std::vector<uint8_t>{0x01}) {
      throw sdbus::Error(sdbus::Error::Name("org.bluez.Error.NotAuthorized"),
                         "Start the authenticated ScreenController session first");
    }
    StartSession(options);
    return;
  }

  if (auth_state_.load() == AuthState::kChallengeSent &&
      std::chrono::steady_clock::now() > challenge_deadline_) {
    ResetSession(true);
    throw sdbus::Error(sdbus::Error::Name("org.bluez.Error.NotAuthorized"),
                       "Authentication challenge timed out");
  }

  received_data_.insert(received_data_.end(), value.begin(), value.end());
  const std::size_t max_buffer =
      auth_state_.load() == AuthState::kAuthenticated
          ? static_cast<std::size_t>(socket::kMaxPayload) + socket::kMaxNameLen + 15U
          : 1024U;
  if (received_data_.size() > max_buffer) {
    (void) SendError(0x02, "receive buffer limit exceeded");
    ResetSession(true);
    return;
  }
  while (ExtractOnePacket()) {
  }
}

void GattApplication::StartSession(const std::map<std::string, sdbus::Variant>& options) {
  const auto device = options.find("device");
  if (device == options.end()) {
    throw sdbus::Error(sdbus::Error::Name("org.bluez.Error.InvalidArguments"),
                       "BlueZ did not identify the GATT client");
  }
  LoadAuthorizedController();
  connected_controller_ = ResolveController(device->second.get<sdbus::ObjectPath>());
  if (authorized_controller_ && *authorized_controller_ != connected_controller_) {
    connected_controller_.clear();
    throw sdbus::Error(sdbus::Error::Name("org.bluez.Error.NotAuthorized"),
                       "This display is commissioned to another controller");
  }
  if (!notifying_.load()) {
    throw sdbus::Error(sdbus::Error::Name("org.bluez.Error.NotPermitted"),
                       "Subscribe to responses before starting a session");
  }

  if (getrandom(nonce_.data(), nonce_.size(), 0) != static_cast<ssize_t>(nonce_.size())) {
    connected_controller_.clear();
    throw sdbus::Error(sdbus::Error::Name("org.bluez.Error.Failed"),
                       "Could not generate authentication challenge");
  }
  auth_state_.store(AuthState::kChallengeSent);
  challenge_deadline_ = std::chrono::steady_clock::now() + std::chrono::seconds{5};
  if (on_connected_) {
    on_connected_();
  }
  const auto nonce = std::as_bytes(std::span{nonce_});
  if (!SendPacket(socket::kTypeChallenge, "auth-v2", nonce)) {
    ResetSession(true);
    throw sdbus::Error(sdbus::Error::Name("org.bluez.Error.Failed"),
                       "Could not send authentication challenge");
  }
}

void GattApplication::ResetSession(const bool notify_disconnect) {
  const bool was_active = auth_state_.load() != AuthState::kWaiting;
  auth_state_.store(AuthState::kWaiting);
  connected_controller_.clear();
  received_data_.clear();
  current_payload_.clear();
  if (notify_disconnect && was_active && on_disconnected_) {
    on_disconnected_();
  }
}

void GattApplication::ValidateAuth(const std::span<const uint8_t> payload) {
  if (!auth::ValidateResponse(auth_key_, nonce_, payload)) {
    (void) SendError(0x01, "auth failed");
    ResetSession(true);
    return;
  }
  if (!authorized_controller_ && !PersistController()) {
    (void) SendError(0x09, "controller commissioning could not be persisted");
    ResetSession(true);
    return;
  }
  auth_state_.store(AuthState::kAuthenticated);
  logger_.LogInfo("GATT client authenticated successfully");
  if (on_authenticated_) {
    on_authenticated_();
  }
}

bool GattApplication::ExtractOnePacket() {
  auto& buffer = received_data_;
  if (buffer.size() < socket::kHeaderMin) {
    return false;
  }
  if (socket::FromVector<uint16_t>(buffer, 0) != socket::kMagic) {
    (void) SendError(0x04, "invalid packet magic");
    ResetSession(true);
    return false;
  }
  const uint8_t type = buffer[2];
  const auto name_length = socket::FromVector<uint32_t>(buffer, 3);
  if (name_length > socket::kMaxNameLen) {
    (void) SendError(0x04, "name too large");
    ResetSession(true);
    return false;
  }
  const std::size_t after_name = 7U + name_length;
  if (buffer.size() < after_name) {
    return false;
  }
  std::string name(reinterpret_cast<const char*>(&buffer[7]), name_length);
  if (type < 0x80) {
    buffer.erase(buffer.begin(), buffer.begin() + static_cast<std::ptrdiff_t>(after_name));
    if (!IsAuthenticated()) {
      (void) SendError(0x01, "authentication required");
      ResetSession(true);
      return false;
    }
    if (on_packet_) {
      on_packet_(
          Packet{.type_ = type, .name_ = std::move(name), .payload_ = {}, .has_payload_ = false});
    }
    return true;
  }
  if (buffer.size() < after_name + 8U) {
    return false;
  }
  const auto payload_length = socket::FromVector<uint32_t>(buffer, after_name);
  if (payload_length > socket::kMaxPayload) {
    (void) SendError(0x02, "payload too large");
    ResetSession(true);
    return false;
  }
  const std::size_t packet_length = after_name + 8U + payload_length;
  if (buffer.size() < packet_length) {
    return false;
  }
  const auto expected_crc = socket::FromVector<uint32_t>(buffer, after_name + 4U);
  const auto* payload_data = &buffer[after_name + 8U];
  const std::span<const uint8_t> payload(payload_data, payload_length);
  if (socket::Crc(payload) != expected_crc) {
    (void) SendError(0x03, "crc mismatch");
    buffer.erase(buffer.begin(), buffer.begin() + static_cast<std::ptrdiff_t>(packet_length));
    return true;
  }
  if (type == socket::kTypeAuthResponse && name == "auth-v2") {
    current_payload_.assign(payload.begin(), payload.end());
    buffer.erase(buffer.begin(), buffer.begin() + static_cast<std::ptrdiff_t>(packet_length));
    if (auth_state_.load() != AuthState::kChallengeSent) {
      (void) SendError(0x01, "unexpected authentication response");
      ResetSession(true);
      return false;
    }
    ValidateAuth(current_payload_);
    return true;
  }
  if (!IsAuthenticated()) {
    (void) SendError(0x01, "authentication required");
    ResetSession(true);
    return false;
  }
  current_payload_.assign(payload.begin(), payload.end());
  buffer.erase(buffer.begin(), buffer.begin() + static_cast<std::ptrdiff_t>(packet_length));
  if (on_packet_) {
    on_packet_(Packet{.type_ = type,
                      .name_ = std::move(name),
                      .payload_ = std::as_bytes(std::span{current_payload_}),
                      .has_payload_ = true});
  }
  return true;
}

bool GattApplication::SendPacket(const uint8_t type, const std::string_view name,
                                 const std::span<const std::byte> payload) {
  if (!notifying_.load()) {
    return false;
  }
  std::vector<std::byte> bytes;
  socket::BuildPacketBytes(type, socket::kMagic, name, payload, bytes);
  return Notify(bytes);
}

bool GattApplication::SendError(const uint32_t code, const std::string_view message) {
  const std::string text = std::format("ERR:{}:{}", code, message);
  return SendPacket(0xCF, "error",
                    std::span(reinterpret_cast<const std::byte*>(text.data()), text.size()));
}

bool GattApplication::Notify(const std::span<const std::byte> bytes) {
  for (std::size_t offset = 0; offset < bytes.size(); offset += kNotificationChunk) {
    const auto length = std::min(kNotificationChunk, bytes.size() - offset);
    {
      const std::lock_guard lock(notification_mutex_);
      notification_value_.assign(reinterpret_cast<const uint8_t*>(bytes.data() + offset),
                                 reinterpret_cast<const uint8_t*>(bytes.data() + offset + length));
    }
    try {
      notify_object_->emitPropertiesChangedSignal(sdbus::InterfaceName(kCharacteristicInterface),
                                                  {sdbus::PropertyName("Value")});
    } catch (const sdbus::Error& error) {
      logger_.LogFmt(LogLevel::ERROR, "Could not send GATT notification: {}", error.getMessage());
      return false;
    }
    if (offset + length < bytes.size()) {
      std::this_thread::sleep_for(std::chrono::milliseconds{2});
    }
  }
  return true;
}

std::string GattApplication::ResolveController(const sdbus::ObjectPath& device_path) const {
  auto device = sdbus::createProxy(*connection_, sdbus::ServiceName("org.bluez"), device_path);
  const auto interface = sdbus::InterfaceName("org.bluez.Device1");
  const auto address = device->getProperty("Address").onInterface(interface).get<std::string>();
  const auto type = device->getProperty("AddressType").onInterface(interface).get<std::string>();
  return std::format("{}/{}", address, type == "public" ? 1 : 2);
}

void GattApplication::LoadAuthorizedController() {
  std::ifstream file(controller_id_path_);
  std::string controller;
  if (!file || !std::getline(file, controller) || controller.empty()) {
    return;
  }
  authorized_controller_ = std::move(controller);
  commissioned_.store(true);
}

bool GattApplication::PersistController() {
  if (connected_controller_.empty()) {
    return false;
  }
  std::error_code error;
  if (const auto parent = controller_id_path_.parent_path(); !parent.empty()) {
    std::filesystem::create_directories(parent, error);
    if (error) {
      return false;
    }
  }
  auto temporary = controller_id_path_;
  temporary += ".new";
  {
    std::ofstream file(temporary, std::ios::trunc);
    if (!file || !(file << connected_controller_ << '\n')) {
      return false;
    }
  }
  std::filesystem::rename(temporary, controller_id_path_, error);
  if (error) {
    std::filesystem::remove(temporary, error);
    return false;
  }
  authorized_controller_ = connected_controller_;
  commissioned_.store(true);
  return true;
}

}  // namespace screen_controller::dbus
