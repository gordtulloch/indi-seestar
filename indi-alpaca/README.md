# INDI alpaca Driver

INDI driver implementation for the ZWO alpaca smart telescope using the ASCOM Alpaca REST API.

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

Binary installed to: `/usr/local/bin/indi_alpaca_telescope`
XML config installed to: `/usr/local/share/indi/indi_alpaca.xml`

## Testing the Driver

### 1. Start the driver
```bash
# Start INDI server with the alpaca driver
indiserver indi_alpaca_telescope

# Or with verbose output for debugging
indiserver -v indi_alpaca_telescope
```

### 2. Test with INDI tools
```bash
# List all properties
indi_getprop "alpaca.*"

# Check connection status
indi_getprop "alpaca.CONNECTION.*"

# Connect to the telescope (make sure alpaca is on and accessible)
indi_setprop "alpaca.CONNECTION.CONNECT=On"
```

### 3. Test with a GUI client
- **KStars**: Add alpaca under Equipment Manager → Telescopes
- **INDI Control Panel**: Connect and control through the GUI

## Configuration

The driver connects to the alpaca via its Alpaca API on port 32323.

Default settings:
- **Host**: alpaca.local
- **Port**: 32323
- **Device Number**: 0

These can be changed in the driver's connection settings.

## Features

### Currently Implemented

#### Telescope Interface
- ✅ Connection management
- ✅ Position reporting (RA/Dec, Alt/Az)
- ✅ GoTo (slew to coordinates)
- ✅ Sync
- ✅ Abort slew
- ✅ Park/Unpark
- ✅ Tracking enable/disable
- ✅ Tracking rate selection (Sidereal/Lunar/Solar)
- ✅ Manual motion control (N/S/E/W)
- ✅ Find Home
- ✅ Device information display

### Planned Features

- Camera interface (exposure control, image download)
- FilterWheel interface (3 filters: Dark, IR, LP)
- Focuser interface (absolute positioning 0-2600 steps)
- Pulse guiding
- Enhanced error handling
- State management improvements

## Known Limitations

Based on comprehensive API testing:

### Telescope
- No Alt/Az slewing (RA/Dec only)
- Site location read-only
- Guide rates read-only
- Track rates read-only

### Future Devices
- Camera: No binning, no cooling control
- Focuser: Absolute positioning only (no relative moves)
- FilterWheel: 3 filters fixed

See [AlpacaAPIComparison.md](../AlpacaAPIComparison.md) for complete API support details.

## Usage

### Starting the Driver

```bash
# Start INDI server with alpaca driver
indiserver indi_alpaca_telescope
```

### With INDI Control Panel

1. Start INDI server: `indiserver indi_alpaca_telescope`
2. Launch INDI Control Panel: `indi_control_panel`
3. Connect to localhost:7624
4. Configure connection settings (host/port)
5. Connect to device

### With KStars/Ekos

1. Open KStars
2. Go to Tools → Ekos
3. Add Profile
4. Select "alpaca" as telescope
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

### Cannot connect to alpaca

1. Verify alpaca is powered on and on network
2. Check hostname resolution: `ping alpaca.local`
3. Verify Alpaca API port: `curl http://alpaca.local:32323/api/v1/telescope/0/connected`
4. Check firewall settings

### Operations fail with "Invalid while parked"

The telescope must be unparked before most operations. Use the Unpark button.

### Find Home fails

Find Home cannot be executed while telescope is parked. Unpark first.

## Contributing

See [DEVELOPMENT_CHECKLIST.md](../DEVELOPMENT_CHECKLIST.md) for development roadmap.

## License

LGPL 2.1 or later

## Author

Gord Tulloch

## Acknowledgments

- INDI Library developers
- ASCOM Initiative for Alpaca API specification
- ZWO for alpaca hardware
- Testing data from comprehensive API validation

## Version History

### 1.0.0 (In Development)
- Initial telescope interface implementation
- Basic position, slewing, parking, tracking
- Manual motion control
- Find Home support
