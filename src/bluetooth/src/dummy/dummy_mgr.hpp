#pragma once

#include <bt/manager.hpp>

namespace screen_controller::bluetooth {

class DummyManager : public IBluetoothManager {
public:
  DummyManager(const DummyManager&) = default;
  DummyManager(DummyManager&&) = default;
  DummyManager& operator=(const DummyManager&) = default;
  DummyManager& operator=(DummyManager&&) = default;

  virtual ~DummyManager() override;

  virtual void Poll() override;

  static std::expected<std::unique_ptr<DummyManager>, std::error_code> Create();

private:
    explicit DummyManager();
};

}  // namespace screen_controller::bluetooth
