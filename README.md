# INDI Alpaca Driver Suite

**Generic INDI drivers for ASCOM Alpaca-compatible devices + Seestar-specific extensions**

[![Status](https://img.shields.io/badge/Status-Testing-yellow)]()
[![INDI](https://img.shields.io/badge/INDI-2.1.7-blue)]()
[![License](https://img.shields.io/badge/License-LGPL--2.1-green)]()

## 🎯 Project Overview

This project provides:
1. **Generic INDI Alpaca drivers** - Work with any ASCOM Alpaca-compatible device
2. **Seestar-specific drivers** - Enhanced drivers for ZWO Seestar S50/S30 Pro smart telescopes

All drivers communicate via the standard ASCOM Alpaca REST API (default port 32323) and are compatible with KStars, Ekos, and any INDI-compatible application.

## 📦 Available Drivers

### Generic Alpaca Drivers (Base Classes)
Work with **any** ASCOM Alpaca-compatible device:

- **indi_alpaca_telescope** - Mount control (slewing, tracking, parking)
- **indi_alpaca_ccd** - Camera control (imaging, gain, temperature)
- **indi_alpaca_filterwheel** - Filter wheel control
- **indi_alpaca_focuser** - Focuser control (absolute/relative positioning)
- **indi_alpaca_dome** - Dome/observatory control

### Seestar-Specific Drivers
Inherit from generic drivers with Seestar enhancements:

- **indi_seestar_telescope** - Enhanced for Seestar S50/S30 Pro with:
  - Adjustable guide rates for autoguiding and dithering (v1.1.2-1+)
  - Device status display (model, firmware, SSID, state)
  - Dew heater control (v1.1.2-1+)
  - PulseGuide support for dithering (v1.1.2-1+)
  - S30 Pro dual camera/focuser support (telephoto + wide-angle)

## 🌟 Seestar Features

### Firmware v1.1.2-1+ Support
The Seestar drivers leverage features added in December 2025 firmware:

✅ **PulseGuide & IsPulseGuiding** - Autoguiding and dithering support  
✅ **Adjustable Guide Rates** - Control PulseGuide speed via INDI  
✅ **Park/Unpark** - Full parking support  
✅ **S30 Pro Wide-Angle** - Dual camera/focuser (device 0=tele, 1=wide)  
✅ **Switch Module** - Dew heater control  
✅ **Device SSID** - Shown in device info

### Firmware v1.2.0-3+ (Latest)
✅ Web update interface (accessed via Alpaca web UI)  
✅ Improved time synchronization

**Note**: Seestar does NOT support custom Alpaca actions. All features use standard ASCOM Alpaca endpoints.

## 🚧 Current Status: Active Development

**Generic Alpaca Drivers**: Beta quality - tested with multiple Alpaca devices  
**Seestar Drivers**: Alpha quality - actively being enhanced

### Status Summary
- ✅ All generic drivers built and functional with standard Alpaca devices
- ✅ Seestar telescope driver with v1.1.2-1 feature support
- ✅ API compatibility verified (48/52 GET methods working on Seestar)
- 🔄 Seestar CCD, focuser, filterwheel drivers planned
- 🔄 Integration testing with KStars/Ekos
- 📋 Bug reports and feedback welcome

### Tested Devices
- **ZWO Seestar S50** (Alpaca v1.1.2-1, v1.2.0-3)
- Generic Alpaca simulators and devices

## 📋 Features & Capabilities

### Telescope Driver
- Connection management and device discovery
- RA/Dec and Alt/Az positioning
- Slewing, tracking, and parking
- Multiple tracking modes (Sidereal, Lunar, Solar)
- Manual axis movement and guide rates
- Sync and alignment support
- Site location (latitude/longitude) query and configuration

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

#### Generic Alpaca Drivers

```bash
cd indi-alpaca
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
```

#### Seestar-Specific Drivers

```bash
cd indi-seestar
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
```

This installs:
- Drivers to `/usr/local/bin/`
- Configuration to `/usr/local/share/indi/`

**Note**: Seestar drivers require the Alpaca drivers to be built first (used as base classes).

## 🚀 Usage

### Starting the Drivers

```bash
# Generic Alpaca drivers (for any Alpaca device):
indiserver -v indi_alpaca_telescope indi_alpaca_ccd indi_alpaca_filterwheel indi_alpaca_focuser

# Seestar-specific driver (for ZWO Seestar):
indiserver -v indi_seestar_telescope indi_alpaca_ccd indi_alpaca_filterwheel indi_alpaca_focuser

# Individual drivers:
indiserver -v indi_seestar_telescope
indiserver -v indi_alpaca_ccd
```

### Connection Setup

**For Seestar S50/S30 Pro:**
1. **Configure Server Address**:
   - Host: `seestar.local` (or your Seestar's IP address)
   - Port: `32323` (Alpaca default)
   - Device Number: `0` (main telescope/camera), `1` (S30 Pro wide-angle, if available)

**For Generic Alpaca Devices:**
1. **Configure Server Address**:
   - Host: Your Alpaca device hostname or IP address
   - Port: `32323` (default Alpaca port, check your device)
   - Device Number: Usually `0` (check your device documentation)

2. **Connect**: Set CONNECTED property to ON

3. **Ready**: All device properties will populate automatically

### Seestar-Specific Features

After connecting with **indi_seestar_telescope**:
- **Status Tab**: View model, firmware, SSID, filter, temp, and state
- **Options Tab > Guide Rates**: Adjust RA/Dec guide rates (0.0-1.0) for autoguiding
- **Main Control > Dew Heater**: Toggle dew heater (requires Switch module)
- **Refresh Button**: Update all Seestar status information

### Using with KStars/Ekos

#### For Seestar S50/S30 Pro:
1. Start INDI server: `indiserver -v indi_seestar_telescope indi_alpaca_ccd indi_alpaca_filterwheel indi_alpaca_focuser`
2. In Ekos Equipment Profile:
   - Mount: "Seestar Telescope"
   - Camera: "Alpaca CCD"
   - Filter Wheel: "Alpaca FilterWheel"
   - Focuser: "Alpaca Focuser"
3. Configure connection to `seestar.local:32323`, device `0`
4. Enable guiding with adjustable guide rates
5. Connect and start imaging!

#### For Generic Alpaca Devices:
1. Start INDI server with generic drivers
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
- [Seestar_Alpaca_Changelog.txt](docs/Seestar_Alpaca_Changelog.txt) - Seestar firmware changelog

### Seestar-Specific Documentation
- **indi-seestar/README.md** - Seestar driver architecture and implementation
- Test results showing 48/52 GET methods working on Seestar v1.1.2-1

### API Testing

The drivers implement the ASCOM Alpaca API specification. Specific device capabilities may vary based on the hardware implementation.

## 🧪 Testing

### Test Programs

The `alpaca-tests/` directory contains validation programs:

```bash
cd alpaca-tests
mkdir build && cd build
cmake ..
make

# Run individual tests:
./alpaca_discovery_test
./telescope_complete_test seestar.local
./seestar_methods_test seestar.local    # Test Seestar-specific features
./camera_methods_test
./filterwheel_methods_test
./focuser_methods_test
```

See [alpaca-tests/README.md](alpaca-tests/README.md) for complete test program documentation.

### Seestar Test Results

Comprehensive testing on Seestar S50 v1.1.2-1:
- ✅ **48 of 52 GET methods working** (92% success rate)
- ✅ All capability queries functional
- ✅ Position, tracking, and status queries operational
- ✅ Park/Unpark working
- ✅ PulseGuide and guide rates working
- ❌ Custom actions not supported (method_sync/method_async return error 1036)
- ❌ Alt/Az slewing not implemented (canslewaltaz=false)

## 🐛 Known Issues & Limitations

### Generic Alpaca Drivers
- Device capabilities vary by manufacturer and firmware
- Some optional ASCOM methods may not be implemented by all devices
- Error handling depends on device firmware quality
- Coordinate updates may be slow during rapid slewing

### Seestar-Specific Limitations
- **No custom actions**: Seestar does not support ASCOM action extensions (error 1036)
- **No Alt/Az slewing**: Only equatorial (RA/Dec) coordinates supported
- **No SiteElevation**: Throws exception in v1.1.2-1+ (not supported)
- **Dew heater control**: Requires Switch module API (placeholder in driver)
- **S30 Pro wide-angle**: Requires device number `1` for wide-angle camera/focuser

### Architecture Notes
- Seestar drivers inherit from generic Alpaca drivers
- All Seestar features use standard ASCOM Alpaca endpoints
- No special protocol extensions needed

Check your device's documentation for specific capabilities and limitations.

## 🤝 Contributing

Contributions welcome! This project is in active development.

**How to help:**
- Test the drivers with your Alpaca devices or Seestar
- Report bugs via GitHub Issues
- Submit pull requests with fixes or enhancements
- Improve documentation
- Test Seestar-specific features and firmware updates

**Priority Areas:**
- Seestar CCD driver implementation
- Seestar focuser and filterwheel drivers
- Switch module integration for dew heater
- S30 Pro dual camera/focuser testing

## 📝 Project Structure

```
indi-seestar/
├── indi-alpaca/              # Generic Alpaca drivers (base classes)
│   ├── indi_alpaca_telescope.cpp/.h
│   ├── indi_alpaca_ccd.cpp/.h
│   ├── indi_alpaca_filterwheel.cpp/.h
│   ├── indi_alpaca_focuser.cpp/.h
│   ├── indi_alpaca_dome.cpp/.h
│   ├── alpaca_telescope_base.cpp    # Library version for inheritance
│   └── indi_alpaca.xml
├── indi-seestar/             # Seestar-specific drivers
│   ├── indi_seestar_telescope.cpp/.h
│   ├── indi_seestar.xml
│   ├── CMakeLists.txt
│   └── README.md
├── indi-switchbot/           # SwitchBot auxiliary driver
├── alpaca-tests/             # API validation test programs
│   ├── seestar_methods_test.cpp     # Seestar feature testing
│   └── ...
├── docs/                     # Detailed documentation
│   ├── Supported.*.md               # API support matrices
│   └── Seestar_Alpaca_Changelog.txt # Seestar firmware history
├── samples/                  # Sample code and API docs
└── README.md                 # This file
```

## 🔗 References

- [INDI Library](https://github.com/indilib/indi) - INDI framework
- [ASCOM Alpaca API](https://ascom-standards.org/api/) - API specification
- [ASCOM Initiative](https://ascom-standards.org/) - Standards organization
- [KStars/Ekos](https://edu.kde.org/kstars/) - Astronomy software
- [ZWO Seestar](https://www.zwoastro.com/product/seestar/) - Smart telescope
- [Seestar Alpaca Changelog](docs/Seestar_Alpaca_Changelog.txt) - Firmware updates

## 📄 License

LGPL-2.1 License - See individual source files for details.

## 🙏 Acknowledgments

- INDI development team for the excellent framework
- ASCOM Initiative for the Alpaca API standard
- ZWO for Seestar Alpaca implementation and documentation
- Contributors to Alpaca-compatible device firmware
- StellarMate for testing platform

## ⚠️ Disclaimer

This is an independent open-source project providing generic ASCOM Alpaca protocol drivers with optional Seestar enhancements. Use at your own risk. Always ensure your equipment is properly set up and supervised during automated operations.

---

**ASCOM Alpaca Protocol**: v1  
**INDI Library Version**: 2.1.7  
**Seestar Firmware Tested**: v1.1.2-1, v1.2.0-3  
**Last Updated**: February 3, 2026
