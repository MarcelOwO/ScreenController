#pragma once

#include <functional>
#include <logging/logger.hpp>
#include <models/app_settings.hpp>
#include <typeindex>
#include <vector>

namespace screen_controller {

// ── Base ──────────────────────────────────────────────────────────────────────

struct Message {
  virtual ~Message() = default;
};

// ── Domain events ─────────────────────────────────────────────────────────────

// Bluetooth received a command string from the phone (e.g. "Select:foo", "Rotate").
struct CommandReceivedEvent : Message {
  explicit CommandReceivedEvent(std::string command_value) : command(std::move(command_value)) {}
  std::string command;
};

// Bluetooth received a raw file payload from the phone ready to be saved.
struct FileReceivedEvent : Message {
  FileReceivedEvent(std::string filename_value, std::vector<std::byte> data_value)
      : filename(std::move(filename_value)), data(std::move(data_value)) {}
  std::string filename;
  std::vector<std::byte> data;
};

// The Bluetooth peer connected or disconnected.
struct ConnectionChangedEvent : Message {
  explicit ConnectionChangedEvent(bool is_connected) : connected(is_connected) {}
  bool connected{false};
};

// ── Interface ─────────────────────────────────────────────────────────────────

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
    InternalSubscribe(typeid(T),
                      [on_receive](const Message& msg) { on_receive(static_cast<const T&>(msg)); });
  }

protected:
  virtual void InternalPublish(std::type_index type, const Message& msg) = 0;
  virtual void InternalSubscribe(std::type_index type,
                                 std::function<void(const Message&)> callback) = 0;
};

// ── Factory ───────────────────────────────────────────────────────────────────

class EventFactory {
public:
  static std::unique_ptr<IEventManager> Create(ILogger& logger, const AppSettings& settings);
};

}  // namespace screen_controller
