//
// Created by marce on 5/8/2025.
//

#include "l2cap_receiver.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/l2cap.h>
#include <fcntl.h>
#include <ng-log/logging.h>
#include <sys/poll.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <functional>
#include <iostream>
#include <vector>

#include "../../../external/stb/stb_image.h"
#include "socket_helper/socket_helper.h"
#include "socket_implementations/socket_options.h"

namespace screen_controller::bluetooth {

constexpr uint16_t kMagic = 0xBEEF;
constexpr size_t kHeaderMin = 2 + 1 + 4;
constexpr uint32_t kMaxNameLen = 1u << 20;
constexpr uint32_t kMaxPayload = 16u << 20;

L2CapReceiver::L2CapReceiver() : l2_cap_socket_(-1), client_socket_(-1) {
  LOG(INFO) << "Creating L2CapReceiver";
}

L2CapReceiver::~L2CapReceiver() {
  LOG(INFO) << "Cleaning up L2CapReceiver";
  if (client_socket_ >= 0) {
    (void)close(client_socket_);
  }
  if (l2_cap_socket_ >= 0) {
    (void)close(l2_cap_socket_);
  }
}

void L2CapReceiver::OnReceived(
    const std::function<void(const std::span<std::byte>& data)>& callback) {
  on_received_ = callback;
}

void L2CapReceiver::OnPacket(
    const std::function<void(const packet&)> callback) {
  on_packet_ = callback;
}

void L2CapReceiver::OnError(
    const std::function<void(int code, std::string_view message)>& callback) {
  on_error_ = callback;
}

bool L2CapReceiver::init() {
  LOG(INFO) << "Initializing L2CapReceiver";
  l2_cap_socket_ = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
  CHECK(l2_cap_socket_ >= 0)
      << "Failed to create l2cap socket:" << strerror(errno);

  SocketOptions::SetReceiveBufferSize(l2_cap_socket_, 256 * 1024);
  SocketOptions::SetSendBufferSize(l2_cap_socket_, 256 * 1024);
  SocketOptions::SetReuseAddress(l2_cap_socket_, 1);
  SocketOptions::SetFlushable(l2_cap_socket_, 1);
  SocketOptions::SetNonBlocking(l2_cap_socket_);

  l2cap_options options{};
  socklen_t optlen = sizeof(options);

  if (getsockopt(l2_cap_socket_, SOL_L2CAP, L2CAP_OPTIONS, &options, &optlen) ==
      0) {
    options.imtu = std::max<int>(options.imtu, 2048);
    options.omtu = std::max<int>(options.omtu, 2048);

    CHECK(setsockopt(l2_cap_socket_, SOL_L2CAP, L2CAP_OPTIONS, &options,
                     sizeof(options)) == 0)
        << "Failed to set L2CAP options: " << strerror(errno);

  } else {
    LOG(WARNING) << "getsockopt(L2CAP_OPTIONS) failed, using default options";
  }

  sockaddr_l2 loc_addr = {
      .l2_family = AF_BLUETOOTH,
      .l2_psm = htobs(0x0081),
      .l2_bdaddr_type = BDADDR_LE_PUBLIC,
  };

  if (bind(l2_cap_socket_, reinterpret_cast<sockaddr*>(&loc_addr),
           sizeof(loc_addr)) < 0) {
    LOG(ERROR) << "Failed to bind l2cap socket with error: " << strerror(errno);
    return false;
  }

  if (listen(l2_cap_socket_, 1) < 0) {
    LOG(ERROR) << "Failed to listen on l2cap socket" << strerror(errno);
    return false;
  }

  TryEnable2MDefaultPhy();

  return true;
}

void L2CapReceiver::CheckClient() {
  if (client_socket_ >= 0) {
    return;
  }

  sockaddr_l2 raddr{};

  socklen_t len = sizeof(raddr);

  const int fd = accept4(l2_cap_socket_, reinterpret_cast<sockaddr*>(&raddr),
                         &len, SOCK_NONBLOCK);

  if (fd < 0) {
    if (errno != EAGAIN && errno != EWOULDBLOCK) {
      LOG(WARNING) << "accept() failed";
    }
    return;
  }

  client_socket_ = fd;

  std::array<char, 18> addrstr{};
  (void)ba2str(&raddr.l2_bdaddr, addrstr.data());
  LOG(INFO) << "Accepted connection from " << batostr(&raddr.l2_bdaddr);

  l2cap_options options{};
  socklen_t optlen = sizeof(options);
  if (getsockopt(client_socket_, SOL_L2CAP, L2CAP_OPTIONS, &options, &optlen) ==
      0) {
    imtu_ = std::max<int>(options.imtu, 256);
    omtu_ = std::max<int>(options.omtu, 256);
    LOG(INFO) << "Negotiated L2CAP: IMTU=" << imtu_ << " OMTU=" << omtu_
              << " mode=" << int(options.mode);
  } else {
    LOG(WARNING) << "getsockopt(L2CAP_OPTIONS) failed on client: "
                 << strerror(errno) << " — defaulting to 672";
    imtu_ = omtu_ = 672;
  }

  received_buffer_.resize(imtu_);
  received_data_.reserve(std::max<int>(imtu_ * 8, 64 * 1024));
}

void L2CapReceiver::poll_socket() {
  CheckClient();

  if (client_socket_ < 0) {
    return;
  }

  pollfd pfd{.fd = client_socket_,
             .events = POLLIN | POLLERR | POLLHUP | POLLRDHUP};

  const int r = poll(&pfd, 1, 0);
  if (r <= 0) {
    return;
  }

  if (pfd.revents & (POLLERR | POLLHUP | POLLRDHUP)) {
    LOG(INFO) << "Client closed or error";
    (void)close(client_socket_);
    client_socket_ = -1;
    received_data_.clear();
  }

  if (pfd.revents & POLLIN) {
    ReadAllAvailable();
    (void)ExtractOnePacket();
  }
}

void L2CapReceiver::ReadAllAvailable() {
  for (;;) {
    const ssize_t need = recv(client_socket_, nullptr, 0, MSG_PEEK);
    if (need < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        break;
      }
      LOG(ERROR) << "recv :" << strerror(errno);
      if (on_error_) {
        on_error_(errno, "peek failed");
      }

      break;
    }
    if (need == 1) {
      LOG(INFO) << "Peer initiated close";
      (void)close(client_socket_);
      client_socket_ = -1;
      break;
    }
    temp_record_.resize(static_cast<size_t>(need));
    const ssize_t n =
        recv(client_socket_, temp_record_.data(), temp_record_.size(), 0);
    if (n <= 0) {
      if (n == 0) {
        LOG(INFO) << "Peer closed during read";
        (void)close(client_socket_);
        client_socket_ = -1;
      } else {
        LOG(ERROR) << "recv(): " << strerror(errno);
        if (on_error_) {
          on_error_(errno, "recv failed");
        }
      }
      break;
    }
    received_data_.insert(received_data_.end(), temp_record_.begin(),
                          temp_record_.end());
  }
}

