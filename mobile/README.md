# ScreenController MAUI app

This .NET 10 MAUI controller targets Android 10+, Mac Catalyst 15+, and Windows 10 1809+. It scans
only for ScreenController displays, stores the per-display key with MAUI Secure Storage, performs the
v2 HMAC handshake, and exposes upload, media selection/deletion, rotation, brightness, logical screen
power, status, and refresh controls.

## Platform transports

| Platform | Native API | Transport |
|---|---|---|
| Android | `BluetoothDevice.createL2capChannel` | Secure LE L2CAP CoC |
| macOS | CoreBluetooth `CBPeripheral.OpenL2CapChannel` | Secure LE L2CAP CoC |
| Windows | `Windows.Devices.Bluetooth.GenericAttributeProfile` | Encrypted GATT write/notify tunnel |

Protocol framing, authentication, compression, response correlation, and commands live in
`ScreenController.Protocol`; platform projects only implement discovery and byte transport. The Pi
advertises one service UUID and accepts both transport forms.

## Prerequisites

- .NET SDK `10.0.301` or the compatible SDK selected by `global.json`
- MAUI workload: `dotnet workload restore src/ScreenController.App/ScreenController.App.csproj`
- Android SDK API 36 and JDK 21 for Android builds
- Xcode on macOS for Mac Catalyst
- Visual Studio 2026/.NET MAUI tooling on Windows for Windows packaging

On macOS the build automatically detects Homebrew `openjdk@21` on Apple Silicon or Intel and the
standard `~/Library/Android/sdk` location. A nonstandard installation can still be selected with
`-p:JavaSdkDirectory=/path/to/jdk -p:AndroidSdkDirectory=/path/to/android-sdk`.

Restore and test the shared protocol:

```bash
cd mobile
dotnet workload restore src/ScreenController.App/ScreenController.App.csproj
dotnet restore ScreenController.Mobile.slnx
dotnet test tests/ScreenController.Protocol.Tests -c Release
```

Build the app on its target operating system:

```bash
dotnet build src/ScreenController.App -f net10.0-android -c Release
dotnet build src/ScreenController.App -f net10.0-maccatalyst -c Release
dotnet build src/ScreenController.App -f net10.0-windows10.0.19041.0 -c Release
```

Windows is included in `TargetFrameworks` only when MSBuild runs on Windows. Android and Mac
Catalyst artifacts require their corresponding signing identities before store or device release.

## Enrollment and use

1. On the Pi, run `sudo screencontroller-show-key --uri` and transfer the URI privately to the
   controller. A deployment can encode the same text in a QR code.
2. In the app, scan, select the display, paste the enrollment URI, and save it.
3. Select Connect. The operating system may ask for Bluetooth permission or confirm initial Just
   Works pairing on the controller; the Pi itself requires no keyboard, mouse, or confirmation.
4. The first successful authenticated controller commissions the Pi. Later controllers are rejected
   until a local administrator runs `sudo screencontroller-reset-controller`.

The enrollment secret is never logged or placed in app preferences. Forget removes the local secure
store entry but intentionally does not reset the Pi.

## Known physical limitation

Screen off and brightness are renderer-level controls so they work consistently with HDMI and DSI.
They do not electrically power off an arbitrary panel or change its hardware backlight. Those require
a Pi-side backend selected for the exact display.
