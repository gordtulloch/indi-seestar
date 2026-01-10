# INDI Alpaca Driver Suite

**Generic INDI driver implementation for ASCOM Alpaca-compatible devices via ASCOM Alpaca REST API**

[![Status](https://img.shields.io/badge/Status-Testing-yellow)]()
[![INDI](https://img.shields.io/badge/INDI-2.1.7-blue)]()
[![License](https://img.shields.io/badge/License-LGPL--2.1-green)]()

## 🎯 Project Overview

This project provides a complete set of generic INDI drivers for ASCOM Alpaca-compatible devices, enabling control through standard astronomy software like KStars, Ekos, and any INDI-compatible application.

These drivers can connect to any device that implements the ASCOM Alpaca protocol, including smart telescopes, astronomical cameras, filter wheels, focusers, and observatory domes.

### Available Drivers

- **indi_alpaca_telescope** - Mount control (slewing, tracking, parking)
- **indi_alpaca_ccd** - Camera control (imaging, gain, temperature)
- **indi_alpaca_filterwheel** - Filter wheel control
- **indi_alpaca_focuser** - Focuser control (absolute/relative positioning)
- **indi_alpaca_dome** - Dome/observatory control (azimuth, shutter, slaving)

All drivers communicate via the standard ASCOM Alpaca REST API (default port 32323).

## 🚧 Current Status: Active Development

**⚠️ These drivers are generic implementations of the ASCOM Alpaca protocol and should be considered BETA quality.**

- ✅ All five drivers built and installed
- ✅ API compatibility verified with multiple Alpaca devices
- 🔄 Testing with various Alpaca-compatible hardware
- 🔄 Integration testing with KStars/Ekos
- 📋 Bug reports and feedback welcome

## 📋 Features & Capabilities

### Telescope Driver
- Connection management and device discovery
- RA/Dec and Alt/Az positioning
- Slewing, tracking, and parking
- Multiple tracking modes (Sidereal, Lunar, Solar)
- Manual axis movement and guide rates
- Sync and alignment support

### CCD Driver
- Image capture with configurable exposure
- Gain and offset control
- Subframe/ROI support
- Temperature monitoring
- Abort capability
- Multiple image formats

### FilterWheel Driver
- Multi-position filter wheel support
- Filter naming and identification
- Focus offset management
- Position tracking

### Focuser Driver
- Absolute and relative positioning
- Temperature monitoring
- Halt capability
- Position tracking

### Dome Driver
- Azimuth control
- Shutter control (open/close)
- Park and home operations
- Telescope slaving
- Abort capability

**Note**: Actual capabilities depend on your specific Alpaca device implementation. Not all devices support all features.

## 🔧 Installation

### Prerequisites

```bash
# On Debian/Ubuntu/StellarMate:
sudo apt-get update
sudo apt-get install build-essential cmake libindi-dev libcfitsio-dev
```

### Building from Source

```bash
cd indi-alpaca
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
   - Host: Your Alpaca device hostname or IP address
   - Port: `32323` (default Alpaca port)
   - Device Number: Usually `0` (check your device documentation)

2. **Connect**: Set CONNECTED property to ON

3. **Ready**: All device properties will populate automatically

### Using with KStars/Ekos

1. Start INDI server with desired drivers
2. In Ekos Equipment Profile:
   - Mount: "Alpaca Telescope"
   - Camera: "Alpaca CCD"
   - Filter Wheel: "Alpaca FilterWheel"
   - Focuser: "Alpaca Focuser"
   - Dome: "Alpaca Dome"
3. Configure connection settings for each device
4. Connect and start imaging!

## 📚 Documentation

Detailed documentation available in the `docs/` folder:

- [Supported.Common.md](docs/Supported.Common.md) - Common ASCOM methods
- [Supported.Telescope.md](docs/Supported.Telescope.md) - Telescope methods
- [Supported.Camera.md](docs/Supported.Camera.md) - Camera methods
- [Supported.FilterWheel.md](docs/Supported.FilterWheel.md) - FilterWheel methods
- [Supported.Focuser.md](docs/Supported.Focuser.md) - Focuser methods
- [CCD_DRIVER_STATUS.md](docs/CCD_DRIVER_STATUS.md) - CCD driver implementation details
- [FILTERWHEEL_DRIVER_STATUS.md](docs/FILTERWHEEL_DRIVER_STATUS.md) - FilterWheel driver details

### API Testing

The drivers implement the ASCOM Alpaca API specification. Specific device capabilities may vary based on the hardware implementation.

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

### General
- Device capabilities vary by manufacturer and firmware
- Some optional ASCOM methods may not be implemented by all devices
- Error handling depends on device firmware quality

### Driver Status
- Coordinate updates may be slow during rapid slewing
- Some devices may return "not implemented" for optional methods
- Feature availability depends on specific device implementation

Check your device's documentation for specific capabilities and limitations.

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
│   ├── alpaca_telescope.cpp/.h         # Telescope driver
│   ├── alpaca_ccd.cpp/.h               # CCD driver
│   ├── alpaca_filterwheel.cpp/.h       # FilterWheel driver
│   ├── alpaca_focuser.cpp/.h           # Focuser driver
│   ├── alpaca_dome.cpp/.h              # Dome driver
│   ├── indi-alpaca.xml                 # INDI device registration
│   └── CMakeLists.txt                  # Build configuration
├── alpaca-tests/           # API validation test programs
├── docs/                   # Detailed documentation
└── README.md              # This file
```

## 🔗 References

- [INDI Library](https://github.com/indilib/indi) - INDI framework
- [ASCOM Alpaca API](https://ascom-standards.org/api/) - API specification
- [ASCOM Initiative](https://ascom-standards.org/) - Standards organization
- [KStars/Ekos](https://edu.kde.org/kstars/) - Astronomy software

## 📄 License

LGPL-2.1 License - See individual source files for details.

## 🙏 Acknowledgments

- INDI development team for the excellent framework
- ASCOM Initiative for the Alpaca API standard
- Contributors to Alpaca-compatible device firmware
- StellarMate for testing platform

## ⚠️ Disclaimer

This is an independent open-source project providing generic ASCOM Alpaca protocol drivers. Use at your own risk. Always ensure your equipment is properly set up and supervised during automated operations.

---

**ASCOM Alpaca Protocol**: v1  
**INDI Library Version**: 2.1.7  
**Last Updated**: January 10, 2026
