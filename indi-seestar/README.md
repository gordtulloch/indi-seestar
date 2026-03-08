# INDI Seestar Drivers

Seestar-specific INDI drivers that inherit from the generic INDI Alpaca drivers and add Seestar S50-specific functionality.

## Architecture

These drivers use inheritance from the base `indi-alpaca` drivers to provide:
- **Base Alpaca Protocol**: All standard ASCOM Alpaca telescope operations
- **Seestar Extensions**: Seestar-specific features like live stacking, auto focus, plate solving

```
alpacaTelescopeDriver (indi-alpaca)
    ↓ inherits
SeestarTelescopeDriver (indi-seestar) + Seestar-specific features
```

## Features

### Seestar Telescope Driver

Inherits all standard telescope operations from `alpacaTelescopeDriver`:
- RA/Dec positioning and tracking
- Slewing and goto
- Park/Unpark
- Site location

Plus Seestar-specific features:
- **Operation Modes**: View, GoTo, Live Stack, Auto Focus
- **Live Stacking**: Start/stop with exposure, gain, and count settings
- **Auto Focus**: Automated focus routine
- **Plate Solving**: Solve current image for precise positioning

## Building

```bash
cd indi-seestar
mkdir -p build && cd build
cmake ..
make -j$(nproc)
sudo make install
```

## Usage

The Seestar driver appears in INDI as "Seestar S50" under Telescopes.

### Connection

1. Set hostname to your Seestar (default: alpaca.local or Seestar's IP)
2. Set port to 32323 (Alpaca default)
3. Set device number to 0
4. Connect

### Seestar-Specific Controls

**Operation Mode** (Main Control tab):
- View: Real-time viewing mode
- GoTo: Slew to target coordinates
- Live Stack: Start live stacking session
- Auto Focus: Run auto focus routine

**Live Stacking** (Main Control tab):
- Start/Stop buttons
- Settings in Options tab:
  - Exposure time (0.1-60s)
  - Gain (0-200)
  - Target frame count

**Auto Focus** (Main Control tab):
- Single button to start auto focus
- Monitor status via property state

**Plate Solve** (Main Control tab):
- Solve current image to sync mount position

## Implementation Details

The Seestar driver uses the Alpaca `/action` endpoint to send Seestar-specific commands. These custom actions are implemented in the Seestar's Alpaca server firmware.

### Code Structure

- `indi_seestar_telescope.h` - Class definition inheriting from alpacaTelescopeDriver
- `indi_seestar_telescope.cpp` - Implementation with Seestar extensions
- `CMakeLists.txt` - Build configuration linking alpaca base library
- `indi_seestar.xml` - INDI driver registration

### Helper Method

```cpp
bool sendSeestarAction(const std::string& actionName, const std::string& parameters = "");
```

Uses the protected `sendAlpacaPUT()` method from the base class to send custom Alpaca actions.

## Adding More Seestar Drivers

To add CCD, Focuser, or FilterWheel drivers:

1. Create `indi_seestar_XXX.h` inheriting from `alpacaXXXDriver`
2. Add Seestar-specific properties and methods
3. Update `CMakeLists.txt` to build with `alpaca_XXX_base.cpp`
4. Update `indi_seestar.xml` with new device entry

## Dependencies

- INDI library 2.0+
- indi-alpaca drivers (base classes)
- httplib (via alpaca drivers)
- nlohmann/json (via alpaca drivers)

## Notes

- The base `indi-alpaca` drivers remain fully functional for any Alpaca-compatible device
- Seestar drivers use a library version of alpaca code to avoid symbol conflicts
- The `INDI_ALPACA_BASE_LIBRARY` define enables library compilation mode
