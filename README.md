# INDI Seestar Driver

INDI driver for the Seestar smart telescope, providing control via the ASCOM Alpaca REST API.

## Project Structure

```
indi-seestar/
├── tests/          # Test programs for concept validation
├── .vscode/        # VS Code configuration
└── README.md       # This file
```

## Development Approach

This project follows an incremental development approach:

1. **Test Programs** - Small C++ programs to test and validate individual concepts
2. **Driver Implementation** - Full INDI driver based on validated approaches
3. **Integration Testing** - End-to-end testing with astronomy software

## Current Status

Currently developing test programs to validate ASCOM Alpaca API connectivity and functionality.

### Seestar Alpaca Firmware
**Tested Version:** v1.1.2-1  
**Alpaca Port:** 32323

### Test Programs

See the [tests directory](tests/README.md) for available test programs.

## ASCOM Alpaca API Support

The following table shows the test status of all Telescope Specific Methods in the ASCOM Alpaca API for Seestar v1.1.2-1:

### Telescope Specific Methods - Test Status

| Method | Type | Endpoint | Test Status | Notes |
|--------|------|----------|-------------|-------|
| **Alignment & Capabilities** |
| alignmentmode | GET | /alignmentmode | ✅ Working | Returns 1 (algAltAz) |
| canfindhome | GET | /canfindhome | ✅ Working | Returns true |
| canpark | GET | /canpark | ✅ Working | Returns true |
| canpulseguide | GET | /canpulseguide | ✅ Working | Returns true |
| cansetdeclinationrate | GET | /cansetdeclinationrate | ✅ Working | Returns false |
| cansetguiderates | GET | /cansetguiderates | ✅ Working | Returns true |
| cansetpark | GET | /cansetpark | ✅ Working | Returns false |
| cansetpierside | GET | /cansetpierside | ✅ Working | Returns false |
| cansetrightascensionrate | GET | /cansetrightascensionrate | ✅ Working | Returns false |
| cansettracking | GET | /cansettracking | ✅ Working | Returns true |
| canslewaltaz | GET | /canslewaltaz | ✅ Working | Returns false (not supported) |
| canslewaltazasync | GET | /canslewaltazasync | ✅ Working | Returns false (not supported) |
| canslewasync | GET | /canslewasync | ✅ Working | Returns true |
| canslew | GET | /canslew | ✅ Working | Returns true |
| cansync | GET | /cansync | ✅ Working | Returns true |
| cansyncaltaz | GET | /cansyncaltaz | ✅ Working | Returns false |
| canunpark | GET | /canunpark | ✅ Working | Returns true |
| canmoveaxis | GET | /canmoveaxis | ✅ Working | Returns true for both axes |
| **Position & Status** |
| altitude | GET | /altitude | ✅ Working | Current altitude in degrees |
| azimuth | GET | /azimuth | ✅ Working | Current azimuth in degrees |
| athome | GET | /athome | ✅ Working | Home position status |
| atpark | GET | /atpark | ✅ Working | Park position status |
| declination | GET | /declination | ✅ Working | Current declination in degrees |
| rightascension | GET | /rightascension | ✅ Working | Current RA in hours |
| sideofpier | GET | /sideofpier | ❌ Not Implemented | Error 1024 |
| siderealtime | GET | /siderealtime | ✅ Working | Local sidereal time |
| slewing | GET | /slewing | ✅ Working | Slewing status |
| **Tracking** |
| tracking | GET | /tracking | ✅ Working | Tracking on/off status |
| tracking | PUT | /tracking | ✅ Working | Enable/disable tracking |
| trackingrate | GET | /trackingrate | ✅ Working | Current tracking rate |
| trackingrate | PUT | /trackingrate | ⚪ Not Tested | |
| trackingrates | GET | /trackingrates | ✅ Working | Available tracking rates |
| **Rates** |
| declinationrate | GET | /declinationrate | ✅ Working | Returns 0 |
| declinationrate | PUT | /declinationrate | ⚪ Not Tested | |
| rightascensionrate | GET | /rightascensionrate | ✅ Working | Returns 0 |
| rightascensionrate | PUT | /rightascensionrate | ⚪ Not Tested | |
| guideratedeclination | GET | /guideratedeclination | ✅ Working | Guide rate in degrees/sec |
| guideratedeclination | PUT | /guideratedeclination | ⚪ Not Tested | |
| guideraterightascension | GET | /guideraterightascension | ✅ Working | Guide rate in degrees/sec |
| guideraterightascension | PUT | /guideraterightascension | ⚪ Not Tested | |
| **Site Information** |
| siteelevation | GET | /siteelevation | ❌ Not Implemented | Error 1024 |
| siteelevation | PUT | /siteelevation | ⚪ Not Tested | |
| sitelatitude | GET | /sitelatitude | ✅ Working | Site latitude in degrees |
| sitelatitude | PUT | /sitelatitude | ⚪ Not Tested | |
| sitelongitude | GET | /sitelongitude | ✅ Working | Site longitude in degrees |
| sitelongitude | PUT | /sitelongitude | ⚪ Not Tested | |
| **Target Coordinates** |
| targetdeclination | GET | /targetdeclination | ⚠️ Requires Setup | Error 1026 if not set |
| targetdeclination | PUT | /targetdeclination | ⚪ Not Tested | |
| targetrightascension | GET | /targetrightascension | ⚠️ Requires Setup | Error 1026 if not set |
| targetrightascension | PUT | /targetrightascension | ⚪ Not Tested | |
| **Telescope Properties** |
| aperturearea | GET | /aperturearea | ❌ Not Implemented | Error 1024 |
| aperturediameter | GET | /aperturediameter | ❌ Not Implemented | Error 1024 |
| equatorialsystem | GET | /equatorialsystem | ✅ Working | Returns 1 (J2000) |
| focallength | GET | /focallength | ❌ Not Implemented | Error 1024 |
| doesrefraction | GET | /doesrefraction | ✅ Working | Returns false |
| doesrefraction | PUT | /doesrefraction | ⚪ Not Tested | |
| **Movement Commands** |
| findhome | PUT | /findhome | ✅ Working | Returns to home position |
| park | PUT | /park | ❌ Not Working | Error 1024 despite canpark=true |
| unpark | PUT | /unpark | ⚪ Not Tested | |
| setpark | PUT | /setpark | ⚪ Not Tested | |
| abortslew | PUT | /abortslew | ⚪ Not Tested | |
| moveaxis | PUT | /moveaxis | ⚪ Not Tested | |
| **Slewing** |
| slewtoaltaz | PUT | /slewtoaltaz | ❌ Not Supported | canslewaltaz=false |
| slewtoaltazasync | PUT | /slewtoaltazasync | ❌ Not Supported | canslewaltazasync=false |
| slewtocoordinates | PUT | /slewtocoordinates | ✅ Working | Requires tracking enabled |
| slewtocoordinatesasync | PUT | /slewtocoordinatesasync | ⚪ Not Tested | |
| slewtotarget | PUT | /slewtotarget | ⚪ Not Tested | |
| slewtotargetasync | PUT | /slewtotargetasync | ⚪ Not Tested | |
| destinationsideofpier | GET | /destinationsideofpier | ⚪ Not Tested | |
| **Syncing** |
| synctoaltaz | PUT | /synctoaltaz | ⚪ Not Tested | |
| synctocoordinates | PUT | /synctocoordinates | ⚪ Not Tested | |
| synctotarget | PUT | /synctotarget | ⚪ Not Tested | |
| **Pulse Guiding** |
| pulseguide | PUT | /pulseguide | ⚪ Not Tested | |
| ispulseguiding | GET | /ispulseguiding | ✅ Working | Returns false |
| **Axis Rates** |
| axisrates | GET | /axisrates | ✅ Working | Returns rate ranges for axes |

