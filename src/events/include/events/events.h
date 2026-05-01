#ifndef EVENTS_H
#define EVENTS_H

#include "app_settings.h"
#include "logging/logger.h"

namespace screen_controller {

class IMediatorManager {
public:
  virtual ~IMediatorManager() = default;
};

class MediatorFactory {
public:
  static std::unique_ptr<IMediatorManager> Create(ILogger& logger, const AppSettings& settings);
};

}  // namespace screen_controller
#endif