bool L2CapReceiver::ExtractOnePacket() {
  auto& buf = received_buffer_;

  if (buf.size() < kHeaderMin) {
    return false;
  }

  size_t off = 0;

  while (off + 1 < buf.size()) {
    if (SocketHelper::le16(&buf[off]) == kMagic) {
      break;
    }
    off++;
  }

  if (off>0) {
    (void)buf.erase(buf.begin(), buf.begin() + off);
  }

  if (buf.size() < kHeaderMin) {
    return false;
  }

  if (SocketHelper::le16(&buf[0]) != kMagic) {
    return false;
  }

  const uint8_t type = buf[2];
  const uint32_t name_len = SocketHelper::le32(&buf[3]);

  if (name_len > kMaxNameLen) {
    LOG(ERROR) << "Name too large: " << name_len;
    if (on_error_) {
      on_error_(-1, "name too long");
    }
    (void)buf.erase(buf.begin());
    return false;
  }
  const size_t after_name = 7 + static_cast<size_t>(name_len);
  if (buf.size() < after_name) {
    return false;
  }
  const auto name_ptr = reinterpret_cast<const char*>(&buf[7]);
  std::string name(name_ptr, name_len);
  if (buf.size(), after_name + 4) {
    return false;
  }
  const uint32_t payload_len =
      (buf.size() >= after_name + 4) ? SocketHelper::le32(&buf[after_name]) : 0;
  if (buf.size() >= after_name && buf.size() < after_name + 8) {
    return false;
  }
  if (auto header_only = [&](const uint8_t t) -> bool { return (t < 0x80); };
      header_only(type)) {
    (void)buf.erase(buf.begin(), buf.begin() + after_name);
    packet packet{.type = type,
                  .name = std::move(name),
                  .payload = {},
                  .crc32 = 0,
                  .has_payload = false};

    if (on_packet_) {
      on_packet_(packet);
    }
    if (on_received_) {
      current_payload_.assign(packet.name.begin(), packet.name.end());
      std::span<std::byte> s(
          reinterpret_cast<std::byte*>(current_payload_.data()),
          current_payload_.size());
      on_received_(s);
    }
    return true;
  }
  const size_t need = after_name + 8 + static_cast<size_t>(payload_len);
  if (payload_len > kMaxPayload) {
    LOG(ERROR) << "Payload too large: " << payload_len;
    if (on_error_) {
      on_error_(-2, "payload too large");
    }

    (void)buf.erase(buf.begin());
    return false;
  }
  if (buf.size() < need) {
    return false;
  }

  const uint32_t crc32 = SocketHelper::le32(&buf[after_name + 4]);
  const uint8_t* payload_ptr = &buf[after_name + 8];

  if (const uint32_t calc = SocketHelper::Crc32(payload_ptr, payload_len);
      calc != crc32) {
    LOG(ERROR) << "CRC mismatch: calc=" << calc << " pkt=" << crc32;
    if (on_error_) {
      on_error_(-3, "crc mismatch");
    }
    (void)buf.erase(buf.begin(), buf.begin() + need);
    return true;
  }

  current_payload_.assign(payload_ptr, payload_ptr + payload_len);
  (void)buf.erase(buf.begin(), buf.begin() + need);

  packet view{
      .type = type,
      .name = std::move(name),
      .payload =
          std::span(reinterpret_cast<const std::byte*>(current_payload_.data()),
                    current_payload_.size()),
      .crc32 = crc32,
      .has_payload = true,
  };

  if (on_packet_) {
    on_packet_(view);
  }

  if (on_received_) {
    on_received_(
        std::span(reinterpret_cast<std::byte*>(current_payload_.data()),
                  current_payload_.size()));
  }
  return true;
}

