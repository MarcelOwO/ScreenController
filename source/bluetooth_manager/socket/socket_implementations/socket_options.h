//
// Created by marce on 06/08/2025.
//

#ifndef SOCKETOPTIONS_H
#define SOCKETOPTIONS_H

namespace screen_controller::bluetooth {
class SocketOptions {
public:
  static bool SetSocketOption(int fd, int level, int option, int value) ;
  static void SetReuseAddress(int fd, int value);
  static void SetReceiveBufferSize(int fd, int value);
  static void SetSendBufferSize(int fd, int value);
  static void SetFlushable(int fd, int value);
  static void SetDefer(int fd, int value);
  static void SetNonBlocking(int socket);
};
}

#endif //SOCKETOPTIONS_H
