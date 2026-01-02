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

The Seestar v1.1.2-1 firmware has excellent ASCOM Alpaca support with comprehensive implementation of standard methods.

**Quick Summary:**
- **Common Methods:** 7/14 working (50%) - Core connection and info methods functional
- **Telescope GET Methods:** 48/52 working (92%)
- **Telescope PUT Methods:** 32/32 tested (100% coverage)
- **Camera GET Methods:** 40/58 working (69%)
- **FilterWheel Methods:** 3/3 GET working (100%), 1/1 PUT working (100%)
- **Focuser GET Methods:** 8/9 working (89%)
- **Focuser PUT Methods:** 2/4 working (50%)
- **Working Features:** Tracking, slewing, parking, guide rates, site location, manual axis movement, camera exposure control, gain control, temperature monitoring, filter wheel control (3 positions: Dark, IR, LP), focuser absolute positioning (0-2600 steps)
- **Main Limitations:** No Alt/Az slewing, no camera cooling control, no binning support, focuser absolute-only positioning (no relative moves)

**Detailed Documentation:**
- [Common Methods Support](Supported.Common.md) - Connection, device info, command methods
- [Telescope Methods Support](Supported.Telescope.md) - All telescope-specific methods
- [Camera Methods Support](Supported.Camera.md) - All camera-specific methods
- [FilterWheel Methods Support](Supported.FilterWheel.md) - All filterwheel-specific methods
- [Focuser Methods Support](Supported.Focuser.md) - All focuser-specific methods

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
