# ScreenController Bluetooth Protocol v2

This is the wire-level contract for the future phone controller. Version 2 is intentionally
incompatible with the earlier fixed-key/XOR draft.

## Security model

The protocol uses two layers:

1. The L2CAP socket requests `BT_SECURITY_MEDIUM`, so BlueZ requires an encrypted Bluetooth LE
   link before application traffic is accepted. Headless `NoInputNoOutput` pairing uses LE Secure
   Connections where supported, but its Just Works association does **not** provide MITM protection.
2. The application performs an HMAC-SHA256 challenge using a random, device-specific 256-bit key.
   Clients without that key are disconnected after five seconds and cannot issue commands.

The server no longer contains a universal key. Provision each Pi with:

```text
SCREEN_CONTROLLER_PSK_HEX=<64 lowercase or uppercase hex characters>
```

The systemd unit reads this from `/etc/screencontroller/screencontroller.env`, which must be owned
by root and mode `0600`. `scripts/provision-pi.sh` generates it. Import the resulting key into the
phone app through a protected enrollment flow (for example, a QR code shown or supplied with that
specific Pi). Do not ship one shared key in every copy of the phone app.

This design authorizes possession of the per-device secret; Bluetooth cannot prove which app binary
is running. A compromised/rooted phone can disclose its key. For deployments exposed to hostile
nearby attackers, add a screen-displayed enrollment code or QR flow before production.

## Discovery and transport

| Property | Value |
|---|---|
| Role | Pi = BLE Peripheral, phone = BLE Central |
| Advertisement name | `ScreenController` |
| Advertisement service UUID | `8e7f1a10-6e40-4d5f-8d7c-9b5a9f88a001` |
| Transport | LE L2CAP Connection-Oriented Channel, `SOCK_SEQPACKET` |
| LE PSM | `0x0081` |
| Byte order | Network order (big-endian) |
| Compressed packet limit | 64 MiB |
| Decompressed file limit | 128 MiB |
| Preferred PHY | LE 2M when the controller and adapter support it |

Before commissioning, the advertisement is active and BlueZ is discoverable/pairable. The first
phone that successfully completes HMAC authentication is atomically recorded as the commissioned
controller using its Bluetooth address and address type. The advertisement and BlueZ
discoverable/pairable flags are disabled while it is connected.

After that phone disconnects, the advertisement is restored so the bonded controller can reconnect,
but discoverable and pairable remain disabled. Connections from any different Bluetooth identity
are closed before an authentication challenge is sent. Replacing a phone is an explicit local
administrative action:

```bash
sudo screencontroller-reset-controller
```

The phone should use a stable bonded LE identity. If its operating system loses the bond or changes
the presented identity, reset commissioning on the Pi and enroll it again.

## Packet framing

Every packet starts with:

```text
Offset  Size  Field
0       2     Magic = 0xBEEF
2       1     Type
3       4     NameLen (uint32)
7       N     UTF-8 Name (not NUL-terminated; maximum 256 bytes)
```

Types below `0x80` end after `Name`. Types `0x80` and above always append the following block,
including when the payload is empty:

```text
7+N     4     PayloadLen (uint32)
11+N    4     CRC32 of Payload
15+N    P     Payload bytes
```

CRC32 is the IEEE/zlib variant (polynomial `0xEDB88320`, initial value `0xFFFFFFFF`, final XOR
`0xFFFFFFFF`). CRC detects transport/framing errors; it is not an authentication primitive.

An L2CAP record may contain only part of a protocol packet. Both implementations must retain bytes
until a complete packet is available and must also handle multiple complete packets in one read.

## Authentication

Immediately after accepting an encrypted L2CAP connection, the Pi sends:

| Direction | Type | Name | Payload |
|---|---:|---|---|
| Pi → phone | `0xC0` | `auth-v2` | 32 cryptographically random bytes |

The phone calculates:

```text
response = HMAC-SHA256(
    key = the 32-byte device key,
    message = ASCII("screen-controller/auth/v2") || nonce
)
```

It must reply within five seconds:

| Direction | Type | Name | Payload |
|---|---:|---|---|
| phone → Pi | `0x80` | `auth-v2` | 32-byte HMAC response |

