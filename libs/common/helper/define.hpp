#pragma once

#include <cstdint>

#include <expected>
#include <optional>
#include <system_error>

namespace screen_controller {

using u32 = uint32_t;
using u8 = uint8_t;
using f32 = float;
using i32 = int32_t;

template <typename T>
using Result = std::expected<T, std::error_code>;

template <typename T>
using Optional = std::optional<T>;

};  // namespace screen_controller
