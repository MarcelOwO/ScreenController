#pragma once

#include <expected>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

namespace screen_controller {

// Unwraps the std::expected<std::unique_ptr<T>, E> returned by a subsystem's
// Create() factory. On failure it throws std::runtime_error with the given
// message; on success it returns the owned pointer, which converts implicitly to
// a std::unique_ptr of any base interface. This collapses the identical
// "if (!x) throw" boilerplate shared by every XxxFactory::Create.
template <typename T, typename E>
[[nodiscard]] std::unique_ptr<T> Unwrap(std::expected<std::unique_ptr<T>, E> result,
                                        std::string_view message) {
  if (!result) {
    throw std::runtime_error(std::string{message});
  }
  return std::move(result.value());
}

}  // namespace screen_controller
