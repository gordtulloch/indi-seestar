# Seestar CCD Driver Status

## Build Status: ✅ COMPLETE

Both INDI drivers have been successfully built and installed:
- `indi_seestar_telescope` - Telescope control driver
- `indi_seestar_ccd` - CCD camera driver

## Installation

Drivers installed to: `/usr/local/bin/`
Configuration file: `/usr/local/../share/indi/indi_seestar.xml`

## CCD Driver Features Implemented

### Camera Specifications
- **Sensor Size**: 1080 x 1920 pixels
- **Pixel Size**: 2.9 µm
- **Bit Depth**: 16-bit (maxADU: 65535)
- **Bayer Pattern**: GRBG (offsets X=1, Y=0)

### Supported Operations
- ✅ **Exposure Control**: 0.00003s - 2000s range
- ✅ **Gain Control**: 0-400 range
- ✅ **Subframe/ROI**: StartX, StartY, NumX, NumY
- ✅ **Image Download**: Via /imagearray JSON endpoint
- ✅ **Temperature Monitoring**: CCD temperature readout
- ✅ **Camera State Tracking**: Idle/Exposing/Reading/Download/Error
- ✅ **Abort Exposure**: Interrupt exposure in progress
- ✅ **Bayer Pattern**: GRBG color filter array

### Known Limitations
- ❌ **No Binning**: Only 1x1 binning supported (hardware limitation)
- ❌ **No Cooler**: No active temperature control
- ❌ **No Pulse Guiding**: Guide commands not available
- ❌ **No Fast Readout**: Single readout mode

## ASCOM Alpaca Camera API Support

Based on testing with Seestar v1.1.2-1 firmware, the camera supports 40 of 58 ASCOM Camera methods (69% success rate).

### Working Methods (40/58)
All essential camera operations are functional:
- Device info queries (name, description, version, etc.)
- Sensor properties (dimensions, pixel size, max ADU)
- Gain control (min/max/current)
- Exposure control (start/abort/duration/min/max)
- Image readout (imageready, imagearray, lastexposure)
- Temperature monitoring
- Frame/subframe configuration
- Camera state tracking

For full method test results, see: `Supported.Camera.md`

## Usage

### Starting the Drivers

Start both drivers together:
```bash
indiserver -v indi_seestar_telescope indi_seestar_ccd
```

Or start individually:
```bash
# Telescope only
indiserver -v indi_seestar_telescope

# CCD only
indiserver -v indi_seestar_ccd
```

### Connection Setup

1. **Configure Alpaca Server Address**:
   - Host: `seestar.local` (or IP address)
   - Port: `32323`
   - Device Number: `0`

2. **Connect the Driver**: 
   - Set `CONNECTED` property to `ON`
   - Driver will query device info and configure sensor

3. **Take Exposures**:
   - Set exposure duration in `CCD_EXPOSURE` property
   - Image will download automatically when ready
   - Monitor progress via `CCD_EXPOSURE_VALUE`

### Example: KStars/Ekos
1. Start INDI server with both drivers
2. In Ekos, add "Seestar" telescope and "Seestar CCD" camera
3. Configure connection (seestar.local:32323)
4. Connect both devices
5. Ready to image!

## Implementation Notes

### INDI API Version
Built against **INDI Library 2.1.7** with proper API compatibility:
- Uses `PropertyTP[index].setText()` for text properties
- Uses `PropertyNP[index].setValue()` for numeric properties  
- Uses `.apply()` method instead of `IDSet*()` functions
- Direct property indexing (e.g., `TemperatureNP[0]`)

### Alpaca Communication
- HTTP client: `cpp-httplib`
- JSON parser: `nlohmann::json`
- All endpoints use `/api/v1/camera/0/` prefix
- Transaction IDs incremented per request
- Error checking on all API calls

### Frame Buffer Management
- Allocates buffer based on sensor size (1080x1920x2 bytes)
- Converts JSON 2D array to flat uint16_t buffer
- Row-major order (Y then X iteration)
- Proper byte order for FITS format

## Next Steps

### Recommended Testing
1. ✅ Verify driver starts without errors
2. ✅ Confirm connection to Seestar
3. ⬜ Test short exposures (1-5s)
4. ⬜ Test long exposures (30-300s)
5. ⬜ Verify gain adjustment
6. ⬜ Test subframe/ROI selection
7. ⬜ Confirm abort functionality
8. ⬜ Check image quality and Bayer pattern
9. ⬜ Temperature monitoring over time
10. ⬜ Full imaging workflow with Ekos

### Future Enhancements
- **Filter Wheel Support**: 3-position wheel (Dark, IR, LP filters)
- **Focuser Control**: Absolute positioning 0-2600 steps
- **Streaming Mode**: Live view if camera supports it
- **Fast Download**: Optimize image download if possible
- **Error Recovery**: Robust handling of API errors
- **Multiple Devices**: Support for multiple Seestars on network

## Related Files
- `indi_seestar_ccd.h` - CCD driver header
- `indi_seestar_ccd.cpp` - CCD driver implementation
- `indi_seestar.h` - Telescope driver header  
- `indi_seestar.cpp` - Telescope driver implementation
- `indi_seestar.xml` - INDI device registration
- `Supported.Camera.md` - Full API test results

## Build Information
- **CMake**: 3.16+
- **Compiler**: GCC 13.3.0, C++17
- **INDI Library**: 2.1.7
- **Dependencies**: libindi, libnova, cfitsio, threads, cpp-httplib, nlohmann::json
- **Build Date**: 2026-01-02
- **Version**: 1.0.0
