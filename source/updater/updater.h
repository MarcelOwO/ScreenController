//
// Created by marce on 04/08/2025.
//

#ifndef UPDATER_H
#define UPDATER_H


#include <ng-log/logging.h>

namespace screen_controller {
class Updater {
 public:
  Updater() = default;
  ~Updater() = default;

  Updater(Updater &) = delete;
  Updater &operator=(Updater &) = delete;
  Updater(Updater &&) = delete;
  Updater &operator=(Updater &&) = delete;

  bool CheckForUpdates();

 private:
};
}  // namespace screen_controller

#endif  // UPDATER_H