bool L2CapReceiver::SendPacket(const uint8_t type, const std::string_view name,
                               std::span<const std::byte> payload) {
  if (client_socket_ < 0) {
    return false;
  }

  std::vector<uint8_t> bytes{};
  SocketHelper::BuildPacketBytes(type, kMagic, name, payload, bytes);

  const ssize_t n =
      send(client_socket_, bytes.data(), bytes.size(), MSG_NOSIGNAL);
  if (n < 0) {
    LOG(ERROR) << "send(): " << strerror(errno);
    if (on_error_) {
      on_error_(errno, "send failed");
    }
    return false;
  }
  return static_cast<size_t>(n) == bytes.size();
}

bool L2CapReceiver::SendError(const uint32_t err_code,
                              const std::string_view message,
                              const uint8_t type) {
  const std::string text =
      "ERR:" + std::to_string(err_code) + ":" + std::string(message);
  const std::span payload(reinterpret_cast<const std::byte*>(text.data()),
                          text.size());
  return SendPacket(type, "error", payload);
}

void L2CapReceiver::TryEnable2MDefaultPhy() {
  const int dev_id = hci_get_route(nullptr);
  if (dev_id < 0) {
    return;
  }

  const int dd = hci_open_dev(dev_id);
  if (dd < 0) {
    return;
  }

  struct __attribute__((packed)) {
    uint8_t all_phys;
    uint8_t tx_phys;
    uint8_t rx_phys;
  } cp{};

  cp.all_phys = 0x00;
  cp.tx_phys = 0x02;
  cp.rx_phys = 0x02;

  uint8_t status = 0;

  hci_request rq{
      .ogf = OGF_LE_CTL,
      .ocf = 0x0031,
      .cparam = &cp,
      .clen = sizeof(cp),
      .rparam = &status,
      .rlen = sizeof(status),
  };

  if (const int ret = hci_send_req(dd, &rq, 1000); ret < 0 || status != 0x00) {
    LOG(INFO) << "LE Set Default PHY (2M) not enabled (ret=" << ret
              << ", status=0x" << std::hex << static_cast<int>(status)
              << std::dec << ")";
  } else {
    LOG(INFO) << "Default LE PHY set to prefer 2M.";
  }

  hci_close_dev(dd);
}

}  // namespace screen_controller::bluetooth