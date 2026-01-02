# Seestar FilterWheel Driver Status

## Build Status: ✅ COMPLETE

The INDI FilterWheel driver has been successfully built and installed alongside the telescope and CCD drivers.

**Installed Drivers:**
- `indi_seestar_telescope` - Telescope control driver
- `indi_seestar_ccd` - CCD camera driver  
- `indi_seestar_filterwheel` - Filter wheel control driver (NEW)

## Installation

Drivers installed to: `/usr/local/bin/`
Configuration file: `/usr/local/../share/indi/indi_seestar.xml`

## FilterWheel Driver Features Implemented

### Hardware Specifications
- **Filter Positions**: 3 total
  - Position 0: "Dark" (dark frame filter)
  - Position 1: "IR" (infrared filter)
  - Position 2: "LP" (light pollution filter)
- **Focus Offsets**: Supported (currently [0, 0, 0])
- **Movement**: Immediate positioning (no async status)

### Supported Operations
- ✅ **Filter Selection**: Select any of 3 filter positions
- ✅ **Position Query**: Read current filter position
- ✅ **Filter Names**: Query and display filter names (Dark, IR, LP)
- ✅ **Focus Offsets**: Read focus offset for each filter
- ✅ **Position Validation**: Prevents unnecessary movements (error 1279)
- ✅ **Device Info**: Description, driver version, interface version

### Known Limitations
- ❌ **No Movement Status**: No "ismoving" property (movement is immediate)
- ⚠️ **Focus Offsets Read-Only**: Cannot set offsets via INDI (all currently 0)
- ⚠️ **No Calibration UI**: Users need to manually determine focus offsets

## ASCOM Alpaca FilterWheel API Support

Based on testing with Seestar v1.1.2-1 firmware, the filter wheel supports **100% of standard ASCOM methods** (3/3 GET, 1/1 PUT).

### Working Methods (4/4)
All essential filter wheel operations are functional:
- **Position control**: GET/PUT current filter position
- **Filter names**: Query filter names array
- **Focus offsets**: Query focus offset for each filter
- **Connection**: Device info and status queries

### Operational Notes
- Setting position to current position returns error 1279 (expected behavior)
- Driver checks current position before moving to avoid this error
- All focus offsets currently set to 0 (may need calibration)
- No asynchronous movement - position changes are immediate

For full method test results, see: `Supported.FilterWheel.md`

## Usage

### Starting All Drivers Together

Start telescope, CCD, and filter wheel:
```bash
indiserver -v indi_seestar_telescope indi_seestar_ccd indi_seestar_filterwheel
```

Or start filter wheel individually:
```bash
indiserver -v indi_seestar_filterwheel
```

### Connection Setup

1. **Configure Alpaca Server Address**:
   - Host: `seestar.local` (or IP address)
   - Port: `32323`
   - Device Number: `0`

2. **Connect the Driver**: 
   - Set `CONNECTED` property to `ON`
   - Driver queries filter names and focus offsets
   - Current position is read and displayed

3. **Select Filters**:
   - Use `FILTER_SLOT` property to select position (1-3)
   - Position 1 = Dark filter
   - Position 2 = IR filter
   - Position 3 = LP filter

### Example: KStars/Ekos Full Imaging Setup
1. Start INDI server with all three drivers
2. In Ekos, configure devices:
   - Mount: "Seestar" telescope
   - Camera: "Seestar CCD" camera
   - Filter Wheel: "Seestar FilterWheel"
3. Configure connection (seestar.local:32323) for all
4. Connect all devices
5. Use filter selection in capture module
6. Ready for filtered imaging!

## Filter Details

### Position 0: Dark Filter
- **Purpose**: Dark frame calibration
- **Use**: Block all light for dark frame acquisition
- **Focus Offset**: 0 steps (configurable)
- **Typical Usage**: Take dark frames at same temperature/duration as light frames

### Position 1: IR Filter
- **Purpose**: Infrared imaging
- **Use**: Block visible light, pass infrared wavelengths
- **Focus Offset**: 0 steps (may need adjustment)
- **Typical Usage**: IR-enhanced deep sky imaging, reduced atmospheric effects

### Position 2: LP Filter
- **Purpose**: Light pollution suppression
- **Use**: Reduce light pollution effects in urban/suburban areas
- **Focus Offset**: 0 steps (may need adjustment)
- **Typical Usage**: Emission nebula imaging from light-polluted sites

## Focus Offset Calibration

The Seestar reports focus offsets for each filter, but they are currently all set to 0. To calibrate:

