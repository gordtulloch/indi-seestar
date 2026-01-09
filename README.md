# INDI alpaca Driver Suite

**Complete INDI driver implementation for the alpaca smart telescope via ASCOM Alpaca REST API**

[![Status](https://img.shields.io/badge/Status-Testing-yellow)]()
[![INDI](https://img.shields.io/badge/INDI-2.1.7-blue)]()
[![License](https://img.shields.io/badge/License-LGPL--2.1-green)]()

## 🎯 Project Overview

This project provides a complete set of INDI drivers for the ZWO alpaca smart telescope, enabling control through standard astronomy software like KStars, Ekos, and any INDI-compatible application.

### Available Drivers

- **indi_alpaca_telescope** - Mount control (slewing, tracking, parking)
- **indi_alpaca_ccd** - Camera control (imaging, gain, temperature)
- **indi_alpaca_filterwheel** - Filter wheel control (Dark, IR, LP filters)
- **indi_alpaca_focuser** - Focuser control (0-2600 steps, absolute positioning)

All drivers communicate with the alpaca via its built-in ASCOM Alpaca REST API (port 32323).

## 🚧 Current Status: Active Testing

**⚠️ These drivers are currently under active testing. While fully implemented, they should be considered BETA quality.**

- ✅ All four drivers built and installed
- ✅ API compatibility verified with alpaca v1.1.2-1
- 🔄 Field testing in progress
- 🔄 Integration testing with KStars/Ekos
- 📋 Bug reports and feedback welcome

## 📋 Features & Capabilities

### Telescope Driver
- **Working**: RA/Dec positioning, slewing, tracking modes, parking, site location, manual axis movement, guide rates
- **Tested**: 65/84 methods (77% coverage)
- **Limitations**: No Alt/Az slewing support

### CCD Driver
- **Sensor**: 1080x1920 pixels, 2.9µm pixel size, 16-bit depth
- **Bayer Pattern**: GRBG color filter array
- **Gain**: 0-400 range
- **Exposure**: 0.00003s - 2000s
- **Working**: Image capture, gain control, subframe/ROI, temperature monitoring, abort
- **Tested**: 40/58 methods (69% coverage)
- **Limitations**: No binning, no cooler, no pulse guiding

### FilterWheel Driver
- **Filters**: 3-position wheel (Dark, IR, LP)
- **Focus Offsets**: Configurable per filter
- **Working**: All filter selection and query methods
- **Tested**: 4/4 methods (100% coverage)

### Focuser Driver
- **Range**: 0-2600 steps
- **Mode**: Absolute positioning only
- **Working**: Absolute moves, halt, temperature monitoring, position tracking
- **Tested**: 10/13 methods (77% coverage)
- **Limitations**: No relative moves (hardware limitation)

## 🔧 Installation

### Prerequisites

```bash
# On Debian/Ubuntu/StellarMate:
sudo apt-get update
sudo apt-get install build-essential cmake libindi-dev libcfitsio-dev
```

### Building from Source

```bash
cd indi-alpaca/indi-alpaca
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
```

This installs:
- Drivers to `/usr/local/bin/`
- Configuration to `/usr/local/share/indi/`

## 🚀 Usage

### Starting the Drivers

```bash
# All drivers together:
indiserver -v indi_alpaca_telescope indi_alpaca_ccd indi_alpaca_filterwheel indi_alpaca_focuser

# Individual drivers:
indiserver -v indi_alpaca_telescope
indiserver -v indi_alpaca_ccd
```

### Connection Setup

1. **Configure Server Address** (in INDI client):
   - Host: `alpaca.local` (or IP address)
   - Port: `32323`
   - Device Number: `0`

2. **Connect**: Set CONNECTED property to ON

3. **Ready**: All device properties will populate automatically

### Using with KStars/Ekos

1. Start INDI server with desired drivers
2. In Ekos Equipment Profile:
   - Mount: "alpaca"
   - Camera: "alpaca CCD"
   - Filter Wheel: "alpaca FilterWheel"
   - Focuser: "alpaca Focuser"
3. Configure connection settings for each device
4. Connect and start imaging!

## 📚 Documentation

Detailed documentation available in the `docs/` folder:

- [Supported.Common.md](docs/Supported.Common.md) - Common ASCOM methods (7/14 working)
- [Supported.Telescope.md](docs/Supported.Telescope.md) - Telescope methods (65/84 working)
- [Supported.Camera.md](docs/Supported.Camera.md) - Camera methods (40/58 working)
- [Supported.FilterWheel.md](docs/Supported.FilterWheel.md) - FilterWheel methods (4/4 working)
- [Supported.Focuser.md](docs/Supported.Focuser.md) - Focuser methods (10/13 working)
- [CCD_DRIVER_STATUS.md](docs/CCD_DRIVER_STATUS.md) - CCD driver implementation details
- [FILTERWHEEL_DRIVER_STATUS.md](docs/FILTERWHEEL_DRIVER_STATUS.md) - FilterWheel driver details

### API Testing Results

Comprehensive API testing completed against alpaca v1.1.2-1:
- **Overall**: 126/173 methods working (73% success rate)
- All critical imaging and positioning functions operational
- See documentation files for complete method-by-method results

## 🧪 Testing

### Test Programs

The `alpaca-tests/` directory contains validation programs used during development:

```bash
cd alpaca-tests
mkdir build && cd build
cmake ..
make

# Run individual tests:
./alpaca_discovery_test
./telescope_complete_test
./camera_methods_test
./filterwheel_methods_test
./focuser_methods_test
```

See [alpaca-tests/README.md](alpaca-tests/README.md) for complete test program documentation.

## 🐛 Known Issues & Limitations

### Hardware Limitations (alpaca)
- No Alt/Az slewing support
- No camera binning
- No camera cooling control
- Focuser absolute positioning only (no relative moves)
- No pulse guiding

### Driver Status
- Coordinate updates may be slow during rapid slewing
- Temperature compensation not available (focuser)
- Some ASCOM methods return "not implemented" errors

## 🤝 Contributing

Contributions welcome! This project is in active development.

**How to help:**
- Test the drivers with your alpaca
- Report bugs via GitHub Issues
- Submit pull requests with fixes or enhancements
- Improve documentation

## 📝 Project Structure

```
indi-alpaca/
├── indi-alpaca/           # Driver source code
│   ├── indi_alpaca.cpp/.h              # Telescope driver
│   ├── indi_alpaca_ccd.cpp/.h          # CCD driver
│   ├── indi_alpaca_filterwheel.cpp/.h  # FilterWheel driver
│   ├── indi_alpaca_focuser.cpp/.h      # Focuser driver
│   ├── indi_alpaca.xml                 # INDI device registration
│   └── CMakeLists.txt                   # Build configuration
├── alpaca-tests/           # API validation test programs
├── docs/                   # Detailed documentation
└── README.md              # This file
```

## 🔗 References

- [INDI Library](https://github.com/indilib/indi) - INDI framework
- [ASCOM Alpaca API](https://ascom-standards.org/api/) - API specification
- [alpaca Telescope](https://www.zwoastro.com/alpaca/) - Hardware information
- [KStars/Ekos](https://edu.kde.org/kstars/) - Astronomy software

## 📄 License

LGPL-2.1 License - See individual source files for details.

## 🙏 Acknowledgments

- INDI development team for the excellent framework
- ASCOM Initiative for the Alpaca API standard
- ZWO for the alpaca smart telescope
- StellarMate for testing platform

## ⚠️ Disclaimer

This is an independent open-source project and is not officially endorsed or supported by ZWO. Use at your own risk. Always ensure your equipment is properly set up and supervised during automated operations.

---

**alpaca Firmware Tested:** v1.1.2-1  
**INDI Library Version:** 2.1.7  
**Last Updated:** January 2, 2026