**Legend:**
- ✅ Working - Tested and confirmed functional
- ❌ Not Working - Tested and returns error/not implemented
- ⚠️ Requires Setup - Functional but needs configuration
- ⚪ Not Tested - Not yet tested

### Summary

**GET Methods:** 48/52 working (92%)  
**PUT Methods Tested:** 3/32 (findhome ✅, tracking ✅, park ❌, slewtocoordinates ✅)

The Seestar v1.1.2-1 has excellent support for ASCOM Alpaca telescope GET methods, with most queries working correctly. Notable limitations include lack of Alt/Az slewing support and some telescope property queries.

## Requirements

### Build Dependencies
- CMake 3.10+
- C++11 compatible compiler
- libcurl development files

### Runtime Dependencies
- INDI library (for final driver)
- Seestar telescope accessible on network

## Installation of Dependencies

On Debian/Ubuntu systems (including StellarMate):
```bash
sudo apt-get update
sudo apt-get install build-essential cmake libcurl4-openssl-dev
```

## Building

### Test Programs
```bash
cd tests
mkdir build
cd build
cmake ..
make
```

## Usage

See individual test program documentation in the [tests directory](tests/README.md).

## Contributing

This is an early-stage development project. Test programs are being developed first to validate concepts before implementing the full INDI driver.

## License

TBD

## References

- [INDI Library](https://github.com/indilib/indi)
- [ASCOM Alpaca API](https://ascom-standards.org/api/)
- [Seestar Telescope](https://www.zwoastro.com/seestar/)
