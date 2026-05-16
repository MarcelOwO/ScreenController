#!/usr/bin/env bash

# Exit immediately if any command fails
set -e

echo "===================================================="
echo " Starting Raspberry Pi Bluetooth Kiosk Installation "
echo "===================================================="

# Ensure script is run as root
if [ "$EUID" -ne 0 ]; then
  echo "Please run as root (sudo ./install.sh)"
  exit 1
fi

# Target the active user running the sudo command
TARGET_USER="${SUDO_USER:-pi}"
TARGET_HOME=$(eval echo "~$TARGET_USER")

echo "[1/5] Installing system packages and development headers..."
apt-get update
apt-get install -y \
    build-essential \
    cmake \
    git \
    clangd \
    pkg-config \
    libglfw3-dev \
    libglm-dev \
    protobuf-compiler \
    libprotobuf-dev \
    libsdbus-c++-dev \
    libzstd-dev \
    libavformat-dev \
    libavcodec-dev \
    libavutil-dev \
    bluez \
    bluez-tools \
    xorg \
    x11-xserver-utils

# Check if ng-log is available via apt; if not, we build it quickly
if ! apt-cache show libglog-dev > /dev/null 2>&1; then
    echo "Installing glog/ng-log fallbacks..."
    apt-get install -y libgoogle-glog-dev
fi

echo "[2/5] Configuring BlueZ Bluetooth pipeline D-Bus permissions..."
cat <<EOF > /etc/dbus-1/system.d/mylinuxapp-bluetooth.conf
<!DOCTYPE busconfig PUBLIC "-//freedesktop//DTD D-BUS Bus Configuration 1.0//EN"
 "http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd">
<busconfig>
  <policy user="$TARGET_USER">
    <allow send_destination="org.bluez"/>
    <allow receive_sender="org.bluez"/>
    <allow send_interface="*"/>
  </policy>
  <policy user="root">
    <allow send_destination="org.bluez"/>
    <allow receive_sender="org.bluez"/>
    <allow send_interface="*"/>
  </policy>
  <policy context="default">
    <allow send_destination="org.bluez"/>
    <allow receive_sender="org.bluez"/>
  </policy>
</busconfig>
EOF

systemctl reload dbus

echo "[3/5] Natively compiling ScreenController..."
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

# Clear any stale submodule cache builds if they exist
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure and compile using your crisp, new find_package setup
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
make -j$(nproc)

# Expose compilation mappings to Doom Emacs' LSP
if [ -f compile_commands.json ]; then
    ln -fs "$BUILD_DIR/compile_commands.json" "$SCRIPT_DIR/compile_commands.json"
    chown "$TARGET_USER:$TARGET_USER" "$SCRIPT_DIR/compile_commands.json"
fi

echo "[4/5] Setting up X11 startup profile..."
cat <<EOF > "$TARGET_HOME/.xinitrc"
#!/bin/sh
xset s off
xset s noblank
xset -dpms

# Run the compiled system-linked binary
exec $BUILD_DIR/ScreenController
EOF

chown "$TARGET_USER:$TARGET_USER" "$TARGET_HOME/.xinitrc"
chmod +x "$TARGET_HOME/.xinitrc"

echo "[5/5] Creating systemd kiosk service for automated boot execution..."
cat <<EOF > /etc/systemd/system/kiosk.service
[Unit]
Description=Bluetooth Media Kiosk Application
After=network.target sound.target bluetooth.service
Requires=bluetooth.service

[Service]
Type=simple
User=$TARGET_USER
Environment=DISPLAY=:0
PAMName=login
TTYPath=/dev/tty1
StandardInput=tty
ExecStart=/usr/bin/startx
Restart=always
RestartSec=3

[Install]
WantedBy=multi-user.target
EOF

systemctl daemon-reload
systemctl enable kiosk.service

echo "===================================================="
echo " Done! Run 'sudo reboot' to launch your application. "
echo "===================================================="
