//
// Created by marce on 5/8/2025.
//

#include "l2cap_receiver.hpp"

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/l2cap.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <sys/random.h>
#include <sys/socket.h>
#include <unistd.h>
#include <logging/logger.hpp>

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <vector>

#include "../auth/authenticator.hpp"
#include "socket_helper.hpp"
#include "socket_options.hpp"
#include "socket_variables.hpp"

namespace screen_controller::socket {

namespace {

std::filesystem::path ControllerIdPath() {
  if (const char* configured = std::getenv("SCREEN_CONTROLLER_CONTROLLER_ID_PATH");
      configured != nullptr) {
    return configured;
  }
  if (const char* state_directory = std::getenv("SCREEN_CONTROLLER_STATE_DIR");
      state_directory != nullptr) {
    return std::filesystem::path{state_directory}.parent_path() / "controller.id";
  }
  return "controller.id";
}

}  // namespace

L2CapReceiver::L2CapReceiver(ILogger& logger, const AppSettings& settings)
    : settings_(settings),
      imtu_(settings.imtu_),
      omtu_(settings.omtu_),
      logger_(logger),
      controller_id_path_(ControllerIdPath()) {
  logger_.LogInfo("Creating L2CapReceiver");

  const auto key = auth::LoadKeyFromEnvironment();
  if (!key) {
    throw std::runtime_error(key.error());
  }
  auth_key_ = *key;
  LoadAuthorizedController();

  l2_cap_socket_ = ::socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);

  if (l2_cap_socket_ < 0) {
    const std::string kErrorMsg =
        std::format("Failed to create l2cap socket: {}", std::string(strerror(errno)));
    logger_.LogError(kErrorMsg);
    throw std::runtime_error(kErrorMsg);
  }

  auto res = socket::SetReceiveBufferSize(l2_cap_socket_, 1024 * 1024);
  if (!res) {
    logger_.LogError("Failed to set receive buffer size");
    throw std::runtime_error("Failed to set receive buffer size");
  }

  res = socket::SetSendBufferSize(l2_cap_socket_, 1024 * 1024);
  if (!res) {
    logger_.LogError("Failed to set send buffer size");
    throw std::runtime_error("Failed to set send buffer size");
  }

  res = socket::SetReuseAddress(l2_cap_socket_, 1);
  if (!res) {
    logger_.LogError("Failed to set reuse address");
    throw std::runtime_error("Failed to set reuse address");
  }

  res = socket::SetFlushable(l2_cap_socket_, 1);
  if (!res) {
    logger_.LogError("Failed to set flushable");
    throw std::runtime_error("Failed to set flushable");
  }

  res = socket::SetBluetoothSecurity(l2_cap_socket_, BT_SECURITY_MEDIUM);
  if (!res) {
    throw std::runtime_error("Failed to require an encrypted Bluetooth link");
  }

  res = socket::SetNonBlocking(l2_cap_socket_);
  if (!res) {
    logger_.LogError("Failed to set non-blocking");
    throw std::runtime_error("Failed to set non-blocking");
  }

  l2cap_options options{};
  socklen_t optlen = sizeof(options);

  if (getsockopt(l2_cap_socket_, SOL_L2CAP, L2CAP_OPTIONS, &options, &optlen) == 0) {
    options.imtu = std::max<int>(options.imtu, 65535);
    options.omtu = std::max<int>(options.omtu, 65535);

    if (setsockopt(l2_cap_socket_, SOL_L2CAP, L2CAP_OPTIONS, &options, sizeof(options)) != 0) {
      const std::string kErrorMsg =
          std::format("Failed to set L2cap options: {}", std::string(strerror(errno)));
      logger_.LogError(kErrorMsg);
    } else {
      imtu_ = options.imtu;
      omtu_ = options.omtu;
    }
  } else {
    logger_.LogInfo("getsockopt(L2CAP_OPTIONS) failed, using default options");
  }

  sockaddr_l2 loc_addr = {
      .l2_family = AF_BLUETOOTH,
      .l2_psm = htobs(0x0081),
      .l2_bdaddr_type = BDADDR_LE_PUBLIC,
  };

