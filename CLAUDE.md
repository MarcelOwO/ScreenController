# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ScreenController is a C++23 fullscreen desktop application that runs as a Bluetooth peripheral, displaying images/videos controlled remotely via the [BackpackControllerApp](https://github.com/MarcelOwO/BackpackControllerApp). It targets Linux (primary) with partial macOS support for development.

## Build System

CMake 4.x, C++23, out-of-source build. The `build/` directory is pre-configured; compiled output goes there.

```bash
# Configure (only needed once or after CMakeLists changes)
cmake -B build -S .

# Build
cmake --build build

# Run
./build/src/ScreenController

# Install (Linux, packages for systemd + dbus)
cmake --build build --target install
```

Build artifact: `build/src/ScreenController`

## Code Style & Linting

Google C++ Style with modifications. Enforced via clang-format and clang-tidy (warnings-as-errors).

```bash
# Format all source files
clang-format -i src/**/*.cpp src/**/*.hpp libs/**/*.hpp

# Lint (uses compile_commands.json from build/)
clang-tidy -p build/ src/path/to/file.cpp
```

Naming conventions (from `.clang-tidy`):
- Classes/Structs/Enums: `CamelCase`
- Methods/Functions: `CamelCase`
- Member variables: `lower_case_` (trailing underscore)
- Parameters/locals: `lower_case`
- Constants/enum values: `kCamelCase` prefix
- Global variables: `g_lower_case` prefix

Line length: 100 columns. Includes are sorted within blocks; blank lines separate blocks.

## Architecture

All code lives in the `screen_controller` namespace.

### Module Structure

Each subsystem follows the same pattern: a public interface in `include/<module>/`, a concrete implementation in `src/`, and a factory that constructs it. The `App` class in `src/app/` owns all subsystems via `std::unique_ptr` to their interfaces.

```
src/
  main.cpp            — entry point, creates App, calls AdjustSettings + Run
  app/                — App class: owns all subsystems, runs render loop
  bluetooth/          — BLE peripheral (Linux: sdbus-c++ + L2CAP socket; non-Linux: dummy)
  graphics/           — OpenGL renderer via glad + glm
  processor/          — file decoding (stb for images, FFmpeg for video, libwebp for WebP)
  storage/            — file path resolution (assets vs user files)
  window/             — GLFW window management (macOS uses Cocoa/OpenGL frameworks)
  events/             — type-erased pub/sub event bus (IEventManager)
  logging/            — spdlog-backed logger (ILogger)

libs/common/          — shared headers (interface-only, no compiled sources)
  models/             — AppSettings, BluetoothPacket, FrameData, PixelData
  enums/              — Command, FileType, LogLevel, ErrorEnum
  interfaces/         — Module, Factory base types
  helper/             — define.hpp (u32, f32 typedefs etc.)
```

### Interface / Factory Pattern

Every subsystem exposes only an abstract interface (`IXxx`) and a factory (`XxxFactory::Create(...)`). The `App` constructor calls all factories in sequence:

```cpp
logger_        = LoggerFactory::Create();
window_manager_= WindowFactory::Create(*logger_, ...);
renderer_      = RendererFactory::Create(*logger_, ...);
bluetooth_     = BluetoothFactory::Create(*logger_, *settings_, callback);
storage_       = StorageFactory::Create(*logger_);
event_manager_ = EventFactory::Create(*logger_, *settings_);
file_processor_= ProcessorFactory::Create(*logger_);
```

Concrete implementations (e.g., `GraphicsRenderer`, `GlfwWindow`) are never included outside their own `src/` directory.

### Bluetooth (Linux-only)

Uses sdbus-c++ to implement BlueZ GATT profiles over D-Bus. Runs as a D-Bus service (`com.owo.screen_controller`). L2CAP socket (`l2cap_receiver`) receives raw binary packets; `unpacker` deserializes them into `Packet` structs (type, name, zstd-compressed payload, CRC32). The Bluetooth module is compiled as a dummy stub on non-Linux platforms.

### Render Loop

`App::RenderLoop()` runs at ~60 fps (16 ms sleep). Each tick:
1. `ProcessFrame()` — pulls decoded frame from `IFileProcessor`, uploads texture to renderer
2. `window_manager_->update(render_fn)` — swap buffers
3. `window_manager_->poll_events()` — GLFW event pump

### Command Handling

`App::ProcessCommand()` interprets `BluetoothPacket` by `type`:
- `type == 0`: file transfer → `storage_manager_->SaveFile()`
- `type == 1`: command string `"Select:<name>"`, `"Delete:<name>"`, or `"Rotate"` → calls appropriate subsystem

### External Dependencies (git submodules)

- `external/glfw` — window/input
- `external/glm` — math
- `external/stb` — image decode (compiled into `external/libstb.a`)
- `external/sdbus-cpp` — D-Bus (Linux only)
- `external/zstd` — payload compression (Linux only)
- `external/spdlog` — logging
- `external/FFmpeg` — video decode (system pkg-config: libavformat, libavcodec, libavutil, libswscale)

System dependencies also needed: `libwebp`, `libbluetooth` (Linux).

### Packaging

Produces a Debian package via CPack. Installs:
- Binary → `/usr/bin/screen_controller`
- Assets → `/usr/share/screencontroller/assets/`
- D-Bus policy → `/usr/share/dbus-1/system.d/com.owo.screen_controller.conf`
- systemd unit → `/usr/lib/systemd/system/owo-screen-controller.service`
