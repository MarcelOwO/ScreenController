# CLAUDE.md

Repository guidance for coding agents working on ScreenController.

## Project

ScreenController is a C++23 fullscreen Raspberry Pi/Linux application. It displays images and video
controlled by a future phone app over Bluetooth LE. macOS is supported for development with a dummy
Bluetooth backend.

## Build and test

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/src/ScreenController
```

Linux startup fails closed unless `SCREEN_CONTROLLER_PSK_HEX` contains a 32-byte key encoded as 64
hex characters. CMake 3.25+ and a C++23 compiler are required. Debian/Raspberry Pi dependency and
deployment commands are maintained in `README.md` and `scripts/provision-pi.sh`.

## Style

Use the repository `.clang-format` and `.clang-tidy` settings. The naming scheme is CamelCase for
types/functions, `lower_case_` for members, `lower_case` for locals/parameters, and `kCamelCase` for
constants. Keep lines within 100 columns.

## Architecture

All project code uses the `screen_controller` namespace. Subsystems expose abstract interfaces from
`include/<module>/`, keep concrete implementations in `src/`, and are constructed by factories.
`App` owns subsystem lifetimes with `std::unique_ptr`.

- `app`: command handling and render loop
- `bluetooth`: BlueZ advertisement/agent management, authenticated LE L2CAP transport
- `events`: type-erased in-process pub/sub
- `graphics`: OpenGL ES on Linux and OpenGL on macOS
- `logging`: spdlog-backed logger
- `processor`: stb, WebP, and streaming FFmpeg decoders
- `storage`: asset lookup and validated/atomic user-file storage
- `window`: fullscreen GLFW window
- `libs/common`: shared models, enums, and helpers

## Bluetooth and security

The Linux backend requires BlueZ link encryption, then authenticates a random nonce with
HMAC-SHA256 and a per-device key. Never add a universal embedded key or log a real key. Incoming
names, compressed sizes, decompressed sizes, CRCs, packet types, and authentication state are all
security boundaries. The canonical phone-facing specification is `BLUETOOTH_PROTOCOL.md`; update it
whenever the wire format or commands change.

Only one controller is accepted. The first successful HMAC authentication persists its Bluetooth
identity. The BLE advertisement is removed while connected, then restored without making the adapter
pairable after commissioning. Large files are zstd frames and should remain bounded; avoid designs
that add an extra full-size copy or decode whole videos into memory.

## Packaging

CPack installs the binary to `/usr/bin/screen_controller`, assets to
`/usr/share/screencontroller/assets`, and the systemd unit to
`/usr/lib/systemd/system/owo-screen-controller.service`. Runtime uploads live below
`/var/lib/screencontroller`; the provisioning key is read from
`/etc/screencontroller/screencontroller.env`. Linux releases use the CPack DEB generator with
runtime dependencies discovered by `dpkg-shlibdeps`; do not replace those dependencies with a
fragile fully static glibc/graphics/media build.