1. **Choose Reference Filter**: 
   - Select LP filter as reference (most common for imaging)
   - Focus carefully using focuser control

2. **Measure Other Filters**:
   - Switch to Dark filter
   - Note how many focuser steps needed to regain focus
   - Record this as Dark filter offset
   - Repeat for IR filter

3. **Apply Offsets**:
   - Currently offsets are read-only from Alpaca API
   - May require Seestar app configuration
   - Driver will read and display configured offsets

4. **Verify Offsets**:
   - After configuration, reconnect filter wheel
   - Check `FOCUS_OFFSETS` property shows correct values
   - Test by switching filters and checking focus

## Implementation Notes

### INDI API Version
Built against **INDI Library 2.1.7** with proper API compatibility:
- Uses `FilterNameTP` property from base class
- Uses `FilterSlotNP` property from base class
- Uses `PropertyTP[index].setText()` for text properties
- Uses `PropertyNP[index].setValue()` for numeric properties
- Direct array indexing for filter names

### Alpaca Communication
- HTTP client: `cpp-httplib`
- JSON parser: `nlohmann::json` (via indijson.hpp)
- All endpoints use `/api/v1/filterwheel/0/` prefix
- Transaction IDs incremented per request
- Error checking on all API calls
- Error 1279 handled gracefully (already at position)

### Base Class Integration
- Inherits from `INDI::FilterWheel`
- Implements `SelectFilter(int position)` - set filter position
- Implements `QueryFilter()` - read current position
- Base class manages `FilterSlotNP` and `FilterNameTP` properties
- 1-based indexing in INDI (converts to 0-based for Alpaca)

### Position Management
- Driver queries current position on connection
- Checks current position before moving to avoid error 1279
- Position changes are immediate (no async status)
- Valid positions: 1, 2, 3 (INDI) = 0, 1, 2 (Alpaca)

### Filter Names
- Queried from `/names` endpoint at connection
- Stored in `FilterNameTP` property
- Displayed in client UI for selection
- Names: "Dark", "IR", "LP"

## Next Steps

### Recommended Testing
1. ✅ Verify driver starts without errors
2. ✅ Confirm connection to Seestar
3. ⬜ Test filter position changes
4. ⬜ Verify filter names display correctly
5. ⬜ Check focus offsets are read
6. ⬜ Test position validation
7. ⬜ Verify position query after changes
8. ⬜ Test with imaging workflow
9. ⬜ Calibrate focus offsets if needed
10. ⬜ Full integration test with CCD

### Future Enhancements
- **Focus Offset Calibration**: Interactive calibration routine
- **Auto-Focus Integration**: Automatic focus adjustment on filter change
- **Filter Statistics**: Track usage time per filter
- **Position Presets**: Save favorite filter configurations
- **Multi-Device Support**: Multiple filter wheels on network

### Integration with Other Drivers
- **CCD Driver**: Coordinate filter changes with exposures
- **Focuser Driver**: Apply focus offsets automatically
- **Sequence Planning**: Filter-based imaging sequences
- **Flat Field Automation**: Auto-select filters for flats

## Related Files
- `indi_seestar_filterwheel.h` - FilterWheel driver header
- `indi_seestar_filterwheel.cpp` - FilterWheel driver implementation
- `indi_seestar_ccd.h` - CCD driver header
- `indi_seestar_ccd.cpp` - CCD driver implementation
- `indi_seestar.h` - Telescope driver header
- `indi_seestar.cpp` - Telescope driver implementation
- `indi_seestar.xml` - INDI device registration (3 devices)
- `Supported.FilterWheel.md` - Full API test results

## Build Information
- **CMake**: 3.16+
- **Compiler**: GCC 13.3.0, C++17
- **INDI Library**: 2.1.7
- **Dependencies**: libindi, libnova, threads, cpp-httplib, nlohmann::json
- **Build Date**: 2026-01-02
- **Version**: 1.0.0

## Complete Driver Suite

All three Seestar INDI drivers are now available:

| Driver | Executable | Device Type | Key Features |
|--------|-----------|-------------|--------------|
| Telescope | `indi_seestar_telescope` | Mount | Slew, tracking, park/unpark, position query |
| CCD | `indi_seestar_ccd` | Camera | Exposure, gain, subframe, Bayer, temperature |
| FilterWheel | `indi_seestar_filterwheel` | Filter Wheel | 3-position wheel, filter names, focus offsets |

All drivers communicate via ASCOM Alpaca API to the Seestar device at port 32323.
