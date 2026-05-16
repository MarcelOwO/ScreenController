#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include <events/events.h>
#include <expected>

namespace screen_controller {

class EventManager : public IEventManager {
public:
  ~EventManager() override;

  static std::expected<std::unique_ptr<EventManager>, std::error_code> Create(
      ILogger& logger, const AppSettings& settings);

  EventManager(const EventManager&) = delete;
  EventManager(EventManager&&) = delete;
  EventManager& operator=(const EventManager&) = delete;
  EventManager& operator=(EventManager&&) = delete;

protected:
  void InternalPublish(std::type_index type, const Message& msg) override;
  void InternalSubscribe(std::type_index type,
                         std::function<void(const Message&)> msg_callback) override;

private:
  EventManager(ILogger& logger, const AppSettings& settings);

  std::mutex mutex_;
  std::unordered_map<std::type_index, std::vector<std::function<void(const Message&)>>> registry_;

  ILogger& logger_;
  const AppSettings& settings_;
};

}  // namespace screen_controller

#endif
