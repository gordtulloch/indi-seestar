# Seestar S50 CCD INDI Driver

## Overview

This driver provides camera support for the Seestar S50/S30 smart telescope through INDI. It inherits from the generic ASCOM Alpaca CCD driver and applies Seestar-specific workarounds for the device's non-standard ImageBytes protocol implementation.

## Seestar CCD vs Alpaca Camera Standard (Quick Summary)

Seestar camera support is good for core imaging, but not a full implementation of the ASCOM Alpaca Camera interface.

- **Coverage (tested on v1.1.2-1):** 40/58 Camera GET methods working (~69%)
- **Strong areas:** exposure control, image download, sensor geometry, ROI/subframe, gain range, camera state, read-only CCD temperature
- **Not implemented by firmware (common `ErrorNumber=1024`):**
  - Offset controls (`offset`, `offsetmin`, `offsetmax`, `offsets`)
  - Active cooling (`setccdtemperature`, `cooleron`, `coolerpower`, `heatsinktemperature`)
  - Fast readout (`fastreadout`)
  - Exposure history/progress (`lastexposureduration`, `lastexposurestarttime`, `percentcompleted`)
  - Sub-exposure duration (`subexposureduration`)
- **Capability constraints:**
  - Binning effectively unavailable (`maxbinx=1`, `maxbiny=1`)
  - `canpulseguide=false` on the camera interface

**Practical takeaway:** the Seestar CCD driver is reliable for normal capture workflows, but clients should feature-detect at runtime instead of assuming full Alpaca Camera coverage.

## Architecture

```
SeestarCCD (indi_seestar_ccd)
    ↓ inherits from
AlpacaCCD (indi_alpaca_ccd)
    ↓ inherits from
INDI::CCD
```

## Seestar ImageBytes Protocol Bugs

The Seestar firmware (v1.1.2-1) implements the ASCOM Alpaca ImageBytes protocol incorrectly. This driver applies automatic workarounds:

### Bug #1: Scrambled Dimension Fields
- **Issue:** Metadata fields contain wrong values
- **Expected:** Rank=2, Dimension1=1920 (width), Dimension2=1080 (height)
- **Actual:** Rank=8, Dimension1=2, Dimension2=1080, Dimension3=1920
- **Workaround:** Extract actual values from wrong fields:
  - Actual Rank = Dimension1 field
  - Actual Width = Dimension3 field
  - Actual Height = Dimension2 field (correct)

### Bug #2: Wrong Element Type
- **Issue:** Reports Int32 (type 2) but sends Int16 data
- **Workaround:** Force Int16 (2 bytes per pixel) regardless of reported type

### Bug #3: Wrong DataStart
- **Issue:** Reports DataStart=0 instead of 44
- **Workaround:** Force DataStart=44 (size of metadata header)

## Implementation

The workarounds are applied by overriding the `parseImageBytesMetadata()` virtual method in the `SeestarCCD` class. The base `AlpacaCCD` class calls this method after receiving ImageBytes data, allowing derived classes to apply device-specific corrections.

## Features

### Core features available on Seestar
- Image capture with configurable exposure times
- Subframe/ROI support
- Temperature monitoring (read-only)
- Gain control (range exposed by firmware)
- Camera state and image-ready polling

### Seestar-specific handling in this driver
- Automatic ImageBytes metadata/data-type corrections
- Verified 1920x1080 image downloads
- Int16 (16-bit) image data

### Firmware-limited or unavailable features
- Binning beyond 1x1 (not available)
- Offset controls (not implemented)
- Cooler setpoint/power controls (not implemented)
- Fast readout (not implemented)
- Exposure progress/history endpoints (not implemented)
- Camera pulse guiding (`canpulseguide=false`)

## Connection Settings

Default connection parameters for Seestar S50:
- **Host:** seestar.local (or device IP address)
- **Port:** 32323
- **Device Number:** 0
- **Device Type:** camera

## Building

```bash
cd indi-seestar
mkdir -p build && cd build
cmake ..
make indi_seestar_ccd
sudo make install
```

## Testing

The camera can be tested using the standalone test program:

```bash
cd alpaca-tests/build
./camera_image_download_test 1.0 test_image
```

This downloads a 1-second exposure and saves:
- `test_image.imagebytes` - Raw binary image data (1920×1080×2 bytes)
- `test_image.imagebytes.json` - Metadata with corrected dimensions

## Running the Driver

```bash
# Start manually
indi_seestar_ccd

# Or through INDI server
indiserver indi_seestar_ccd

# Connect with client
indi_setprop "Seestar S50 CCD.CONNECTION.CONNECT=On"
```

## Files

- **indi_seestar_ccd.h** - Header file with class definition
- **indi_seestar_ccd.cpp** - Implementation with workarounds
- **CMakeLists.txt** - Build configuration
- **indi_seestar.xml** - INDI driver XML (includes both telescope and CCD)

## Related Documentation

- [SEESTAR_IMAGEBYTES_BUGS.md](../alpaca-tests/SEESTAR_IMAGEBYTES_BUGS.md) - Detailed analysis of protocol bugs
- [camera_image_download_test.cpp](../alpaca-tests/camera_image_download_test.cpp) - Test program source

## Known Limitations

1. Color image support not yet implemented (Seestar appears to send monochrome data)
2. Some exposure modes may not be fully tested
3. JSON ImageArray fallback available but slower than ImageBytes

## Future Improvements

- Implement color image support if Seestar adds it
- Add Seestar-specific camera capabilities detection
- Optimize buffer handling for large images
- Add configurable workaround enable/disable option

## License

GNU General Public License v2.0 or later

## Authors

- Rick Bassham - Seestar-specific implementation
- Jasem Mutlaq - Base Alpaca CCD driver
