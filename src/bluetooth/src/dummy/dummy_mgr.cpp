#include "dummy_mgr.hpp"

namespace screen_controller::bluetooth {

DummyManager::~DummyManager() = default;
DummyManager::DummyManager() = default;

void DummyManager::Poll() {}

bool DummyManager::SendPacket(uint8_t /*type*/, std::string_view /*name*/,
                              std::span<const std::byte> /*payload*/) {
  return false;
}

std::expected<std::unique_ptr<DummyManager>, std::error_code> DummyManager::Create() {
  auto bluetooth_manager = std::unique_ptr<DummyManager>(new DummyManager());

  return bluetooth_manager;
}

}  // namespace screen_controller::bluetooth
