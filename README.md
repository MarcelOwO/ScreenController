# ScreenController

ScreenController is a C++23 fullscreen Linux application for a dedicated Raspberry Pi display. A
phone controller connects over encrypted Bluetooth LE L2CAP, authenticates with a per-device key,
uploads images/video, chooses content, rotates it, and controls rendered brightness/output.

The phone application is not part of this repository yet. Its wire contract is documented in
[BLUETOOTH_PROTOCOL.md](BLUETOOTH_PROTOCOL.md).

## Current capabilities

- Fullscreen OpenGL display for JPEG, PNG, BMP, GIF, WebP, MP4, and WebM
- BLE advertisement and one-client LE L2CAP transport
- BlueZ link encryption plus HMAC-SHA256 application authentication
- zstd-compressed uploads with CRC, size limits, filename validation, and atomic writes
- File listing, selection, deletion, rotation, status, brightness, and logical screen on/off
- installable Debian package, systemd service, and Raspberry Pi provisioning script

`ScreenOff` and brightness currently change rendered output; they do not power down arbitrary HDMI
or DSI panels. Physical backlight/DPMS support needs a backend specific to the attached display.

## Build

The project uses CMake 3.25+ and C++23. On Debian/Raspberry Pi OS:

```bash
sudo apt-get install \
  bluez build-essential cmake ninja-build pkg-config openssl \
  libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libwebp-dev \
  libbluetooth-dev libssl-dev libsystemd-dev \
  libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev \
  libxfixes-dev libxrender-dev

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Linux startup intentionally fails when no provisioning key is configured:

```bash
export SCREEN_CONTROLLER_PSK_HEX="$(openssl rand -hex 32)"
./build/src/ScreenController
```

For local macOS development the Bluetooth module is a dummy; FFmpeg and WebP development packages
are still required.

## Raspberry Pi package

The supported deployment artifact is an ARM64 Debian package. It contains the application,
project-owned libraries, shaders/assets, systemd service, commissioning tools, dependency metadata,
and install/remove hooks. System libraries such as FFmpeg, OpenSSL, BlueZ, Mesa, and libc remain
dynamically linked so Raspberry Pi OS can deliver security and ABI updates.

Build the package on a Raspberry Pi running the same Raspberry Pi OS/Debian release as the target:

```bash
cmake -S . -B build-pi -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_TESTING=ON
cmake --build build-pi --parallel
ctest --test-dir build-pi --output-on-failure
cpack --config build-pi/CPackConfig.cmake -G DEB -B build-pi/packages
```

Copy the resulting `screencontroller_2.0.0_arm64.deb` to a new Pi and install it:

```bash
sudo apt install ./screencontroller_2.0.0_arm64.deb
sudo systemctl status owo-screen-controller --no-pager -l
sudo screencontroller-show-key
```

Installation generates a unique root-readable key, enables the service, creates private state
directories, and selects the first regular local user when a graphical desktop session is present.
On a custom kiosk image without such a user, the hardened service defaults to root. The target must
provide a graphical session on display `:0`; a direct DRM/GBM display backend is not implemented yet.

## Raspberry Pi provisioning

The script installs build packages, synchronizes this working tree, builds and tests an ARM64
package, installs it through APT, and prints its newly generated enrollment key:

```bash
./scripts/provision-pi.sh owo@10.0.0.163
```

It prints the device key once for enrollment into the future phone app. Treat it like a password.
The Pi must be reachable over SSH and the user must have passwordless or interactive `sudo` access.

Useful diagnostics:

```bash
ssh owo@10.0.0.163 'systemctl status owo-screen-controller --no-pager -l'
ssh owo@10.0.0.163 'journalctl -u owo-screen-controller -b --no-pager -n 200'
```

The service stores uploads below `/var/lib/screencontroller/files`, records its commissioned phone
at `/var/lib/screencontroller/controller.id`, reads assets from
`/usr/share/screencontroller/assets`, and reads its secret from
`/etc/screencontroller/screencontroller.env`.

The first phone that completes HMAC authentication is permanently commissioned. After that, the Pi
remains advertisable for that bonded phone but is no longer discoverable or pairable, and L2CAP
connections from other Bluetooth addresses are rejected before the authentication challenge. To
replace the controller deliberately:

```bash
sudo screencontroller-reset-controller
```

See [APPLIANCE_LOCKDOWN.md](APPLIANCE_LOCKDOWN.md) for the current kiosk boundary and the deferred
offline lockdown. This release intentionally does **not** modify Wi-Fi, networking, SSH, login
services, or firewall rules.

The current device at `owo@10.0.0.163` also has a non-root testing deployment at
`/home/owo/ScreenController-codex`. It runs as the enabled user service
`owo-screen-controller.service`; its key is stored with mode `0600` at
`~/.config/screencontroller/environment`. Useful test-deployment commands are:

```bash
ssh owo@10.0.0.163 'systemctl --user status owo-screen-controller --no-pager -l'
ssh owo@10.0.0.163 'systemctl --user restart owo-screen-controller'
```

## Security notes

- Never commit or document a real provisioning key.
- Use a unique key per Pi and store phone-side keys in Android Keystore/iOS Keychain.
- Headless Bluetooth Just Works pairing encrypts the radio link but does not prevent an active MITM;
  the HMAC key prevents unauthenticated command access. A screen-displayed enrollment QR/code is the
  recommended next security feature.
- Bluetooth possession is enforced by the secret plus the commissioned controller identity. No
  protocol can prove an unmodified official app is running on a rooted phone that has extracted the
  secret.
- On Raspberry Pi OS Desktop the package runs as the desktop user with only `CAP_NET_ADMIN` and
  `CAP_NET_RAW`; on a userless custom graphical image it falls back to the unit's root account. The
  systemd sandbox limits writable paths, address families, kernel access, and privilege escalation.

## License

[MIT](https://choosealicense.com/licenses/mit/)
