# GoTo Error 1279 Troubleshooting Guide

## Problem

When attempting to slew the telescope using the GoTo command, error 1279 may occur. This error indicates a **state-dependent operation failure** - the telescope is not in a valid state for the requested operation.

## Common Error Messages

### Error 1: Slew Operation Failed
```
[ERROR] Failed to start GoTo 
[ERROR] Alpaca error /slewtotarget: 1279 - SlewToCoordinatesAsync fail: fail to operate
```

### Error 2: Tracking Cannot Be Enabled
```
[ERROR] Failed to enable tracking. Cannot proceed with GoTo.
[ERROR] Alpaca error /tracking: 1279 - SET_SCOPE_SET_TRACK_STATE fail: mount sync failed
```

## Root Cause

Error 1279 is a **state-dependent error** that occurs when an operation is attempted in an invalid telescope state. According to [Seestar Alpaca API testing](../docs/Supported.Telescope.md):

- **Slew operations** require tracking to be enabled
- **Tracking** cannot be enabled if:
  - Telescope is parked
  - Telescope is at home position
  - Mount has not completed initialization
  - Site location is not configured

## Understanding Telescope States

### 1. Parked State
- Telescope is in a safe, stored position
- **Cannot track or slew while parked**
- Must unpark before any movement

### 2. At Home Position  
- Telescope is at a calibration/reference position
- **Tracking cannot be enabled at home**
- Telescope must move away from home before tracking

### 3. Unparked & Away from Home
- ✅ Valid state for enabling tracking
- ✅ Valid state for slewing operations

## Solutions

### Quick Fix Checklist

1. **Check Park Status**
   ```
   Is telescope parked? → Unpark it
   ```

2. **Check Home Position**
   ```
   Is telescope at home? → Move away from home or use native app to initialize
   ```

3. **Verify Site Location**
   ```
   Is location configured? → Set latitude/longitude in INDI client
   ```

4. **Check Initialization**
   ```
   Is device initialized? → May need initialization via native app first
   ```

### Detailed Steps

#### Step 1: Unpark the Telescope

If the telescope is parked, you'll see:
```
[ERROR] Cannot GoTo while parked. Please unpark the telescope first.
```

**Solution:** Use your INDI client's Park/Unpark controls to unpark.

#### Step 2: Move Away from Home Position

If tracking fails with "mount sync failed", the telescope may be at home:
```
[ERROR] Cannot enable tracking while at home position.
```

**Solutions:**
- Use manual slew controls to move the telescope
- Initialize the device using its native application
- The telescope automatically moves away from home during normal operation

#### Step 3: Verify Site Location

Some mounts require site location before tracking:
```
[ERROR] Site location not configured
```

**Solution:** Set your location in the INDI client:
1. Connect to the telescope
2. Find "Site Management" or "Location" properties
3. Enter your latitude and longitude
4. Save the configuration

#### Step 4: Device Initialization

Some devices (like Seestar S50) may require initialization through their native app:

**For Seestar S50:**
1. Connect via Seestar app
2. Complete the startup sequence
3. Device will be ready for INDI control
4. You can then disconnect the app and use INDI

## Implementation Fix

The driver now includes comprehensive state checking and helpful error messages:

### 1. Pre-Flight Checks

```cpp
// Check if parked
if (isParked) {
    LOG_ERROR("Cannot GoTo while parked. Please unpark the telescope first.");
    return false;
}

// Check if at home
if (atHome) {
    LOG_WARN("Telescope is at home position. This may prevent tracking.");
}
```

### 2. Tracking Verification with Diagnostics

```cpp
if (!isTracking) {
    // Try to enable tracking
    if (!enableTracking()) {
        LOG_ERROR("Failed to enable tracking. Cannot proceed with GoTo.");
        LOG_ERROR("This typically means the telescope is not in a valid state.");
        LOG_ERROR("Check that:");
        LOG_ERROR("  1. Telescope is not parked");
        LOG_ERROR("  2. Telescope is not at home position");
        LOG_ERROR("  3. Site location is properly configured");
        LOG_ERROR("  4. Device has completed initialization");
        return false;
    }
    
    // Verify it actually enabled
    if (!verifyTracking()) {
        LOG_ERROR("Tracking not enabled after request.");
        return false;
    }
}
```

### 3. Better API Usage

Changed from multi-step process to direct slewing:
```cpp
// Use slewtocoordinatesasync - more reliable, single atomic operation
slewRequest["RightAscension"] = ra;
slewRequest["Declination"] = dec;
sendAlpacaPUT("/slewtocoordinatesasync", slewRequest, response);
```

