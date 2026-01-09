# alpaca Camera Specific Methods - Test Results

This document contains detailed test results for all ASCOM Alpaca Camera Specific Methods tested against alpaca firmware **v1.1.2-1**.

**Test Date**: 2025  
**Test Device**: alpaca at alpaca.local:32323  
**Device Path**: `/api/v1/camera/0/`

## Summary

- **Total Methods Tested**: 58 GET methods
- **Working Methods**: 40 (69%)
- **Not Implemented**: 17 (29%)
- **Request Failed**: 1 (2%)

## Key Findings

### ✅ Excellent Support For:
- **Camera properties**: Full sensor dimensions, pixel size, bayer pattern
- **Capabilities**: All 8 capability queries working
- **Binning**: Full binning information (though only 1x1 supported)
- **Subframe**: Complete subframe control available
- **Gain control**: Gain range 0-400, current gain readable
- **Temperature**: CCD temperature monitoring (26.9°C in test)
- **Exposure**: State, limits, and readiness checks

### ❌ Not Supported:
- **Offset control**: All offset methods not implemented
- **Cooling**: No cooler control (cangetcoolerpower=false)
- **Pulse guiding**: Not implemented (canpulseguide=false)
- **Fast readout**: Not supported
- **Exposure history**: Last exposure tracking not available
- **Progress tracking**: Percent completed not implemented
- **Sub-exposures**: Sub-exposure duration not implemented

### 📊 Sensor Details Discovered:
- **Resolution**: 1080x1920 pixels
- **Pixel Size**: 2.9µm x 2.9µm
- **Sensor Type**: 2 (Color sensor)
- **Bayer Pattern**: GRBG (offset X=1, Y=0)
- **Max ADU**: 65535 (16-bit)
- **Gain Range**: 0-400
- **Exposure Range**: 0.00003s - 2000s

## Complete Test Results

### Camera Properties

| Endpoint | Method | Status | Test Value | Notes |
|----------|--------|--------|------------|-------|
| `/cameraxsize` | GET | ✅ Working | 1080 | Sensor width in pixels |
| `/cameraysize` | GET | ✅ Working | 1920 | Sensor height in pixels |
| `/pixelsizex` | GET | ✅ Working | 2.9 | Pixel width in microns |
| `/pixelsizey` | GET | ✅ Working | 2.9 | Pixel height in microns |
| `/sensorname` | GET | ✅ Working | "" | Empty string returned |
| `/sensortype` | GET | ✅ Working | 2 | Color sensor (2=Color) |
| `/bayeroffsetx` | GET | ✅ Working | 1 | Bayer pattern X offset |
| `/bayeroffsety` | GET | ✅ Working | 0 | Bayer pattern Y offset |
| `/maxadu` | GET | ✅ Working | 65535 | Maximum ADU value (16-bit) |
| `/electronsperadu` | GET | ❌ Not Implemented | - | Error 1024 |
| `/fulwellcapacity` | GET | ❌ Request Failed | - | HTTP failure |

**Bayer Pattern**: With offsets X=1, Y=0, the pattern is GRBG

### Camera Capabilities

| Endpoint | Method | Status | Test Value | Notes |
|----------|--------|--------|------------|-------|
| `/canabortexposure` | GET | ✅ Working | true | Can abort exposures |
| `/canasymmetricbin` | GET | ✅ Working | false | Only symmetric binning |
| `/canfastreadout` | GET | ✅ Working | false | Fast readout not supported |
| `/cangetcoolerpower` | GET | ✅ Working | false | No cooler power info |
| `/canpulseguide` | GET | ✅ Working | false | Pulse guiding not supported |
| `/cansetccdtemperature` | GET | ✅ Working | false | Cannot control temperature |
| `/canstopexposure` | GET | ✅ Working | true | Can stop exposures |
| `/hasshutter` | GET | ✅ Working | false | No mechanical shutter |

### Binning

| Endpoint | Method | Status | Test Value | Notes |
|----------|--------|--------|------------|-------|
| `/maxbinx` | GET | ✅ Working | 1 | Max horizontal binning |
| `/maxbiny` | GET | ✅ Working | 1 | Max vertical binning |
| `/binx` | GET | ✅ Working | 1 | Current horizontal binning |
| `/biny` | GET | ✅ Working | 1 | Current vertical binning |

**Note**: Binning is not supported (max 1x1 only)

### Subframe

| Endpoint | Method | Status | Test Value | Notes |
|----------|--------|--------|------------|-------|
| `/startx` | GET | ✅ Working | 0 | Subframe start X position |
| `/starty` | GET | ✅ Working | 0 | Subframe start Y position |
| `/numx` | GET | ✅ Working | 1080 | Subframe width |
| `/numy` | GET | ✅ Working | 1920 | Subframe height |

**Note**: Full frame currently configured (0,0 to 1080x1920)

### Gain & Offset

| Endpoint | Method | Status | Test Value | Notes |
|----------|--------|--------|------------|-------|
| `/gain` | GET | ✅ Working | 0 | Current gain setting |
| `/gainmin` | GET | ✅ Working | 0 | Minimum gain value |
| `/gainmax` | GET | ✅ Working | 400 | Maximum gain value |
| `/gains` | GET | ❌ Not Implemented | - | Error 1024 |
| `/offset` | GET | ❌ Not Implemented | - | Error 1024 |
| `/offsetmin` | GET | ❌ Not Implemented | - | Error 1024 |
| `/offsetmax` | GET | ❌ Not Implemented | - | Error 1024 |
| `/offsets` | GET | ❌ Not Implemented | - | Error 1024 |

