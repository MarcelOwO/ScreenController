#pragma once

#include <functional>
#include <logging/logger.hpp>
#include <models/app_settings.hpp>
#include <typeindex>

namespace screen_controller {

struct Message {
  virtual ~Message() = default;
};

class IEventManager {
public:
  virtual ~IEventManager() = default;

  template <typename T>
  void Publish(const T& message) {
    static_assert(std::is_base_of_v<Message, T>, "T must inherit from Message");
    InternalPublish(typeid(T), message);
  }

  template <typename T>
  void Subscribe(std::function<void(const T&)> on_receive) {
    static_assert(std::is_base_of_v<Message, T>, "T must inherit from Message");
    auto wrapper = [on_receive](const Message& msg) { on_receive(static_cast<const T&>(msg)); };
    InternalSubscribe(typeid(T), std::move(wrapper));
  }

protected:
  virtual void InternalPublish(std::type_index type, const Message& msg) = 0;
  virtual void InternalSubscribe(std::type_index type,
                                 std::function<void(const Message&)> msg_callback) = 0;
};

class EventFactory {
public:
  static std::unique_ptr<IEventManager> Create(ILogger& logger, const AppSettings& settings);
};

}  // namespace screen_controller
