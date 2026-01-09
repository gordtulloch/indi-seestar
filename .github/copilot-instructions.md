# ASCOM Alpaca API Reference for Copilot

This file contains reference information about the ASCOM Alpaca Device API to help with development of the INDI alpaca driver.

## alpaca Alpaca Firmware Version
**Current Version**: v1.1.2-1

## Official Documentation
- Main API: https://ascom-standards.org/api/#/ASCOM%20Methods%20Common%20To%20All%20Devices
- Interface Definitions: https://ascom-standards.org/Help/Developer/html/N_ASCOM_DeviceInterface.htm

## URL Format

Alpaca Device API URLs follow this pattern:
```
http(s)://host:port/api/v1/{device_type}/{device_number}/{method}
```

### Examples:
- Telescope Interface Version: `http://192.168.1.89:7843/api/v1/telescope/0/interfaceversion`
- Focuser Position: `http://192.168.1.89:7843/api/v1/focuser/0/position`
- Rotator Halt: `http://192.168.1.89:7843/api/v1/rotator/0/halt`

### Important Notes:
- **URLs are case sensitive** - all elements must be in **lowercase**
- Device type must be lowercase (e.g., "telescope", "focuser", "camera")
- Method names must be lowercase (e.g., "interfaceversion", "connected", "action")
- Parameter names are NOT case sensitive
- Parameter values can be mixed case

## HTTP Methods
- **GET operations**: Parameters go in the URL query string
- **PUT operations**: Parameters go in the request body (application/x-www-form-urlencoded)

## Response Format

All responses are JSON with this common structure:
```json
{
  "Value": <response_value>,
  "ClientTransactionID": <client_id>,
  "ServerTransactionID": <server_id>,
  "ErrorNumber": <error_code>,
  "ErrorMessage": "<error_message>"
}
```

### Success Indicators:
- `ErrorNumber`: 0 (zero means success)
- `ErrorMessage`: "" (empty string means success)

### HTTP Status Codes:
- **200**: Request was correctly formatted and passed to device handler
  - Note: You MUST still check ErrorNumber/ErrorMessage for actual success
- **400**: Invalid request (bad device number, misspelt device type, etc.)
- **500**: Unexpected error within device

## ASCOM Methods Common To All Devices

These methods are available for all device types (telescope, camera, focuser, etc.):

### Connection Management
- `PUT /{device_type}/{device_number}/connect` - Start asynchronous connect
- `PUT /{device_type}/{device_number}/disconnect` - Start asynchronous disconnect
- `GET /{device_type}/{device_number}/connected` - Get connected state
- `PUT /{device_type}/{device_number}/connected` - Set connected state
- `GET /{device_type}/{device_number}/connecting` - Check if connect/disconnect in progress

### Device Information
- `GET /{device_type}/{device_number}/description` - Device description
- `GET /{device_type}/{device_number}/driverinfo` - Driver description
- `GET /{device_type}/{device_number}/driverversion` - Driver version
- `GET /{device_type}/{device_number}/interfaceversion` - ASCOM interface version
- `GET /{device_type}/{device_number}/name` - Device name
- `GET /{device_type}/{device_number}/devicestate` - Full operational state in one call

### Command Methods
- `PUT /{device_type}/{device_number}/action` - Invoke device-specific action
  - Parameters: `Action=<action_name>&Parameters=<json_params>`
- `PUT /{device_type}/{device_number}/commandblind` - Send arbitrary string (no response)
- `PUT /{device_type}/{device_number}/commandbool` - Send string, get boolean response
- `PUT /{device_type}/{device_number}/commandstring` - Send string, get string response

### Supported Actions
- `GET /{device_type}/{device_number}/supportedactions` - List of supported actions

## PUT Request Parameters

All PUT requests should include these standard parameters:
- `ClientID=<integer>` - Client identifier
- `ClientTransactionID=<integer>` - Transaction ID from client

### Example PUT Request Body:
```
Connected=true&ClientID=1&ClientTransactionID=123
Action=play_sound&Parameters=&ClientID=1&ClientTransactionID=124
```

## Device-Specific Methods

Different device types have additional specific methods:
- **Camera**: Exposure controls, binning, gain, etc.
- **Telescope**: Slew, tracking, coordinates, parking, etc.
- **Focuser**: Position, movement, temperature compensation
- **Rotator**: Angle, rotation, sync
- **Dome**: Shutter, azimuth, slaving
- **FilterWheel**: Position, filter names
- **Switch**: Multi-purpose switches
- **ObservingConditions**: Weather data
- **CoverCalibrator**: Calibrator and cover control
- **SafetyMonitor**: Safety status

## Discovery Protocol

Devices can be discovered via UDP broadcast:
- **Port**: 32227
- **Message**: "alpacadiscovery1" (broadcast)
- **Response**: JSON with `{"AlpacaPort": <port_number>}`

## Management API

Separate from the Device API, the Management API provides:
- `/management/apiversions` - Supported API versions
- `/management/v1/description` - Device description
- `/management/v1/configureddevices` - List of devices

## Common alpaca Configuration

Based on testing:
- **Alpaca Port**: 32323 (not the standard 11111)
- **Device Type**: telescope
- **Device Number**: 0
- **Connection Required**: Must call `/connected` with `Connected=true` before other operations

## Example Workflow

1. **Discovery** (optional):
   ```
   UDP broadcast "alpacadiscovery1" to port 32227
   ```

2. **Connect**:
   ```
   PUT http://alpaca.local:32323/api/v1/telescope/0/connected
   Body: Connected=true&ClientID=1&ClientTransactionID=1
   ```

3. **Execute Commands**:
   ```
   PUT http://alpaca.local:32323/api/v1/telescope/0/action
   Body: Action=<action_name>&Parameters=&ClientID=1&ClientTransactionID=2
   ```

4. **Query Status**:
   ```
## alpaca Alpaca v1.1.2-1 Tested Features

The alpaca v1.1.2-1 firmware has excellent ASCOM Alpaca support. See the comprehensive test results table in [README.md](../README.md#ascom-alpaca-api-support).

### Key Findings from Testing

**Excellent Support:**
- 48 of 52 GET methods working (92% success rate)
- All capability queries (can*) functional
- Position, tracking, and status queries fully operational
- Site information (latitude/longitude) available
- Axis rates and guide rates accessible

**Known Limitations:**
- Alt/Az slewing not supported (canslewaltaz=false)
- Park command returns error despite canpark=true
- Some telescope properties not available (aperture, focal length, elevation)
- Side of pier not implemented

**Untested:**
- Most PUT methods for rates and site configuration
- Sync commands
- Pulse guiding commands
- Axis movement commands
- Target coordinate setting

For detailed test status of all methods, see the table in README.md.

## Notes for alpaca Development

- The alpaca Alpaca v1.1.2-1 implements limited ASCOM Alpaca API
- Always check `ErrorNumber` in responses - Error 0 means success
- Error codes: 1024 = not implemented, 1036 = action not implemented, 1279 = operational failure
- Park/Unpark commands require native API (different port/protocol)
- Native API commands (documented in Postman collection) are NOT accessible via Alpaca endpoint
- Telescope must be initialized (via app) before Alpaca commands work properly
- Use the `/action` endpoint with device-specific action names
- Always check `ErrorNumber` in responses - Error 1036 means action not implemented
- The alpaca may have custom actions beyond standard Alpaca - these need to be discovered through documentation or experimentation
