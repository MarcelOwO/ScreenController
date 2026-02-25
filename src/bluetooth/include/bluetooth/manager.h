#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

#include <logging/logger.h>

namespace screen_controller {

class IBluetoothManager {
 public:
  virtual ~IBluetoothManager() = default;
};

class BluetoothFactory {
 public:
  static std::unique_ptr<IBluetoothManager> Create();
};

}  // namespace screen_controller

#endif  //