**Gain Range**: 0-400 (continuous range, discrete list not available)  
**Offset**: Not supported in alpaca firmware

### Readout Modes

| Endpoint | Method | Status | Test Value | Notes |
|----------|--------|--------|------------|-------|
| `/readoutmode` | GET | ✅ Working | 0 | Current readout mode |
| `/readoutmodes` | GET | ✅ Working | ["0"] | Available readout modes |
| `/fastreadout` | GET | ❌ Not Implemented | - | Error 1024 |

**Note**: Single readout mode available (mode 0)

### Temperature & Cooling

| Endpoint | Method | Status | Test Value | Notes |
|----------|--------|--------|------------|-------|
| `/ccdtemperature` | GET | ✅ Working | 26.9375 | Current CCD temp (°C) |
| `/setccdtemperature` | GET | ❌ Not Implemented | - | Error 1024 |
| `/cooleron` | GET | ❌ Not Implemented | - | Error 1024 |
| `/coolerpower` | GET | ❌ Not Implemented | - | Error 1024 |
| `/heatsinktemperature` | GET | ❌ Not Implemented | - | Error 1024 |

**Note**: Temperature monitoring only - no active cooling control

### Exposure

| Endpoint | Method | Status | Test Value | Notes |
|----------|--------|--------|------------|-------|
| `/camerastate` | GET | ✅ Working | 0 | Camera state (0=Idle) |
| `/imageready` | GET | ✅ Working | false | Image available flag |
| `/ispulseguiding` | GET | ❌ Not Implemented | - | Error 1024 |
| `/lastexposureduration` | GET | ❌ Not Implemented | - | Error 1024 |
| `/lastexposurestarttime` | GET | ❌ Not Implemented | - | Error 1024 |
| `/exposuremax` | GET | ✅ Working | 2000 | Max exposure (seconds) |
| `/exposuremin` | GET | ✅ Working | 0.00003 | Min exposure (seconds) |
| `/exposureresolution` | GET | ✅ Working | 0.000001 | Exposure resolution (1µs) |
| `/percentcompleted` | GET | ❌ Not Implemented | - | Error 1024 |

**Camera States**:
- 0 = Idle
- 1 = Waiting
- 2 = Exposing
- 3 = Reading
- 4 = Download
- 5 = Error

**Exposure Range**: 30µs to 2000 seconds (33.3 minutes)

### Sub-Exposure

| Endpoint | Method | Status | Test Value | Notes |
|----------|--------|--------|------------|-------|
| `/subexposureduration` | GET | ❌ Not Implemented | - | Error 1024 |

## Camera State Values

The `/camerastate` endpoint returns integer values:
- **0**: Idle - Not exposing
- **1**: Waiting - Waiting to start exposure
- **2**: Exposing - Exposure in progress
- **3**: Reading - Reading out from camera
- **4**: Download - Downloading image data
- **5**: Error - Error occurred

## Sensor Type Values

The `/sensortype` endpoint returns integer values:
- **0**: Monochrome
- **1**: Color (not Bayer encoded)
- **2**: RGGB Bayer encoding
- **3**: CMYG Bayer encoding
- **4**: CMYG2 Bayer encoding
- **5**: LRGB Truesense encoding

alpaca reports **2** (RGGB Bayer encoding)

## Bayer Pattern Determination

With the offsets:
- `bayeroffsetx` = 1
- `bayeroffsety` = 0

The Bayer pattern starting at pixel (0,0) is: **GRBG**

Standard patterns by offset:
- (0,0): RGGB
- (1,0): GRBG ← alpaca
- (0,1): GBRG
- (1,1): BGGR

## Implementation Notes

### For INDI Driver Development:

1. **Sensor Configuration**:
   - Fixed 1080x1920 resolution
   - 2.9µm square pixels
   - GRBG Bayer pattern
   - 16-bit depth (0-65535 ADU)

2. **Exposure Control**:
   - Support exposure range: 0.00003s to 2000s
   - Monitor `/camerastate` for progress
   - Use `/imageready` to check for completion
   - Can abort via `/abortexposure` PUT

3. **Gain Control**:
   - Continuous range 0-400
   - No discrete gain list available
   - Use slider/numeric control in UI

4. **Subframe Support**:
   - Can set custom ROI via `/startx`, `/starty`, `/numx`, `/numy`
   - Full frame is 0,0 to 1080x1920

5. **Temperature Monitoring**:
   - Read-only temperature available
   - No cooling control
   - Useful for environmental logging

6. **Not Available**:
   - No binning (1x1 only)
   - No offset control
   - No active cooling
   - No pulse guiding via camera
   - No exposure history tracking
   - No progress percentage

7. **Image Acquisition**:
   - Use `/startexposure` PUT with Duration parameter
   - Poll `/camerastate` until state=0 (Idle)
   - Check `/imageready` returns true
   - Retrieve image via `/imagearray` or `/imagearrayvariant`
   - Use `/stopexposure` or `/abortexposure` to cancel

## Related Documentation

- [Supported.Telescope.md](Supported.Telescope.md) - Telescope methods test results
- [Supported.Common.md](Supported.Common.md) - Common ASCOM methods test results
- [README.md](README.md) - Main project documentation

## Testing Information

**Test Program**: `camera_methods_test.cpp`  
**Build Command**: `cmake .. && make camera_methods_test`  
**Run Command**: `./camera_methods_test`

The test program connects to the camera device and queries all GET endpoints to determine implementation status.
