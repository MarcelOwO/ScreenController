//
// Created by marce on 5/8/2025.
//

#include "l2cap_receiver.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/l2cap.h>
#include <fcntl.h>
#include <logging/logger.h>
#include <sys/poll.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <functional>
#include <vector>

#include "socket_helper/socket_helper.h"
#include "socket_implementations/socket_options.h"

namespace screen_controller {

L2CapReceiver::L2CapReceiver(ILogger& logger, const AppSettings& settings)
    : socket_options_(logger),
      logger_(logger),
      settings_(settings),
      imtu_(settings.imtu_),
      omtu_(settings.omtu_) {
  logger_.LogInfo("Creating L2CapReceiver");

  l2_cap_socket_ = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);

  if (l2_cap_socket_ < 0) {
    logger_.LogError("Failed to create l2cap socket: {}", std::string(strerror(errno)));
    throw std::runtime_error("Failed to create l2cap socket: {}", std::string(strerror(errno)));
  }

  auto res = socket_options_.SetReceiveBufferSize(l2_cap_socket_, 256 * 1024);
  if (!res) {
    logger_.LogError("Failed to set receive buffer size");
    throw std::runtime_error("Failed to set receive buffer size");
  }

  res = socket_options_.SetSendBufferSize(l2_cap_socket_, 256 * 1024);
  if (!res) {
    logger_.LogError("Failed to set send buffer size");
    throw std::runtime_error("Failed to set send buffer size");
  }

  res = socket_options_.SetReuseAddress(l2_cap_socket_, 1);
  if (!res) {
    logger_.LogError("Failed to set reuse address");
    throw std::runtime_error("Failed to set reuse address");
  }

  res = socket_options_.SetFlushable(l2_cap_socket_, 1);
  if (!res) {
    logger_.LogError("Failed to set flushable");
    throw std::runtime_error("Failed to set flushable");
  }

  res = socket_options_.SetNonBlocking(l2_cap_socket_);
  if (!res) {
    logger_.LogError("Failed to set none blocking");
    throw std::runtime_error("Failed to set none blocking");
  }

  l2cap_options options{};

  socklen_t optlen = sizeof(options);