  if (bind(l2_cap_socket_, reinterpret_cast<sockaddr*>(&loc_addr), sizeof(loc_addr)) < 0) {
    const std::string kErrorMsg =
        std::format("Failed to bind l2cap socket: {}", std::string(strerror(errno)));
    logger_.LogError(kErrorMsg);
    throw std::runtime_error(kErrorMsg);
  }

  if (listen(l2_cap_socket_, 1) < 0) {
    const std::string kErrorMsg =
        std::format("Failed to listen on l2cap socket: {}", std::string(strerror(errno)));
    logger_.LogError(kErrorMsg);
    throw std::runtime_error(kErrorMsg);
  }

  TryEnable2MDefaultPhy();
}

L2CapReceiver::~L2CapReceiver() {
  logger_.LogInfo("Cleaning up L2CapReceiver");
  if (client_socket_ >= 0) {
    (void) close(client_socket_);
  }
  if (l2_cap_socket_ >= 0) {
    (void) close(l2_cap_socket_);
  }
}

void L2CapReceiver::OnReceived(std::function<void(const std::span<std::byte>&)> callback) {
  on_received_ = std::move(callback);
}

void L2CapReceiver::OnPacket(std::function<void(const Packet&)> callback) {
  on_packet_ = std::move(callback);
}

void L2CapReceiver::OnError(std::function<void(int, std::string_view)> callback) {
  on_error_ = std::move(callback);
}

void L2CapReceiver::OnConnected(std::function<void()> callback) {
  on_connected_ = std::move(callback);
}

void L2CapReceiver::OnAuthenticated(std::function<void()> callback) {
  on_authenticated_ = std::move(callback);
}

void L2CapReceiver::OnDisconnected(std::function<void()> callback) {
  on_disconnected_ = std::move(callback);
}

void L2CapReceiver::CloseClient(bool notify) {
  if (client_socket_ >= 0) {
    (void) close(client_socket_);
    client_socket_ = -1;
  }
  received_data_.clear();
  connected_controller_.clear();
  auth_state_ = AuthState::kWaiting;
  if (notify && on_disconnected_) {
    on_disconnected_();
  }
}

void L2CapReceiver::CheckClient() {
  if (client_socket_ >= 0) {
    return;
  }

  sockaddr_l2 raddr{};
  socklen_t len = sizeof(raddr);

  const int kFd = accept4(l2_cap_socket_, reinterpret_cast<sockaddr*>(&raddr), &len, SOCK_NONBLOCK);

  if (kFd < 0) {
    if (errno != EAGAIN && errno != EWOULDBLOCK) {
      logger_.LogFmt(LogLevel::WARN, "accept() failed: {}", strerror(errno));
    }
    return;
  }

  std::array<char, 18> addrstr{};
  (void) ba2str(&raddr.l2_bdaddr, addrstr.data());
  const std::string controller =
      std::format("{}/{}", addrstr.data(), static_cast<unsigned int>(raddr.l2_bdaddr_type));
  if (!authorized_controller_) {
    LoadAuthorizedController();
  }
  if (authorized_controller_ && *authorized_controller_ != controller) {
    logger_.LogFmt(LogLevel::WARN, "Rejected non-commissioned controller {}", controller);
    (void) close(kFd);
    return;
  }

  client_socket_ = kFd;
  connected_controller_ = controller;

  l2cap_options options{};
  socklen_t optlen = sizeof(options);
  if (getsockopt(client_socket_, SOL_L2CAP, L2CAP_OPTIONS, &options, &optlen) == 0) {
    imtu_ = std::max<int>(options.imtu, 256);
    omtu_ = std::max<int>(options.omtu, 256);
    logger_.LogFmt(LogLevel::INFO, "Negotiated L2CAP: IMTU={} OMTU={}", imtu_, omtu_);
  } else {
    imtu_ = omtu_ = 672;
  }

  received_data_.reserve(std::max(imtu_ * 16, 256 * 1024));

  logger_.LogFmt(LogLevel::INFO, "Accepted connection from {}", connected_controller_);

  // Generate random nonce and send challenge.
  const ssize_t random_bytes = getrandom(nonce_.data(), nonce_.size(), 0);
  if (random_bytes != static_cast<ssize_t>(nonce_.size())) {
    logger_.LogError("Failed to generate a complete authentication nonce");
    CloseClient(false);
    return;
  }
  const auto kNonceSpan = std::as_bytes(std::span{nonce_});
  if (!SendPacket(kTypeChallenge, "auth-v2", kNonceSpan)) {
    logger_.LogError("Failed to send auth challenge — closing connection");
    CloseClient(false);
    return;
  }

  auth_state_ = AuthState::kChallengeSent;
  challenge_deadline_ = std::chrono::steady_clock::now() + std::chrono::seconds{5};
  logger_.LogInfo("Auth challenge sent — waiting for response");

  if (on_connected_) {
    on_connected_();
  }
}

