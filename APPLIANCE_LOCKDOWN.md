# Appliance lockdown

ScreenController 2.0 packages the Raspberry Pi as a fullscreen, single-purpose display application
without cutting off the administrator's current access path.

## Applied by the package

- Starts the fullscreen controller with systemd on the graphical target.
- Generates a unique 256-bit authentication key with root-only permissions.
- Commissions exactly one successfully authenticated Bluetooth controller.
- Disables new Bluetooth pairing after commissioning and rejects other controller identities.
- Rejects unrelated Bluetooth profile authorization requests.
- Limits the service to Bluetooth/Unix sockets, required capabilities, read-only system paths, and
  private application state.
- Restarts the application automatically after a crash.

## Intentionally not applied yet

The package does not disable Wi-Fi, Ethernet, SSH, virtual consoles, USB, the display manager, or
package updates. In particular, Wi-Fi must remain enabled on the current test Pi because it is the
only available SSH connection.

A production offline-appliance profile should be a separate, explicit installation option after the
display and recovery procedure have been proven. It should, at minimum:

1. Provide a direct console or physical recovery mechanism.
2. Disable Wi-Fi and unwanted network services without removing Bluetooth firmware support.
3. Disable SSH and interactive logins only after an offline update/recovery path exists.
4. Allowlist required USB input/storage classes, or disable them if no service port is required.
5. Use a read-only root filesystem or an A/B image with writable application state.
6. Add a hardware/software watchdog and a tested rollback path.
7. Replace the desktop dependency with a direct DRM/GBM kiosk backend if the base OS must expose no
   general-purpose graphical session.

Do not apply those steps remotely to a device whose only management route is Wi-Fi.
