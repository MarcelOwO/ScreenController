#pragma once

#include <expected>
#include <system_error>

#include "sdbus-c++/Error.h"

namespace screen_controller::dbus {

template <typename Func>
auto run_dbus_op(Func&& op) -> decltype(op()) {
  try {
    return op();
  } catch (const sdbus::Error& e) {
    return std::unexpected(std::make_error_code(std::errc::operation_not_permitted));
  }
}

}  // namespace screen_controller::dbus