## Testing the Fix

After rebuilding:

```bash
cd /home/stellarmate/Projects/indi-seestar/indi-alpaca/build
cmake --build . --target indi_alpaca_telescope
```

### Test Procedure

1. **Start the driver**:
   ```bash
   indiserver -v ./indi_alpaca_telescope
   ```

2. **Monitor status** (in another terminal):
   ```bash
   cd /home/stellarmate/Projects/indi-seestar/alpaca-tests
   ./indi_telescope_monitor.py
   ```

3. **Connect via INDI client** (KStars, etc.)

4. **Try different scenarios:**
   - Connect when telescope is parked → Should get clear "unpark first" message
   - Unpark, then try GoTo → Should check tracking and enable if needed
   - If at home → Should get diagnostic message about moving away from home

**Expected Output:**
- Clear error messages indicating what's wrong
- Helpful suggestions for fixing the issue
- Automatic tracking enablement when possible
- Successful slew when all prerequisites are met

## Error Message Reference

| Error Message | Meaning | Solution |
|--------------|---------|----------|
| "Cannot GoTo while parked" | Telescope is in park position | Unpark telescope |
| "Cannot enable tracking while at home position" | Mount is at home/reference position | Move away from home or initialize device |
| "Tracking not enabled after request" | Tracking enable command failed | Check device state and initialization |
| "mount sync failed" | Internal mount error | Device needs initialization or is in invalid state |
| "Site location not configured" | Location data missing | Configure latitude/longitude |

## ASCOM Error Codes

| Code | Name | Meaning | Common Causes |
|------|------|---------|---------------|
| 0 | Success | Operation completed | N/A |
| 1024 | Not Implemented | Feature not supported | Device limitation |
| 1026 | Value Not Set | Required value missing | Target not set before slew |
| 1279 | Invalid Operation | Wrong state for operation | Parked, at home, not initialized, tracking off |

## Related Files

- [alpaca_telescope.cpp](../indi-alpaca/alpaca_telescope.cpp) - Main implementation with fixes
- [Supported.Telescope.md](../docs/Supported.Telescope.md) - API testing documentation
- [copilot-instructions.md](../.github/copilot-instructions.md) - ASCOM Alpaca API reference
- [TELESCOPE_MONITOR.md](../alpaca-tests/TELESCOPE_MONITOR.md) - Status monitoring tool

## FAQ

**Q: Why can't I slew while parked?**  
A: This is a safety feature. The telescope must be unparked to allow movement.

**Q: What does "at home position" mean?**  
A: Home is a reference/calibration position. Some mounts require moving away from home before tracking can start.

**Q: Do I need to use the native app?**  
A: For initial device setup and initialization, yes. After that, INDI can control the device.

**Q: Can I slew without tracking enabled?**  
A: No. Most mounts require tracking to be enabled for slewing because the target needs to be tracked after the slew completes.

**Q: The errors are confusing - which do I fix first?**  
A: Follow this order:
   1. Unpark (if parked)
   2. Move away from home (if at home)
   3. Configure location (if needed)
   4. Initialize device (if needed)
   5. Try GoTo again

## Getting Help

If you still encounter issues after following this guide:

1. Check the driver logs for detailed error messages
2. Use the telescope monitor to see real-time status
3. Verify your device is accessible on the network
4. Try the native app to ensure the device is functional
5. Report issues with full log output and device model

---

## Park Operation Issues

### Problem: Park Never Completes

**Symptom:** Park operation starts but never completes, even though telescope is physically at park position.

**Root Cause:** The original parking completion detection only checked if slewing stopped, but didn't verify the telescope actually reached park position according to the Alpaca API.

**Fix:** Enhanced `ReadScopeStatus()` to properly detect park completion:

```cpp
if (TrackState == SCOPE_PARKING)
{
    // Parking is complete when:
    // 1. Telescope has stopped slewing AND
    // 2. Alpaca reports telescope is at park position
    if (!isSlewing && isParked)
    {
        TrackState = SCOPE_PARKED;
        SetParked(true);
        LOG_INFO("Parking complete - telescope at park position");
    }
}
```

**Also Fixed:** `Park()` now checks if already at park position before issuing park command:

```cpp
// Check if already parked
if (alreadyParked)
{
    LOG_INFO("Telescope is already at park position");
    isParked = true;
    SetParked(true);
    TrackState = SCOPE_PARKED;
    return true;
}
```

This prevents unnecessary park operations and immediately reflects the correct state.
