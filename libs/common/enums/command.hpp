//
// Created by marce on 4/23/2025.
//
#pragma once

#include <string>

namespace screen_controller {

struct ToStringT {};
inline constexpr ToStringT kToString;

enum class Command {
  kNone,
  kSetScreen,
  kDeleteFile,
  kSaveFile,
};

inline std::string operator|(const Command& command, ToStringT) {
  switch (command) {
    case Command::kNone:
      return "none";
    case Command::kSetScreen:
      return "set_screen";
    case Command::kDeleteFile:
      return "delete_file";
    case Command::kSaveFile:
      return "save_file";
    default:
      return "unknown";
  }
}

}  // namespace screen_controller
