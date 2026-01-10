# INDI Alpaca Drivers

Generic INDI driver implementation for ASCOM Alpaca-compatible devices using the ASCOM Alpaca REST API.

These drivers provide a bridge between INDI-based astronomy software (like KStars/Ekos) and any device that implements the ASCOM Alpaca protocol, including telescopes, cameras, filter wheels, focusers, and domes.

## Build Requirements

- INDI Library (>= 1.9.0)
- CMake (>= 3.16)
- libcurl
- C++17 compatible compiler
- pthread

### Install Dependencies (Ubuntu/Debian)

```bash
sudo apt-get update
sudo apt-get install \
    build-essential \
    cmake \
    libindi-dev \
    libcurl4-openssl-dev \
    libnova-dev
```

## Building

```bash
cd indi-alpaca
mkdir build
cd build
cmake ..
make
sudo make install
```

### Build Status
✅ **Successfully built and installed!**
- Tested on Ubuntu Noble (StellarMate)
- libindi-dev 2.1.7
- libnova-dev 0.16
- libcurl 8.5.0
- GCC 13.3.0

Binaries installed to: `/usr/local/bin/`
- `indi_alpaca_telescope`
- `indi_alpaca_ccd`
- `indi_alpaca_filterwheel`
- `indi_alpaca_focuser`
- `indi_alpaca_dome`

XML config installed to: `/usr/local/share/indi/indi_alpaca.xml`

## Testing the Driver

### 1. Start the driver
```bash
# Start INDI server with one or more Alpaca drivers
indiserver indi_alpaca_telescope
indiserver indi_alpaca_ccd
indiserver indi_alpaca_telescope indi_alpaca_ccd indi_alpaca_focuser

# Or with verbose output for debugging
indiserver -v indi_alpaca_telescope
```

### 2. Test with INDI tools
```bash
# List all properties (replace device_name with actual device name)
indi_getprop "Alpaca Telescope.*"

# Check connection status
indi_getprop "Alpaca Telescope.CONNECTION.*"

# Connect to the device (make sure Alpaca device is on and accessible)
indi_setprop "Alpaca Telescope.CONNECTION.CONNECT=On"
```

### 3. Test with a GUI client
- **KStars**: Add Alpaca devices under Equipment Manager
- **INDI Control Panel**: Connect and control through the GUI

## Configuration

The drivers connect to Alpaca-compatible devices via the ASCOM Alpaca REST API.

Default settings:
- **Host**: alpaca.local (configurable for your device's hostname/IP)
- **Port**: 32323 (standard Alpaca port, but configurable)
- **Device Number**: 0

These can be changed in the driver's connection settings.

## Features

### Available Drivers

#### Telescope Driver (indi_alpaca_telescope)
- Connection management
- Position reporting (RA/Dec, Alt/Az)
- GoTo (slew to coordinates)
- Sync
- Abort slew
- Park/Unpark
- Tracking enable/disable
- Tracking rate selection (Sidereal/Lunar/Solar)
- Manual motion control (N/S/E/W)
- Device information display

#### Camera Driver (indi_alpaca_ccd)
- Exposure control
- Image download
- Temperature monitoring
- Gain/offset control
- Binning support

#### FilterWheel Driver (indi_alpaca_filterwheel)
- Filter position control
- Filter name display
- Position reporting

#### Focuser Driver (indi_alpaca_focuser)
- Absolute positioning
- Temperature compensation
- Position reporting

#### Dome Driver (indi_alpaca_dome)
- Azimuth control
- Shutter control
- Slaving to telescope
- Park/Home operations

### Planned Enhancements

- Enhanced error handling
- State management improvements
- Device-specific action support
- Multi-device coordination

## Known Limitations

These are general Alpaca protocol limitations - specific device capabilities may vary:

### General
- Capabilities depend on the specific Alpaca device implementation
- Some devices may not implement all optional Alpaca methods
- Error handling depends on device firmware quality

See device-specific documentation for detailed API support.

## Usage

### Starting the Drivers

```bash
# Start INDI server with Alpaca drivers
indiserver indi_alpaca_telescope
indiserver indi_alpaca_ccd
indiserver indi_alpaca_telescope indi_alpaca_ccd indi_alpaca_focuser
```

### With INDI Control Panel

1. Start INDI server: `indiserver indi_alpaca_telescope`
2. Launch INDI Control Panel: `indi_control_panel`
3. Connect to localhost:7624
4. Configure connection settings (host/port/device number)
5. Connect to device

### With KStars/Ekos

1. Open KStars
2. Go to Tools → Ekos
3. Add Profile
4. Select Alpaca devices from the equipment lists
5. Start Ekos
6. Connect and use

## Testing

Test programs are available in the `../tests` directory for API validation.

## Documentation

- [README.md](../README.md) - Main project documentation
- [AlpacaAPIComparison.md](../AlpacaAPIComparison.md) - Complete API support comparison
- [DEVELOPMENT_CHECKLIST.md](../DEVELOPMENT_CHECKLIST.md) - Implementation checklist
- [Supported.Telescope.md](../Supported.Telescope.md) - Detailed telescope API test results
- [Supported.Camera.md](../Supported.Camera.md) - Camera API test results
- [Supported.FilterWheel.md](../Supported.FilterWheel.md) - FilterWheel API test results
- [Supported.Focuser.md](../Supported.Focuser.md) - Focuser API test results

## Troubleshooting

### Cannot connect to Alpaca device

1. Verify device is powered on and on network
2. Check hostname resolution: `ping <your-device-hostname>`
3. Verify Alpaca API port: `curl http://<device-host>:<port>/api/v1/telescope/0/connected`
4. Check firewall settings
5. Verify device number matches your Alpaca device configuration

### Operations fail with "Invalid while parked"

Some devices must be unparked before operations. Use the Unpark button/command.

### Device not responding

1. Check INDI logs for error messages
2. Verify Alpaca API is responding with curl/browser
3. Check device number configuration (usually 0)
4. Ensure no other application is controlling the device

## Contributing

See [DEVELOPMENT_CHECKLIST.md](../DEVELOPMENT_CHECKLIST.md) for development roadmap.

## License

LGPL 2.1 or later

## Author

Gord Tulloch

## Acknowledgments

- INDI Library developers
- ASCOM Initiative for Alpaca API specification
- Contributors to Alpaca device firmware

## Version History

### 1.0.0 (In Development)
- Initial release of generic Alpaca drivers
- Telescope interface implementation
- Camera interface implementation (from INDI core)
- FilterWheel interface implementation
- Focuser interface implementation
- Dome interface implementation (enhanced from INDI core)
