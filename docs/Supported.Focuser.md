# alpaca Focuser Specific Methods - Test Results

This document contains detailed test results for all ASCOM Alpaca Focuser Specific Methods tested against alpaca firmware **v1.1.2-1**.

**Test Date**: January 2, 2026  
**Test Device**: alpaca at alpaca.local:32323  
**Device Path**: `/api/v1/focuser/0/`

## Summary

- **Total GET Methods Tested**: 9
- **Working GET Methods**: 8 (89%)
- **Not Implemented**: 1 (stepsize)
- **Total PUT Methods Tested**: 4
- **Working PUT Methods**: 2 (50%)
- **Not Implemented**: 2 (tempcomp setting)

## Key Findings

### ✅ Excellent Support:
- **Absolute positioning** - Full support for absolute moves
- **Position range**: 0-2600 steps
- **Temperature monitoring**: 27.06°C in test
- **Movement control**: Move and halt commands working
- **Status monitoring**: IsMoving flag available

### ❌ Not Supported:
- **Relative moves** - Error 1025 (invalid value)
- **Step size** - Not implemented (error 1024)
- **Temperature compensation control** - Read-only, cannot enable/disable

### ⚠️ Important Notes:
- Only absolute positioning supported (no relative moves)
- Must use absolute position values (0-2600)
- Temperature compensation available flag is false
- Temperature can be read but not used for compensation

### 📊 Focuser Configuration:
- **Range**: 0-2600 steps
- **Current Position**: 1314 (at test time)
- **Absolute Positioning**: Yes
- **Maximum Increment**: 2600 (full range in one move)
- **Temperature**: 27.0625°C (read-only)
- **Temperature Compensation**: Not available

## Complete Test Results

### Focuser Capabilities

| Endpoint | Method | Status | Test Value | Notes |
|----------|--------|--------|------------|-------|
| `/absolute` | GET | ✅ Working | true | Supports absolute positioning |
| `/tempcompavailable` | GET | ✅ Working | false | No temp compensation |

### Focuser Properties

| Endpoint | Method | Status | Test Value | Notes |
|----------|--------|--------|------------|-------|
| `/maxstep` | GET | ✅ Working | 2600 | Maximum position value |
| `/maxincrement` | GET | ✅ Working | 2600 | Maximum move in one command |
| `/stepsize` | GET | ❌ Not Implemented | - | Error 1024 |
| `/position` | GET | ✅ Working | 1314 | Current absolute position |

**Position Range**: 0 to 2600 steps

### Temperature

| Endpoint | Method | Status | Test Value | Notes |
|----------|--------|--------|------------|-------|
| `/temperature` | GET | ✅ Working | 27.0625 | Current temperature (°C) |
| `/tempcomp` | GET | ✅ Working | false | Temp compensation state |

**Note**: Temperature is read-only, no active compensation control available

### Movement State

| Endpoint | Method | Status | Test Value | Notes |
|----------|--------|--------|------------|-------|
| `/ismoving` | GET | ✅ Working | false | Movement status flag |

### Focuser PUT Methods

| Endpoint | Method | Status | Test Parameters | Notes |
|----------|--------|--------|-----------------|-------|
| `/move` | PUT | ❌ Error 1025 | Position=-10 | Relative move not supported |
| `/move` | PUT | ✅ Working | Position=1364 | Absolute move successful |
| `/halt` | PUT | ✅ Working | (no params) | Stop movement command |
| `/tempcomp` | PUT | ❌ Not Implemented | TempComp=true/false | Error 1024 |

**Critical**: The `/move` endpoint only accepts absolute positions (0-2600), not relative movements.

## Error Code 1025 - Invalid Value

When attempting a relative move (negative or beyond maxstep), the focuser returns error 1025:
- **Error 1025**: Invalid Value
- **Cause**: Attempted relative move with Position=-10
- **Solution**: Always use absolute position values (0-2600)

## Movement Behavior

### Absolute Positioning
The alpaca focuser uses **absolute positioning only**:
- Valid range: 0 to 2600
- Position 0 = fully retracted (closest focus)
- Position 2600 = fully extended (farthest focus)
- Current position can be queried via `/position`

### Movement Commands
To move the focuser:
1. Query current position: `GET /position`
2. Calculate target absolute position
3. Send move command: `PUT /move` with `Position=<target>`
4. Poll `/ismoving` to check completion
5. Use `/halt` to stop movement if needed

### No Relative Movement Support
The focuser does **not** support relative moves:
- Cannot use negative values
- Cannot use offsets from current position
- Must always specify absolute target position
- Driver must calculate absolute positions from relative requests

## Implementation Notes

### For INDI Driver Development:

1. **Position Control**:
   - Range: 0-2600 steps
   - Use absolute positioning only
   - Query current position before relative moves
   - Calculate: new_pos = current_pos + relative_offset
   - Validate: 0 <= new_pos <= 2600