void L2CapReceiver::PollSocket() {
  CheckClient();

  if (client_socket_ < 0) {
    return;
  }

  // Enforce auth timeout before reading anything.
  if (auth_state_ == AuthState::kChallengeSent &&
      std::chrono::steady_clock::now() > challenge_deadline_) {
    logger_.LogWarn("Auth timeout — closing unauthenticated connection");
    SendError(0x01, "auth timeout");
    CloseClient();
    return;
  }

  pollfd pfd{.fd = client_socket_, .events = POLLIN | POLLERR | POLLHUP | POLLRDHUP};
  const int kR = poll(&pfd, 1, 0);
  if (kR <= 0) {
    return;
  }

  if (pfd.revents & (POLLERR | POLLHUP | POLLRDHUP)) {
    logger_.LogInfo("Client disconnected or socket error");
    CloseClient();
    return;
  }

  if (pfd.revents & POLLIN) {
    ReadAllAvailable();
    while (ExtractOnePacket()) {
    }
  }
}

void L2CapReceiver::ReadAllAvailable() {
  for (;;) {
    const ssize_t kNeed = recv(client_socket_, nullptr, 0, MSG_PEEK | MSG_TRUNC);
    if (kNeed < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        break;
      }
      logger_.LogFmt(LogLevel::ERROR, "recv peek: {}", strerror(errno));
      if (on_error_) {
        on_error_(errno, "peek failed");
      }
      break;
    }
    if (kNeed == 0) {
      logger_.LogInfo("Peer closed connection");
      CloseClient();
      break;
    }

    temp_record_.resize(static_cast<size_t>(kNeed));
    const ssize_t kN = recv(client_socket_, temp_record_.data(), temp_record_.size(), 0);
    if (kN <= 0) {
      if (kN == 0) {
        CloseClient();
      } else {
        logger_.LogFmt(LogLevel::ERROR, "recv: {}", strerror(errno));
        if (on_error_) {
          on_error_(errno, "recv failed");
        }
      }
      break;
    }
    received_data_.insert(received_data_.end(), temp_record_.begin(), temp_record_.begin() + kN);
    const std::size_t max_buffer = auth_state_ == AuthState::kAuthenticated
                                       ? static_cast<std::size_t>(kMaxPayload) + kMaxNameLen + 15U
                                       : 1024U;
    if (received_data_.size() > max_buffer) {
      logger_.LogWarn("Peer exceeded the receive-buffer limit");
      SendError(0x02, "receive buffer limit exceeded");
      CloseClient();
      break;
    }
  }
}

void L2CapReceiver::ValidateAuth(std::span<const uint8_t> payload) {
  if (!auth::ValidateResponse(auth_key_, nonce_, payload)) {
    logger_.LogError("Auth failed — PSK mismatch, closing connection");
    SendError(0x01, "auth failed");
    CloseClient();
    return;
  }

  if (!authorized_controller_ && !PersistController()) {
    logger_.LogError("Authentication succeeded but controller commissioning could not be saved");
    SendError(0x09, "controller commissioning could not be persisted");
    CloseClient();
    return;
  }

  logger_.LogInfo("Client authenticated successfully");
  auth_state_ = AuthState::kAuthenticated;
  if (on_authenticated_) {
    on_authenticated_();
  }
}

void L2CapReceiver::LoadAuthorizedController() {
  std::ifstream file(controller_id_path_);
  std::string controller;
  if (!file || !std::getline(file, controller) || controller.empty()) {
    logger_.LogInfo("Device is not commissioned to a controller");
    return;
  }
  authorized_controller_ = std::move(controller);
  logger_.LogInfo("Loaded the commissioned controller identity");
}

