//
// Created by marce on 5/8/2025.
//

#include "l_2_cap_receiver.h"

#include <fcntl.h>
#include <ng-log/logging.h>
#include <sys/poll.h>
#include <unistd.h>

#include <array>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <vector>

#include "../../../external/stb/stb_image.h"

namespace screen_controller::bluetooth {

L2CapReceiver::L2CapReceiver()
    : received_data_(), l2_cap_socket_(-1), client_socket_(-1) {
  LOG(INFO) << "Creating L2CapReceiver";
}

L2CapReceiver::~L2CapReceiver() {
  PLOG(INFO) << "Cleaning up L2CapReceiver";
  if (client_socket_ >= 0) {
    (void)close(client_socket_);
  }
  if (l2_cap_socket_ >= 0) {
    (void)close(l2_cap_socket_);
  }
}

void L2CapReceiver::OnReceived(
    std::function<void(const std::span<std::byte>)> callback) {
  on_received_ = callback;
}

bool L2CapReceiver::init() {
  l2_cap_socket_ = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
  PCHECK(l2_cap_socket_ >= 0) << "Failed to create l2cap socket";

  SetReuseAddress(1);
  SetReceiveBufferSize(65535 / 2);
  SetSendBufferSize(65535 / 2);
  SetFlushable(0);
  SetNonBlocking();

  sockaddr_l2 loc_addr = {};
  loc_addr.l2_family = AF_BLUETOOTH;
  loc_addr.l2_bdaddr = {{0, 0, 0, 0, 0, 0}};
  loc_addr.l2_bdaddr_type = BDADDR_LE_PUBLIC;
  loc_addr.l2_cid = htobs(0);
  loc_addr.l2_psm = htobs(0x0081);

  PCHECK(bind(l2_cap_socket_, (sockaddr*)&loc_addr, sizeof(loc_addr)) >= 0)
      << "Failed to bind l2cap socket with error: " << strerror(errno);

  if (listen(l2_cap_socket_, 1) < 0) {
    PLOG(ERROR) << "Failed to listen on l2cap socket" << strerror(errno);
    return false;
  }

  return true;
}

void L2CapReceiver::run() {
  if (client_socket_ < 0) {
    sockaddr_l2 rem_addr{};
    socklen_t opt = sizeof(rem_addr);
    client_socket_ =
        accept(l2_cap_socket_, reinterpret_cast<sockaddr*>(&rem_addr), &opt);

    if (client_socket_ < 0) {
      return;
    }

    LOG(INFO) << "Accepted connection from " << batostr(&rem_addr.l2_bdaddr);

    transfer_complete_ = false;
    SetNonBlocking();
  }
  pollfd fds{.fd = client_socket_, .events = POLLIN};
  if (poll(&fds, 1, 0) > 0 && (fds.revents & POLLIN)) {
    int read_bytes = 0;
    LOG(INFO) << "Data available to read on client socket";
    std::array<uint8_t, 65335> buffer{};
    while (true) {
      buffer.fill(0);

      const ssize_t len = ::read(client_socket_, buffer.data(), buffer.size());
      read_bytes += len;

      if (len > 0) {
        for (int i = 0; i < len; ++i) {
          received_data_.push_back(buffer[i]);
        }
        LOG(INFO) << "Received chunk: " << len << " bytes";
      } else if (len == 0) {
        LOG(INFO) << "Sender disconnected cleanly.";
        transfer_complete_ = true;
        break;
      } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
        LOG(INFO) << "waiting for data";
        break;
      } else {
        PLOG(ERROR) << "Read error: " << strerror(errno);
        transfer_complete_ = true;
        break;
      }
    }
    if (transfer_complete_) {
      LOG(INFO) << "Transfer complete, total bytes received: " << read_bytes;
      const std::span<std::byte> s(
          reinterpret_cast<std::byte*>(received_data_.data()),
          received_data_.size());

      on_received_(std::move(s));

      received_data_.clear();

      if (client_socket_ >= 0) {
        client_socket_ = -1;
      }
      transfer_complete_ = false;
    }
  }
}

bool L2CapReceiver::SetSocketOption(const int level, const int option,
                                    const int value) const {
  const int val =
      setsockopt(l2_cap_socket_, level, option, &value, sizeof(int));
  return val >= 0;
}

void L2CapReceiver::SetReuseAddress(const int value) {
  PCHECK(SetSocketOption(SOL_SOCKET, SO_REUSEADDR, value))
      << "Failed to set socket to reuse address";
}

void L2CapReceiver::SetReceiveBufferSize(const int value) {
  PCHECK(SetSocketOption(SOL_SOCKET, SO_RCVBUF, value))
      << "Failed to set socket receive buffer size";
}

void L2CapReceiver::SetSendBufferSize(const int value) {
  PCHECK(SetSocketOption(SOL_SOCKET, SO_SNDBUF, value))
      << "Failed to set socket send buffer size";
}
void L2CapReceiver::SetFlushable(const int value) {
  PCHECK(SetSocketOption(SOL_SOCKET, BT_FLUSHABLE, value))
      << "Failed to set socket flushable";
}
void L2CapReceiver::SetDefer(const int value) {
  PCHECK(SetSocketOption(SOL_SOCKET, BT_DEFER_SETUP, value))
      << "Failed to set socket defer";
}
void L2CapReceiver::SetNonBlocking() {
  PCHECK(fcntl(l2_cap_socket_, F_SETFL, O_NONBLOCK) >= 0)
      << "Failed to set non-blocking mode";
}

}  // namespace screen_controller::bluetooth