2. **Movement Commands**:
   - Use `/move` PUT with absolute Position parameter
   - Poll `/ismoving` GET for status (or use timer)
   - Implement timeout for movement completion
   - Use `/halt` PUT to abort movement

3. **Temperature Monitoring**:
   - Query `/temperature` for display
   - No compensation available (tempcompavailable=false)
   - Can log temperature for diagnostics
   - No SET operation supported

4. **Status Polling**:
   - `/ismoving` returns boolean
   - Poll at reasonable interval (0.5-1 second)
   - Don't send new move while ismoving=true
   - Handle movement completion properly

5. **Error Handling**:
   - Validate position range (0-2600) before sending
   - Handle error 1025 (invalid value)
   - Return appropriate INDI error codes
   - Don't attempt relative moves

6. **Step Size**:
   - Not available from API (error 1024)
   - Cannot calculate physical movement per step
   - Use steps as unitless values
   - May need calibration routine

7. **No Temperature Compensation**:
   - Don't expose temp comp controls in UI
   - tempcompavailable returns false
   - PUT /tempcomp returns error 1024
   - Temperature is informational only

## Usage Example

### Query Current Position
```bash
curl "http://alpaca.local:32323/api/v1/focuser/0/position"
```
Response: `{"Value": 1314, "ErrorNumber": 0, ...}`

### Move to Absolute Position
```bash
curl -X PUT "http://alpaca.local:32323/api/v1/focuser/0/move" \
  -d "Position=1400&ClientID=1&ClientTransactionID=1"
```
Response: `{"Value": null, "ErrorNumber": 0, ...}`

### Check If Moving
```bash
curl "http://alpaca.local:32323/api/v1/focuser/0/ismoving"
```
Response: `{"Value": false, "ErrorNumber": 0, ...}`

### Halt Movement
```bash
curl -X PUT "http://alpaca.local:32323/api/v1/focuser/0/halt" \
  -d "ClientID=1&ClientTransactionID=2"
```
Response: `{"Value": null, "ErrorNumber": 0, ...}`

### Read Temperature
```bash
curl "http://alpaca.local:32323/api/v1/focuser/0/temperature"
```
Response: `{"Value": 27.0625, "ErrorNumber": 0, ...}`

## Relative Move Implementation

Since the alpaca focuser only supports absolute positioning, the INDI driver must implement relative moves by:

```cpp
// Pseudocode for relative move
int current_position = get_position();  // Query current position
int relative_steps = requested_offset;  // From INDI client
int target_position = current_position + relative_steps;

// Validate range
if (target_position < 0) target_position = 0;
if (target_position > 2600) target_position = 2600;

// Execute absolute move
move_absolute(target_position);
```

## Temperature Monitoring

The focuser provides temperature data that could be useful for:
- Environmental logging
- Manual focus adjustment decisions
- Display in monitoring UI
- Correlation with focus drift

However, automatic temperature compensation is **not available**:
- `tempcompavailable` returns false
- Cannot enable/disable compensation
- No automatic focus adjustment
- Must be implemented externally if needed

## Performance Characteristics

Based on testing:
- **Movement**: Appears to be relatively fast (no long delays observed)
- **Position Update**: Immediate after command completion
- **IsMoving Flag**: Updates appropriately
- **Halt Response**: Immediate stop when commanded

## Comparison with ASCOM Standard

| Feature | ASCOM Standard | alpaca Implementation |
|---------|---------------|------------------------|
| Absolute positioning | Optional | ✅ Supported |
| Relative positioning | Common | ❌ Not supported (error 1025) |
| Step size | Optional | ❌ Not implemented |
| Temperature reading | Optional | ✅ Supported |
| Temp compensation | Optional | ❌ Not available |
| IsMoving status | Required | ✅ Supported |
| Halt command | Required | ✅ Supported |

## Related Documentation

- [Supported.Telescope.md](Supported.Telescope.md) - Telescope methods test results
- [Supported.Camera.md](Supported.Camera.md) - Camera methods test results
- [Supported.FilterWheel.md](Supported.FilterWheel.md) - FilterWheel methods test results
- [Supported.Common.md](Supported.Common.md) - Common ASCOM methods test results
- [README.md](README.md) - Main project documentation

## Testing Information

**Test Program**: `focuser_methods_test.cpp`  
**Build Command**: `cmake .. && make focuser_methods_test`  
**Run Command**: `./focuser_methods_test`

The test program connects to the focuser device, queries all GET endpoints, and tests movement commands including absolute move, relative move (fails), and halt.

## Conclusion

The alpaca Focuser implementation is **good** with 89% of GET methods and 50% of PUT methods working. The key limitation is the **absolute-only positioning** - no relative moves are supported. This is a design choice that requires the INDI driver to manage relative positioning calculations. The 0-2600 step range provides adequate precision for focusing operations, and the halt command allows for movement interruption.