  if (getsockopt(l2_cap_socket_, SOL_L2CAP, L2CAP_OPTIONS, &options, &optlen) == 0) {
    options.imtu = std::max<int>(options.imtu, 2048);
    options.omtu = std::max<int>(options.omtu, 2048);

    if (setsockopt(l2_cap_socket_, SOL_L2CAP, L2CAP_OPTIONS, &options, sizeof(options)) != 0) {
      logger_.LogError("Failed to set L2cap options: {}", std::string(strerror(errno)));
      throw std::runtime_error("Failed to set L2cap options: {}", std::string(strerror(errno)));
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
    logger_.LogError("Failed to bind l2cap socket with error: {}", std::string(strerror(errno)));
    throw std::runtime_error("Failed to bind l2cap socket with error: {}",
                             std::string(strerror(errno)));
  }

  if (listen(l2_cap_socket_, 1) < 0) {
    logger_.LogError("Failed to listen on l2cap socket" + std::string(strerror(errno)));
    throw std::runtime_error("Failed to listen on l2cap socket: {}", std::string(strerror(errno)));
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

void L2CapReceiver::OnReceived(
    const std::function<void(const std::span<std::byte>& data)>& callback) {
  on_received_ = callback;
}

void L2CapReceiver::OnPacket(const std::function<void(const Packet&)> kCallback) {
  on_packet_ = kCallback;
}

void L2CapReceiver::OnError(
    const std::function<void(int code, std::string_view message)>& callback) {
  on_error_ = callback;
}

void L2CapReceiver::CheckClient() {
  if (client_socket_ >= 0) {
    return;
  }

  sockaddr_l2 raddr{};

  socklen_t len = sizeof(raddr);

  const int fd = accept4(l2_cap_socket_, reinterpret_cast<sockaddr*>(&raddr), &len, SOCK_NONBLOCK);

  if (fd < 0) {
    if (errno != EAGAIN && errno != EWOULDBLOCK) {
      logger_.LogInfo(" accept() failed ");
    }
    return;
  }

  client_socket_ = fd;

  std::array<char, 18> addrstr{};
  (void) ba2str(&raddr.l2_bdaddr, addrstr.data());
  logger_.LogInfo("Accepted connection from " + std::string(batostr(&raddr.l2_bdaddr)));

  l2cap_options options{};
  socklen_t optlen = sizeof(options);
  if (getsockopt(client_socket_, SOL_L2CAP, L2CAP_OPTIONS, &options, &optlen) == 0) {
    imtu_ = std::max<int>(options.imtu, 256);
    omtu_ = std::max<int>(options.omtu, 256);
    logger_.LogInfo("Negotiated L2CAP: IMTU=" + std::to_string(imtu_) + " OMTU=" +
                    std::to_string(omtu_) + " mode=" + std::to_string(int(options.mode)));
  } else {
    logger_.LogInfo("getsockopt(L2CAP_OPTIONS) failed on client: " + std::string(strerror(errno)) +
                    " — defaulting to 672");
    imtu_ = omtu_ = 672;
  }

  received_buffer_.resize(imtu_);
  received_data_.reserve(std::max<int>(imtu_ * 8, 64 * 1024));
}

void L2CapReceiver::PollSocket() {
  CheckClient();

  if (client_socket_ < 0) {
    return;
  }

  pollfd pfd{.fd = client_socket_, .events = POLLIN | POLLERR | POLLHUP | POLLRDHUP};

  const int r = poll(&pfd, 1, 0);
  if (r <= 0) {
    return;
  }

  if (pfd.revents & (POLLERR | POLLHUP | POLLRDHUP)) {
    logger_.LogInfo("Client closed or error");
    (void) close(client_socket_);
    client_socket_ = -1;
    received_data_.clear();
  }

  if (pfd.revents & POLLIN) {
    ReadAllAvailable();
    (void) ExtractOnePacket();
  }
}

void L2CapReceiver::ReadAllAvailable() {
  for (;;) {
    const ssize_t need = recv(client_socket_, nullptr, 0, MSG_PEEK);
    if (need < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        break;
      }
      logger_.LogError("recv :" + std::string(strerror(errno)));
      if (on_error_) {
        on_error_(errno, "peek failed");
      }

      break;
    }
    if (need == 1) {
      logger_.LogInfo("Peer initiated close");
      (void) close(client_socket_);
      client_socket_ = -1;
      break;
    }
    temp_record_.resize(static_cast<size_t>(need));
    const ssize_t n = recv(client_socket_, temp_record_.data(), temp_record_.size(), 0);
    if (n <= 0) {
      if (n == 0) {
        logger_.LogInfo("Peer closed during read");
        (void) close(client_socket_);
        client_socket_ = -1;
      } else {
        logger_.LogError("recv(): " + std::string(strerror(errno)));
        if (on_error_) {
          on_error_(errno, "recv failed");
        }
      }
      break;
    }
    received_data_.insert(received_data_.end(), temp_record_.begin(), temp_record_.end());
  }
}

bool L2CapReceiver::ExtractOnePacket() {
  auto& buf = received_buffer_;

  if (buf.size() < kHeaderMin) {
    return false;
  }

  size_t off = 0;

  while (off + 1 < buf.size()) {
    if (socket::Le16(&buf[off]) == kMagic) {
      break;
    }
    off++;
  }

  if (off > 0) {
    (void) buf.erase(buf.begin(), buf.begin() + off);
  }

  if (buf.size() < kHeaderMin) {
    return false;
  }

  if (socket::Le16(&buf) != kMagic) {
    return false;
  }

  const uint8_t kType = buf[2];
  const uint32_t kNameLen = socket::Le32(&buf[3]);

  if (kNameLen > kMaxNameLen) {
    logger_.LogError("Name too large: " + std::to_string(kNameLen));
    if (on_error_) {
      on_error_(-1, "name too long");
    }
    (void) buf.erase(buf.begin());
    return false;
  }

  const size_t kAfterName = 7 + static_cast<size_t>(kNameLen);

  if (buf.size() < kAfterName) {
    return false;
  }

  const auto* const kNamePtr = reinterpret_cast<const char*>(&buf[7]);

  std::string name(kNamePtr, kNameLen);

  if (buf.size(), kAfterName + 4) {
    return false;
  }

  const uint32_t payload_len = (buf.size() >= kAfterName + 4) ? socket::Le32(&buf[kAfterName]) : 0;
  if (buf.size() >= kAfterName && buf.size() < kAfterName + 8) {
    return false;
  }
  if (auto header_only = [&](const uint8_t t) -> bool { return (t < 0x80); }; header_only(kType)) {
    (void) buf.erase(buf.begin(), buf.begin() + kAfterName);
    Packet packet{
        .type = kType, .name = std::move(name), .payload = {}, .crc32 = 0, .has_payload = false};

    if (on_packet_) {
      on_packet_(packet);
    }
    if (on_received_) {
      current_payload_.assign(packet.name.begin(), packet.name.end());
      const std::span s(reinterpret_cast<std::byte*>(current_payload_.data()),
                        current_payload_.size());
      on_received_(s);
    }
    return true;
  }
  const size_t need = kAfterName + 8 + static_cast<size_t>(payload_len);
  if (payload_len > kMaxPayload) {
    logger_.LogError("Payload too large: " + payload_len);
    if (on_error_) {
      on_error_(-2, "payload too large");
    }

    (void) buf.erase(buf.begin());
    return false;
  }
  if (buf.size() < need) {
    return false;
  }

  const uint32_t kCrc32 = socket::Le32(&buf[kAfterName + 4]);

  const uint8_t* payload_ptr = &buf[kAfterName + 8];

  if (const uint32_t kCalc = socket::Crc32(payload_ptr, payload_len); kCalc != kCrc32) {
    logger_.LogError("CRC mismatch: calc=" + std::to_string(kCalc) +
                     " pkt=" + std::to_string(kCrc32));
    if (on_error_) {
      on_error_(-3, "crc mismatch");
    }
    (void) buf.erase(buf.begin(), buf.begin() + need);
    return true;
  }

  current_payload_.assign(payload_ptr, payload_ptr + payload_len);
  (void) buf.erase(buf.begin(), buf.begin() + need);

  Packet view{
      .type_ = kType,
      .name_ = std::move(name),
      .payload_ = std::span(reinterpret_cast<const std::byte*>(current_payload_.data()),
                            current_payload_.size()),
      .crc32_ = kCrc32,
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
    logger_.LogError("No client connected");
    return false;
  }

  std::vector<uint8_t> bytes{};

  socket::BuildPacketBytes(kType, kMagic, kName, payload, bytes);

  const ssize_t kSentBytes = send(client_socket_, bytes.data(), bytes.size(), MSG_NOSIGNAL);
  if (kSentBytes < 0) {
    logger_.LogInfo("send():" + std::string(strerror(errno)));
    if (on_error_) {
      on_error_(errno, "send failed");
    }
    return false;
  }
  return static_cast<size_t>(kSentBytes) == bytes.size();
}

bool L2CapReceiver::SendError(const uint32_t kErrCode, const std::string_view kMessage,
                              const uint8_t kType) {
  const std::string kText = "ERR:" + std::to_string(kErrCode) + ":" + std::string(kMessage);
  const std::span kPayload(reinterpret_cast<const std::byte*>(kText.data()), kText.size());
  return SendPacket(kType, "error", kPayload);
}

void L2CapReceiver::TryEnable2MDefaultPhy() {
  const int kDevId = hci_get_route(nullptr);

  if (kDevId < 0) {
    return;
  }

  const int dd = hci_open_dev(kDevId);
  if (dd < 0) {
    return;
  }

  struct __attribute__((packed)) Data {
    uint8_t all_phys_;
    uint8_t tx_phys_;
    uint8_t rx_phys_;
  };

  Data cp{
      .all_phys_ = 0x00,
      .tx_phys_ = 0x02,
      .rx_phys_ = 0x02,
  };

  uint8_t status = 0;

  hci_request rq{
      .ogf = OGF_LE_CTL,
      .ocf = 0x0031,
      .cparam = &cp,
      .clen = sizeof(cp),
      .rparam = &status,
      .rlen = sizeof(status),
  };

  if (const int kRet = hci_send_req(dd, &rq, 1000); kRet < 0 || status != 0x00) {
    logger_.LogInfo("LE Set Default PHY (2M) not enabled (ret=" + std::to_string(kRet) +
                    ", status=0x" + std::to_string(status) + +")");
  } else {
    logger_.LogInfo("Default LE PHY set to prefer 2M.");
  }

  hci_close_dev(dd);
}

}  // namespace screen_controller
