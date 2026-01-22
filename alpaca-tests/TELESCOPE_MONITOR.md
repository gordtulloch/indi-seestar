# INDI Telescope Monitor

A standalone test client to monitor INDI telescope driver status in real-time.

## Purpose

This tool connects to an INDI telescope driver (via indiserver) and displays real-time status updates. It's useful for:

- Verifying that `ReadScopeStatus()` is updating INDI clients correctly
- Debugging telescope position updates
- Monitoring telescope state changes (slewing, tracking, parked, etc.)
- Testing INDI telescope driver implementations

## About ReadScopeStatus()

According to the [INDI telescope interface documentation](https://docs.indilib.org/interfaces/telescope-interface.html#key-concepts-for-telescope-driver-development), the `ReadScopeStatus()` method is critical for maintaining accurate position reporting:

> The `ReadScopeStatus()` method is called periodically (typically once per second) and is responsible for:
> - Reading current encoder positions from the mount
> - Converting mount-specific coordinates (Alt/Az or RA/Dec encoders) to equatorial coordinates
> - **Calling `NewRaDec(ra, dec)` to notify clients of the current position**
> - Checking if slewing or parking operations are complete

The alpaca telescope driver implements this in [alpaca_telescope.cpp](../indi-alpaca/alpaca_telescope.cpp):
- Queries the Alpaca API for current RA/Dec coordinates
- Updates internal state tracking (slewing, tracking, parked)
- Calls `NewRaDec()` to push updates to INDI clients
- Updates `TrackState` based on telescope status

## Usage

### Python Version (Recommended)

The Python version is simpler and doesn't require compilation:

```bash
# Basic usage (connects to localhost:7624, device: alpaca_telescope)
./indi_telescope_monitor.py

# Connect to remote server
./indi_telescope_monitor.py --host 192.168.1.100 --port 7624

# Monitor different device
./indi_telescope_monitor.py --device "Telescope Simulator"

# Show help
./indi_telescope_monitor.py --help
```

### Requirements

Python 3 with standard library (no additional packages needed).

## Example Output

```
==================================================
  INDI Telescope Monitor
==================================================
Connecting to: localhost:7624
Telescope:     alpaca_telescope
Press Ctrl+C to exit
==================================================

Connected to INDI server at localhost:7624

=== TELESCOPE CONNECTED ===

==================================================
  TELESCOPE STATUS MONITOR
==================================================

📍 COORDINATES:
  Current Position:
    RA  = 05h 34m 31.20s  (5.575333 hours = 83.630000°)
    Dec = +22° 00' 50.40"  (22.014000°)
    Alt = 45.23°
    Az  = 180.50°

⚙️  STATE:
  Status: TRACKING

🎯 TRACKING:
  Enabled: YES
  Mode:    TRACK_SIDEREAL

🅿️  PARK:
  Parked:  NO
==================================================
```

## What It Monitors

The monitor displays real-time updates for:

- **Coordinates**: Current RA/Dec (in HMS/DMS and decimal), Alt/Az
- **State**: Current telescope state (IDLE, TRACKING, SLEWING, PARKED, etc.)
- **Tracking**: Whether tracking is enabled and the tracking mode (Sidereal, Lunar, Solar)
- **Park Status**: Whether the telescope is parked

Updates appear whenever properties change on the server side.

## Testing ReadScopeStatus()

To verify `ReadScopeStatus()` is working:

1. Start indiserver with the alpaca telescope driver:
   ```bash
   indiserver -v indi_alpaca_telescope
   ```

2. In another terminal, run the monitor:
   ```bash
   ./indi_telescope_monitor.py
   ```

3. Connect to the telescope using KStars or another INDI client

4. Watch the monitor output - you should see:
   - Position updates as the telescope tracks
   - State changes when you command slews
   - Tracking status changes
   - Park/unpark operations

If you don't see position updates while tracking is enabled, it indicates `ReadScopeStatus()` may not be calling `NewRaDec()` properly.

## Implementation Notes

The Python monitor:
- Connects to indiserver using raw TCP socket
- Parses INDI XML protocol messages
- Monitors these INDI properties:
  - `EQUATORIAL_EOD_COORD` - Current RA/Dec position
  - `TARGET_EOD_COORD` - Target coordinates
  - `HORIZONTAL_COORD` - Alt/Az coordinates
  - `TELESCOPE_TRACK_STATE` - Tracking on/off
  - `TELESCOPE_TRACK_MODE` - Tracking mode
  - `TELESCOPE_PARK` - Park status
  - `CONNECTION` - Connection status

## Troubleshooting

**"Connection refused"**: Make sure indiserver is running:
```bash
ps aux | grep indiserver
```

**No updates appearing**: 
1. Check that telescope is connected (look for "TELESCOPE CONNECTED" message)
2. Verify device name matches (use `--device` option)
3. Check indiserver logs for errors

**Position not updating**:
- This likely indicates `ReadScopeStatus()` isn't being called or isn't calling `NewRaDec()`
- Check that the driver's timer is running (`SetTimer(POLLMS)` in Connect())
- Verify `ReadScopeStatus()` is calling `NewRaDec(ra, dec)` when coordinates change

## Related Files

- [alpaca_telescope.cpp](../indi-alpaca/alpaca_telescope.cpp) - Main telescope driver implementation
- [alpaca_telescope.h](../indi-alpaca/alpaca_telescope.h) - Header file with ReadScopeStatus() declaration
- [INDI Telescope Interface Docs](https://docs.indilib.org/interfaces/telescope-interface.html)
