#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include <events/events.h>

namespace screen_controller {

class MediatorManager : IMediatorManager {
public:
  ~MediatorManager() override;

  static std::expected<std::unique_ptr<MediatorManager>, std::error_code> Create(
      ILogger& logger, const AppSettings& settings);

  MediatorManager(const MediatorManager&) = delete;
  MediatorManager(MediatorManager&&) = delete;
  MediatorManager& operator=(const MediatorManager&) = delete;
  MediatorManager& operator=(MediatorManager&&) = delete;
};

}  // namespace screen_controller

#endif
