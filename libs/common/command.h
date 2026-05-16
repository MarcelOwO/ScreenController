//
// Created by marce on 4/23/2025.
//

#ifndef COMMAND_H
#define COMMAND_H

namespace screen_controller::common {
enum class Command {
  kNone,
  kSetScreen,
  kDeleteFile,
  kSaveFile,
};

inline std::string command_to_string(Command e) {
  switch (e) {
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
}  // namespace screen_controller::common
#endif  // COMMAND_H
