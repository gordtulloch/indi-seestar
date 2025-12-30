# INDI Seestar (S30/S50) Driver (C++)

This folder contains a **C++ INDI driver** for ZWO **Seestar S30/S50**.

It is based on reverse-engineering / behavior observed in the existing Python Alpaca driver in `alpaca_device/`.

## Status

- Provides a **Telescope** device skeleton.
- Implements TCP JSON message framing (`\r\n` delimited) and synchronous request/response handling.
- Implements basic operations:
  - Connect / Disconnect
  - Poll coordinates (`scope_get_equ_coord`)
  - Goto RA/Dec (`scope_goto`)
  - Abort slew (`iscope_stop_view` stage `AutoGoto`)
  - Sync (`scope_sync`)

Imaging (binary frames / RTSP) is intentionally not implemented yet.

## Build (Linux)

### Dependencies

On Debian/Ubuntu:

```bash
sudo apt update
sudo apt install -y build-essential cmake libindi-dev nlohmann-json3-dev
```

### Build

```bash
mkdir -p build
cd build
cmake ..
cmake --build . -j
```

The resulting driver binary will be named `indi_seestar_telescope`.

## Run (Linux)

Example:

```bash
indiserver -v /usr/bin/indi_simulator_ccd ./indi_seestar_telescope
```

Then connect from an INDI client (KStars/Ekos, etc.).

## Notes on Seestar Network Protocol

The Python reference driver uses:

- **TCP control** (default port `5555`): JSON messages, `\r\n` terminated.
  - `scope_get_equ_coord` returns `{ ra: <hours>, dec: <degrees> }`
  - `scope_goto` params: `[ra_hours, dec_degrees]`
  - `scope_sync` params: `[ra_hours, dec_degrees]`
  - `iscope_stop_view` params: `{ stage: "AutoGoto" }`

- **UDP intro** (port `4720`): `{"id":1,"method":"scan_iscope","params":""}` (used for discovery)

- **Imaging / stream** (observed): RTSP `rtsp://<host>:4554/stream` and a binary TCP stream (commonly port `7556`) with an 80-byte header.

## Next steps

- Add robust state handling (slew/tracking) based on Seestar events.
- Add location/time properties and push to device (`pi_set_time`, `set_setting`, etc.).
- Add optional CCD/imager support (INDI CCD interface) once imaging protocol is stabilized.