bool L2CapReceiver::PersistController() {
  if (connected_controller_.empty()) {
    return false;
  }

  std::error_code error;
  if (const auto parent = controller_id_path_.parent_path(); !parent.empty()) {
    std::filesystem::create_directories(parent, error);
    if (error) {
      logger_.LogFmt(LogLevel::ERROR, "Could not create controller state directory: {}",
                     error.message());
      return false;
    }
  }

  auto temporary_path = controller_id_path_;
  temporary_path += ".new";
  {
    std::ofstream file(temporary_path, std::ios::trunc);
    if (!file || !(file << connected_controller_ << '\n')) {
      return false;
    }
  }
  std::filesystem::rename(temporary_path, controller_id_path_, error);
  if (error) {
    std::filesystem::remove(temporary_path, error);
    return false;
  }
  authorized_controller_ = connected_controller_;
  logger_.LogInfo("Commissioned the first authenticated controller");
  return true;
}

bool L2CapReceiver::ExtractOnePacket() {
  std::vector<uint8_t>& buf = received_data_;

  if (buf.size() < kHeaderMin) {
    return false;
  }

  size_t off = 0;

  // Scan for magic bytes.
  while (off + 1 < buf.size()) {
    if (socket::FromVector<uint16_t>(buf, off) == kMagic) {
      break;
    }
    ++off;
  }
  if (off > 0) {
    buf.erase(buf.begin(), buf.begin() + off);
  }
  if (buf.size() < kHeaderMin) {
    return false;
  }

  const uint8_t kType = buf[2];
  const auto kNameLen = socket::FromVector<uint32_t>(buf, 3);

  if (kNameLen > kMaxNameLen) {
    logger_.LogFmt(LogLevel::ERROR, "Name too large: {}", kNameLen);
    SendError(0x04, "name too large");
    CloseClient();
    return false;
  }

  const size_t kAfterName = 7 + static_cast<size_t>(kNameLen);
  if (buf.size() < kAfterName) {
    return false;
  }

  std::string name(reinterpret_cast<const char*>(&buf[7]), kNameLen);

  // Commands (type < 0x80) carry only the name, no payload.
  if (kType < 0x80) {
    buf.erase(buf.begin(), buf.begin() + kAfterName);

    if (auth_state_ != AuthState::kAuthenticated) {
      logger_.LogWarn("Closing client that sent a command before authentication");
      SendError(0x01, "authentication required");
      CloseClient();
      return true;
    }

    Packet packet{.type_ = kType, .name_ = std::move(name), .payload_ = {}, .has_payload_ = false};
    if (on_packet_) {
      on_packet_(packet);
    }
    return true;
  }

  // Packets with payload (type >= 0x80).
  if (buf.size() < kAfterName + 8) {
    return false;
  }

  const uint32_t kPayloadLen = socket::FromVector<uint32_t>(buf, kAfterName);

  if (kPayloadLen > kMaxPayload) {
    logger_.LogFmt(LogLevel::ERROR, "Payload too large: {}", kPayloadLen);
    if (on_error_) {
      on_error_(-2, "payload too large");
    }
    SendError(0x02, "payload too large");
    CloseClient();
    return false;
  }

  const size_t kNeed = kAfterName + 8 + static_cast<size_t>(kPayloadLen);
  if (buf.size() < kNeed) {
    return false;
  }

  const uint32_t kCrc32 = socket::FromVector<uint32_t>(buf, kAfterName + 4);
  const uint8_t* kPayloadPtr = &buf[kAfterName + 8];
  const auto kPayloadSpan = std::span(kPayloadPtr, kPayloadLen);
  const uint32_t kCalc = Crc(kPayloadSpan);

  if (kCalc != kCrc32) {
    logger_.LogFmt(LogLevel::ERROR, "CRC mismatch: calc={} pkt={}", kCalc, kCrc32);
    if (on_error_) {
      on_error_(-3, "crc mismatch");
    }
    SendError(0x03, "crc mismatch");
    buf.erase(buf.begin(), buf.begin() + kNeed);
    return true;
  }

  // Handle auth response before anything else.
  if (kType == kTypeAuthResponse && name == "auth-v2") {
    const std::span<const uint8_t> kRespSpan(kPayloadPtr, kPayloadLen);
    buf.erase(buf.begin(), buf.begin() + kNeed);
    if (auth_state_ != AuthState::kChallengeSent) {
      logger_.LogWarn("Authentication response arrived in an invalid state");
      SendError(0x01, "unexpected authentication response");
      CloseClient();
      return true;
    }
    ValidateAuth(kRespSpan);
    return true;
  }

  // Drop all other payloaded packets until authenticated.
  if (auth_state_ != AuthState::kAuthenticated) {
    logger_.LogWarn("Closing client that sent data before authentication");
    buf.erase(buf.begin(), buf.begin() + kNeed);
    SendError(0x01, "authentication required");
    CloseClient();
    return true;
  }

  current_payload_.assign(kPayloadPtr, kPayloadPtr + kPayloadLen);
  buf.erase(buf.begin(), buf.begin() + kNeed);

  Packet view{
      .type_ = kType,
      .name_ = std::move(name),
      .payload_ = std::span(reinterpret_cast<const std::byte*>(current_payload_.data()),
                            current_payload_.size()),
      .has_payload_ = true,
  };

  if (on_packet_) {
    on_packet_(view);
  }
  if (on_received_) {
    on_received_(
        std::span(reinterpret_cast<std::byte*>(current_payload_.data()), current_payload_.size()));
  }
  return true;
}

