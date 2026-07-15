#!/usr/bin/env bash
set -euo pipefail

target="${1:-owo@10.0.0.163}"
project_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
remote_dir="/home/owo/ScreenController-codex"

ssh "$target" sudo apt-get update
ssh "$target" sudo apt-get install -y \
  bluez build-essential cmake dpkg-dev ninja-build pkg-config rsync openssl \
  libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libwebp-dev \
  libbluetooth-dev libssl-dev libsystemd-dev \
  libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev \
  libxfixes-dev libxrender-dev \
  libwayland-dev libxkbcommon-dev wayland-protocols

rsync -a --delete \
  --exclude='.git/' \
  --exclude='/build/' \
  --exclude='/build-*/' \
  --exclude='/.sysroot/' \
  --exclude='/external/FFmpeg/' \
  --exclude='/logs/' \
  --exclude='/runtime-files/' \
  "$project_dir/" "$target:$remote_dir/"

ssh "$target" "cmake -S '$remote_dir' -B '$remote_dir/build-pi' -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_TESTING=ON && \
  cmake --build '$remote_dir/build-pi' --parallel 4 && \
  ctest --test-dir '$remote_dir/build-pi' --output-on-failure && \
  cpack --config '$remote_dir/build-pi/CPackConfig.cmake' -G DEB \
    -B '$remote_dir/build-pi/packages'"

ssh "$target" "sudo apt-get install -y '$remote_dir'/build-pi/packages/screencontroller_*.deb; \
  sudo systemctl --no-pager --full status owo-screen-controller.service; \
  printf '\nController enrollment URI (keep secret):\n'; \
  sudo screencontroller-show-key --uri"
