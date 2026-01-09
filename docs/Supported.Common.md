# alpaca Alpaca Common Methods API Support

This document provides comprehensive test results for all ASCOM Alpaca Common Methods (methods available to all device types) on the alpaca v1.1.2-1 firmware.

**Tested Firmware Version:** v1.1.2-1  
**Alpaca Port:** 32323  
**Device Type:** telescope/0  
**Test Date:** January 2026

## ASCOM Common Methods - Test Status

These methods are defined in the ASCOM standard as being available for all device types (telescope, camera, focuser, etc.).

| Method | Type | Endpoint | Test Status | Notes |
|--------|------|----------|-------------|-------|
| **Connection Management** |
| connected | GET | /connected | ✅ Working | Returns current connection state |
| connected | PUT | /connected | ✅ Working | Set connection state (connect/disconnect) |
| connecting | GET | /connecting | ❌ Not Implemented | Error 1024 |
| connect | PUT | /connect | ⚪ Not Tested | Async connect method |
| disconnect | PUT | /disconnect | ❌ Not Implemented | Error 1024 (use connected=false) |
| **Device Information** |
| description | GET | /description | ✅ Working | Returns "alpaca S30_[id] Telescope" |
| driverinfo | GET | /driverinfo | ✅ Working | Returns "Telescope V3" |
| driverversion | GET | /driverversion | ✅ Working | Returns "1.1.2-1" |
| interfaceversion | GET | /interfaceversion | ✅ Working | Returns 3 (ITelescopeV3) |
| name | GET | /name | ✅ Working | Returns "alpaca S30_[id] Telescope" |
| devicestate | GET | /devicestate | ❌ Not Implemented | Error 1024 |
| **Command Methods** |
| action | PUT | /action | ❌ Not Working | Error 1036: action not implemented |
| supportedactions | GET | /supportedactions | ✅ Working | Returns empty array [] |
| commandblind | PUT | /commandblind | ❌ Not Implemented | Error 1024 |
| commandbool | PUT | /commandbool | ❌ Not Implemented | Error 1024 |
| commandstring | PUT | /commandstring | ❌ Not Implemented | Error 1024 |

## Legend

- ✅ **Working** - Tested and confirmed functional
- ❌ **Not Working** - Tested and returns error/not implemented
- ⚪ **Not Tested** - Not yet tested

## Summary

**GET Methods:** 6/8 working (75%)  
**PUT Methods:** 1/6 working (17%)

### Working Methods (7)
- **Connection:** connected (GET/PUT)
- **Information:** description, driverinfo, driverversion, interfaceversion, name
- **Actions:** supportedactions (returns empty array)

### Not Implemented (7)
- **Connection:** connecting (GET), disconnect (PUT)
- **State:** devicestate (GET)
- **Commands:** action, commandblind, commandbool, commandstring

## Key Findings

### Connection Management
- ✅ **connected GET/PUT works** - This is the primary method for connection management
- ❌ **connecting GET not implemented** - Cannot query async connection status
- ❌ **disconnect PUT not implemented** - Use `PUT /connected` with `Connected=false` instead

### Device Information
- All standard information queries work correctly
- Device identifies as "Telescope V3" (ASCOM ITelescopeV3 interface)
- Driver version correctly reports firmware version (1.1.2-1)
- ❌ **devicestate not implemented** - Cannot get full device state in single call

### Action/Command Methods
- ❌ **No custom actions supported** - `supportedactions` returns empty array
- ❌ **No command methods** - commandblind/commandbool/commandstring not implemented
- This confirms that alpaca only implements standard ASCOM methods, not custom extensions via Alpaca

## Connection Workflow

The proper connection workflow for alpaca is:

1. **Connect:**
   ```
   PUT /connected
   Body: Connected=true&ClientID=1&ClientTransactionID=1
   ```

2. **Verify Connection:**
   ```
   GET /connected
   Returns: {"Value": true, "ErrorNumber": 0, ...}
   ```

3. **Disconnect:**
   ```
   PUT /connected
   Body: Connected=false&ClientID=1&ClientTransactionID=2
   ```

Note: Do not use the `/disconnect` endpoint as it's not implemented.

## ASCOM Error Codes

- **0** - Success
- **1024** - Not implemented
- **1036** - Action not implemented (for /action endpoint)

## Comparison with ASCOM Standard

The alpaca implements a **subset** of the ASCOM Common Methods:
- ✅ Core connection management (connected GET/PUT)
- ✅ All device information queries
- ❌ Advanced connection methods (connecting, disconnect)
- ❌ Full device state query
- ❌ Custom action/command methods

This is sufficient for standard ASCOM driver operation, as the core methods provide all essential functionality.

## Test Programs

Test program used to validate this support:
- `common_methods_test.cpp` - Tests all ASCOM Common Methods

## Related Documentation

- [Supported Telescope Methods](Supported.Telescope.md) - Telescope-specific methods
- [ASCOM Alpaca API Documentation](https://ascom-standards.org/api/)