bool L2CapReceiver::SendPacket(const uint8_t kType, const std::string_view kName,
                               std::span<const std::byte> payload) {
  if (client_socket_ < 0) {
    logger_.LogWarn("SendPacket: no client connected");
    return false;
  }

  std::vector<std::byte> bytes;
  socket::BuildPacketBytes(kType, kMagic, kName, payload, bytes);

  // Send in chunks no larger than the negotiated output MTU.
  const size_t kChunkSize = static_cast<size_t>(std::max(omtu_, 512));
  size_t offset = 0;
  while (offset < bytes.size()) {
    const size_t kChunk = std::min(bytes.size() - offset, kChunkSize);
    const ssize_t kSent = send(client_socket_, bytes.data() + offset, kChunk, MSG_NOSIGNAL);
    if (kSent < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
      pollfd descriptor{.fd = client_socket_, .events = POLLOUT, .revents = 0};
      if (poll(&descriptor, 1, 2000) > 0) {
        continue;
      }
      logger_.LogError("send timed out waiting for socket capacity");
      return false;
    }
    if (kSent <= 0) {
      logger_.LogFmt(LogLevel::ERROR, "send: {}", strerror(errno));
      if (on_error_) {
        on_error_(errno, "send failed");
      }
      return false;
    }
    offset += static_cast<size_t>(kSent);
  }
  return true;
}

bool L2CapReceiver::SendError(const uint32_t kErrCode, const std::string_view kMessage,
                              const uint8_t kType) {
  const std::string kText = std::format("ERR:{}:{}", kErrCode, kMessage);
  const std::span kPayload(reinterpret_cast<const std::byte*>(kText.data()), kText.size());
  return SendPacket(kType, "error", kPayload);
}

void L2CapReceiver::TryEnable2MDefaultPhy() {
  const int kDevId = hci_get_route(nullptr);
  if (kDevId < 0) {
    return;
  }
  const int kDd = hci_open_dev(kDevId);
  if (kDd < 0) {
    return;
  }

  struct __attribute__((packed)) Data {
    uint8_t all_phys_;
    uint8_t tx_phys_;
    uint8_t rx_phys_;
  };
  Data cp{.all_phys_ = 0x00, .tx_phys_ = 0x02, .rx_phys_ = 0x02};
  uint8_t status = 0;
  hci_request rq{.ogf = OGF_LE_CTL,
                 .ocf = 0x0031,
                 .cparam = &cp,
                 .clen = sizeof(cp),
                 .rparam = &status,
                 .rlen = sizeof(status)};

  if (hci_send_req(kDd, &rq, 1000) < 0 || status != 0x00) {
    logger_.LogInfo("LE 2M PHY not available");
  } else {
    logger_.LogInfo("LE PHY set to prefer 2M");
  }
  hci_close_dev(kDd);
}

}  // namespace screen_controller::socket
