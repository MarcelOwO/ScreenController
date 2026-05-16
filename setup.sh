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

# 1. Detect and Target the active user running the sudo command
TARGET_USER="${SUDO_USER:-pi}"
TARGET_HOME=$(eval echo "~$TARGET_USER")

echo "[1/5] Installing core system dependencies..."
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

echo "[2/5] Configuring unrestricted Root/Global Bluetooth D-Bus permissions..."
# Grant full, unrestricted administrative permissions to the BlueZ interface
cat <<EOF > /etc/dbus-1/system.d/mylinuxapp-bluetooth.conf
<!DOCTYPE busconfig PUBLIC "-//freedesktop//DTD D-BUS Bus Configuration 1.0//EN"
 "http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd">
<busconfig>
  <!-- Allow the root user and the active kiosk user absolute control -->
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

  <!-- Fallback blanket permission for the system bus regarding BlueZ -->
  <policy context="default">
    <allow send_destination="org.bluez"/>
    <allow receive_sender="org.bluez"/>
  </policy>
</busconfig>
EOF

# Reload the D-Bus daemon configuration
systemctl reload dbus
systemctl reload dbus

echo "[3/5] Natively compiling your CMake application..."
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure and compile using all available CPU cores
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
make -j$(nproc)

# Expose compilation mappings to Doom Emacs' LSP
if [ -f compile_commands.json ]; then
    ln -fs "$BUILD_DIR/compile_commands.json" "$SCRIPT_DIR/compile_commands.json"
    chown "$TARGET_USER:$TARGET_USER" "$SCRIPT_DIR/compile_commands.json"
fi

echo "[4/5] Setting up X11 startup profile..."
# Configure X11 to skip desktop elements and jump directly to your app in fullscreen
cat <<EOF > "$TARGET_HOME/.xinitrc"
#!/bin/sh
# Disable screen sleep, blanking, and power-saving management
xset s off
xset s noblank
xset -dpms

# Run the compiled application binary
exec $BUILD_DIR/MyLinuxApp
EOF

chown "$TARGET_USER:$TARGET_USER" "$TARGET_HOME/.xinitrc"
chmod +x "$TARGET_HOME/.xinitrc"

echo "[5/5] Creating systemd kiosk service for automated boot execution..."
# Define a system service that launches an X server running your application configuration
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
# ExecStart triggers startx which implicitly runs the ~/.xinitrc script written above
ExecStart=/usr/bin/startx
Restart=always
RestartSec=3

[Install]
WantedBy=multi-user.target
EOF

# Reload changes, enable the service to hook into the boot cycle, and clear standard display managers
systemctl daemon-reload
systemctl enable kiosk.service

echo "===================================================="
echo " Done! Run 'sudo reboot' to launch your application. "
echo "===================================================="
