# INDI Seestar Test Programs

This directory contains test programs to validate concepts and functionality before implementing the full INDI driver for the Seestar telescope.

## Test Programs

### 1. alpaca_discovery_test

Discovers ASCOM Alpaca devices on the local network via UDP broadcast.

**Purpose:**
- Sends UDP broadcast to port 32227 (Alpaca discovery port)
- Listens for responses from Alpaca-compatible devices
- Displays discovered device IP addresses and Alpaca API ports
- No prior knowledge of device address required

**Building:**
```bash
cd tests/build
cmake ..
make
```

**Usage:**
```bash
# Use default timeout (5 seconds)
./alpaca_discovery_test

# Specify custom timeout in seconds
./alpaca_discovery_test 10
```

**Expected Output:**
The program will display:
- Broadcast confirmation
- IP address of each discovered device
- Alpaca API port from device response
- Raw response data (typically JSON)

### 2. play_sound_test

Sends a play_sound action command to make the Seestar beep.

**Purpose:**
- Tests the action endpoint of the Seestar API
- Sends play_sound command using multiple methods (POST, PUT, native API)
- Validates command execution and response handling
- Provides audible confirmation of successful API communication

**Usage:**
```bash
# Using default hostname (seestar.local) and port (4700)
./play_sound_test

# Specify custom hostname
./play_sound_test 192.168.1.100

# Specify custom hostname and port
./play_sound_test seestar.local 32323
```

**Expected Output:**
The program will:
- Try multiple API endpoint patterns
- Display HTTP response codes and data
- You should hear a beep from the Seestar if successful

### 3. alpaca_version_test

Tests basic connectivity to the Seestar telescope via ASCOM Alpaca REST API.

**Purpose:**
- Connects to the Seestar at `seestar.local` (or specified hostname)
- Retrieves the management API versions
- Retrieves the telescope interface version
- Validates HTTP communication with the Alpaca API

**Building:**
```bash
cd tests
mkdir build
cd build
cmake ..
make
```

**Requirements:**
- libcurl development files (`libcurl4-openssl-dev` on Debian/Ubuntu)

**Usage:**
```bash
# Using default hostname (seestar.local) and port (11111)
./alpaca_version_test

# Specify custom hostname
./alpaca_version_test 192.168.1.100

# Specify custom hostname and port
./alpaca_version_test seestar.local 11111
```

**Expected Output:**
The program will display:
- HTTP response codes
- API version information in JSON format
- Connection status

## ASCOM Alpaca API Reference

The Seestar telescope implements the ASCOM Alpaca REST API. Key endpoints:

- **Management API Versions:** `GET /management/apiversions`
- **Telescope Interface Version:** `GET /api/v1/telescope/{device_number}/interfaceversion`

### Standard Alpaca Response Format

All Alpaca API responses follow this JSON structure:
```json
{
  "Value": <response_value>,
  "ClientTransactionID": 0,
  "ServerTransactionID": 0,
  "ErrorNumber": 0,
  "ErrorMessage": ""
}
```

## Development Notes

These test programs are designed to:
1. Validate connectivity and API access
2. Test parsing of Alpaca responses
3. Prototype functionality before driver implementation
4. Serve as examples for the full INDI driver

## Next Steps

Future test programs will cover:
- Device discovery
- Telescope status queries (position, tracking, etc.)
- Movement commands (slew, sync, park)
- Image capture and download
- Device configuration
