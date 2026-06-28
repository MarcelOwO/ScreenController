# ScreenController Bluetooth Protocol

Reference for the phone app (BackpackControllerApp) to implement the Bluetooth client.

---

## Transport

| Property | Value |
|---|---|
| Transport | Bluetooth LE (BLE / BT 5.x) |
| Protocol | L2CAP Connection-Oriented Channel (CoC), `SOCK_SEQPACKET` |
| PSM | `0x0081` |
| MTU | Negotiated at connect; device requests up to 65535 bytes |
| PHY | Prefers LE 2M (set via HCI at startup) |
| Max payload | 64 MB |

The phone app acts as the **BLE Central** (initiates connection).  
The device acts as the **BLE Peripheral** (listens, accepts).

---

## Discoverability

- The device is **discoverable and pairable** while no phone is connected.
- After a successful authentication handshake the device becomes **non-discoverable and non-pairable**, blocking all other connection attempts.
- On disconnect the device reverts to discoverable mode automatically.

---

## Packet Format

All packets — in both directions — share this binary layout (big-endian integers):

```
Offset  Size  Field
------  ----  -----
0       2     Magic          = 0xBEEF
2       1     Type           (see type table below)
3       4     NameLen        (uint32, number of bytes that follow)
7       N     Name           (UTF-8 string, not null-terminated)

— For types with payload (type >= 0x80) —
7+N     4     PayloadLen     (uint32)
11+N    4     CRC32          (IEEE 802.3 CRC of the raw payload bytes)
15+N    P     Payload        (PayloadLen bytes)
```

Types `< 0x80` carry **only** the Name field (no payload block).  
Types `>= 0x80` always carry a payload block (even if `PayloadLen == 0`).

---

## Packet Types

### Phone → Device

| Type | Constant | Name field | Payload |
|------|----------|-----------|---------|
| `0x01` | `kTypeCommand` | Command string (see Commands) | — |
| `0x80` | `kTypeAuthResponse` | `"auth"` | 32-byte PSK response (see Auth) |
| `0x81` | `kTypeFileTransfer` | Filename (e.g. `"photo.jpg"`) | zstd-compressed file data |

### Device → Phone

| Type | Constant | Name field | Payload |
|------|----------|-----------|---------|
| `0xC0` | `kTypeChallenge` | `"challenge"` | 16-byte random nonce |
| `0xC1` | `kTypeFileList` | `"files"` | Newline-separated filenames |
| `0xC2` | `kTypeAck` | Ack tag string | — |
| `0xCF` | `kTypeDeviceError` | `"error"` | UTF-8 error string `ERR:<code>:<message>` |

---

## Authentication Handshake

Authentication happens **once per L2CAP connection**, immediately after connect.  
The phone has **5 seconds** to complete it or the connection is closed.

```
Phone                           Device
  |                               |
  |------- L2CAP connect -------->|
  |                               |
  |<-- 0xC0 "challenge" nonce ----|  (16 random bytes)
  |                               |
  |--- 0x80 "auth" response ----->|  (32-byte XOR response, see below)
  |                               |
  |          [if valid]           |
  |<===== session open ==========>|
  |                               |
  |          [if invalid]         |
  |<-- 0xCF "error" ERR:1:... ----|
  |        [connection closed]    |
```

### Computing the Auth Response

Both apps share the same **pre-shared key (PSK)**: 32 bytes.

```
PSK (hex):
4F 77 4F 42 61 63 6B 70  61 63 6B 43 6F 6E 74 72
6F 6C 6C 65 72 32 30 32  35 4F 57 4F 21 4F 57 4F

PSK (ASCII): OwOBackpackController2025OWO!OWO
```

Given the 16-byte nonce from the device, compute:

```
response[i] = PSK[i] XOR nonce[i % 16]    for i in 0..31
```

Send this 32-byte `response` as the payload of an `0x80 / "auth"` packet.

The CRC32 in the packet header covers the payload bytes (the response itself).

---

## Commands (type 0x01)

The `Name` field is the entire command string.

| Command | Effect |
|---------|--------|
| `Rotate` | Rotate the displayed image by 90° |
| `Select:<filename>` | Display the named file from device storage |
| `Delete:<filename>` | Delete the named file from device storage |
| `GetFiles` | Device replies with a `0xC1` file list packet |

---

## File Transfer (type 0x81, phone → device)

1. Set `Name` = filename including extension (e.g. `"holiday.mp4"`).
2. Compress the file bytes with **zstd** (any compression level).
3. Set `Payload` = compressed bytes.
4. Compute CRC32 over the compressed bytes and set it in the header.
5. Send the packet. The device decompresses, saves to `files/<filename>`, and the file becomes available for `Select:`.

Supported file types: `.jpg`, `.jpeg`, `.png`, `.bmp`, `.gif`, `.mp4`, `.webm`, `.webp`

For large files the packet may span multiple L2CAP PDUs; the device reassembles automatically. The phone must similarly reassemble incoming device packets.

---

## File List Response (type 0xC1, device → phone)

Payload is a UTF-8 string: filenames separated by `\n`, one trailing newline.

```
photo1.jpg\nphoto2.png\nvideo.mp4\n
```

Parse by splitting on `\n` and discarding empty tokens.

---

## Error Packet (type 0xCF)

Payload is a UTF-8 string: `ERR:<code>:<message>`

| Code | Meaning |
|------|---------|
| 1 | Auth failed / timeout |
| 2 | Payload too large |
| 3 | CRC mismatch |

---

## Recommended Connection Flow (phone app)

```
1. Scan for BLE devices advertising with the device name / known service UUID.
2. Connect via L2CAP CoC on PSM 0x0081.
3. Immediately read the incoming 0xC0 challenge packet.
4. Compute and send the 0x80 auth response within 5 seconds.
5. Send 0x01 "GetFiles" to retrieve the current file list.
6. Use the file list to populate the UI.
7. Send commands or files as needed.
8. On app background / disconnect: close the L2CAP socket (device becomes discoverable again).
```

---

## Notes for Implementers

- The device runs as a systemd service (`owo-screen-controller`) started at boot.
- All integers in packet headers are **big-endian**.
- The CRC is IEEE 802.3 (same polynomial as Ethernet / zlib CRC32), initialised to `0xFFFFFFFF`, final XOR `0xFFFFFFFF`.
- The zstd decompressor on the device accepts any valid zstd frame; no specific compression parameters are required.
- Keep socket read buffers at least 1 MB on the phone side to handle large file list or future response packets.