The Pi compares the response in constant time. Any command, upload, malformed response, or timeout
before successful authentication is rejected and the connection is closed.

## Packet types

### Phone → Pi

| Type | Name field | Payload |
|---:|---|---|
| `0x01` | Command string | none |
| `0x80` | `auth-v2` | HMAC response |
| `0x81` | File name | One complete zstd frame |

### Pi → phone

| Type | Name field | Payload |
|---:|---|---|
| `0xC0` | `auth-v2` | Random challenge |
| `0xC1` | `files` | Newline-separated UTF-8 file names |
| `0xC2` | Operation name | Empty payload block |
| `0xC3` | `status` | UTF-8 JSON status |
| `0xCF` | `error` | UTF-8 `ERR:<code>:<message>` |

## Commands

Send commands as type `0x01`, with the entire command in `Name`.

| Command | Result |
|---|---|
| `GetFiles` | Sends a `0xC1/files` response |
| `GetStatus` | Sends a `0xC3/status` response |
| `Select:<filename>` | Starts displaying the named uploaded file |
| `Delete:<filename>` | Deletes the named uploaded file |
| `Rotate` | Rotates the rendered content 90 degrees |
| `SetBrightness:<0-100>` | Sets software-rendered brightness |
| `ScreenOff` | Renders black without stopping playback or disconnecting |
| `ScreenOn` | Restores rendered output |

Successful state-changing commands return type `0xC2`; its `Name` is one of `Rotate`, `Select`,
`Delete`, `SetBrightness`, `ScreenOff`, `ScreenOn`, or `Upload`.

Brightness and screen-off are currently renderer controls. They work with every connected display
but do not cut panel power. Hardware backlight/DPMS control should be added as a board/display-specific
backend rather than assumed by the phone app.

The status payload currently has this schema:

```json
{"brightness":100,"displayEnabled":true}
```

Unknown JSON properties added in later versions must be ignored by the phone.

## File upload

1. Validate the source extension.
2. Compress the complete file as a zstd frame that includes its content size. Level 1 is recommended
   for images/video because they are already compressed and Bluetooth throughput is the bottleneck.
3. Send type `0x81`, the base filename in `Name`, and the zstd frame in `Payload`.
4. Wait for `0xC2/Upload` before presenting the file as available.

Allowed extensions are `.jpg`, `.jpeg`, `.png`, `.bmp`, `.gif`, `.webp`, `.mp4`, and `.webm`
(case-insensitive). Names must be a single base name: no `/`, `\\`, `.`/`..`, or parent path. The Pi
writes to a temporary file and atomically publishes it only after validation and decompression.

For speed, negotiate the largest platform-supported L2CAP MTU, keep a send buffer of at least 1 MiB,
avoid application-level delays between writes, and keep one connection open for a batch of uploads.

## File list

The `0xC1/files` payload contains sorted filenames separated by `\n`, with a trailing newline when
the list is non-empty. Split on `\n` and discard empty entries.

## Errors

| Code | Meaning |
|---:|---|
| 1 | Authentication failed or timed out |
| 2 | Packet/receive limit exceeded |
| 3 | CRC mismatch |
| 4 | Malformed, unsupported, or unknown command/packet |
| 5 | Invalid zstd data or decompressed-size violation |
| 6 | File save/delete failure |
| 7 | File selection/decoding failure |
| 8 | Invalid brightness value |
| 9 | Authenticated controller could not be persisted |

After framing or authentication errors, the phone must assume the socket can be closed and reconnect
from discovery. Never retry an upload blindly unless no `Upload` acknowledgement was received and the
file list confirms it is absent.

## Recommended phone flow

1. Load the enrolled device key from Android Keystore/iOS Keychain.
2. Scan for the service UUID and connect/pair with the advertised Pi. Pairing is available only
   before the first controller has been commissioned.
3. Open LE L2CAP PSM `0x0081`.
4. Read `0xC0/auth-v2`, calculate HMAC-SHA256, and send `0x80/auth-v2` within five seconds.
5. Request `GetStatus` and `GetFiles`.
6. Keep one socket and process acknowledgements/errors for every mutation.
7. On background/disconnect, close the channel so the Pi advertises again.
