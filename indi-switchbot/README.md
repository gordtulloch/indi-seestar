# INDI SwitchBot Driver

INDI auxiliary driver for controlling SwitchBot devices via Bluetooth Low Energy.

## Overview

This driver enables control of SwitchBot Bot devices through the INDI ecosystem, allowing integration with astronomy software like KStars/Ekos. Common use cases include:

- Turning observatory equipment on/off
- Remote power control for devices
- Automated equipment control as part of observatory automation

## Features

- Direct Bluetooth Low Energy (BLE) communication
- No cloud connection required
- Three control modes:
  - **Press**: Momentary button press
  - **Turn On**: Switch to ON position
  - **Turn Off**: Switch to OFF position
- Status monitoring
- Local control with minimal latency

## Requirements

- Linux system with Bluetooth support
- libbluetooth-dev package
- INDI library (2.0.0 or later)
- Root/sudo access for Bluetooth operations

## Installation

### Install Dependencies

```bash
# Ubuntu/Debian
sudo apt-get install libbluetooth-dev libindi-dev cmake build-essential

# Fedora
sudo dnf install bluez-libs-devel indi-devel cmake gcc-c++
```

### Build and Install

```bash
mkdir build
cd build
cmake ..
make
sudo make install
```

## Configuration

### Finding Your SwitchBot Device

To find your SwitchBot's Bluetooth MAC address:

```bash
# Using hcitool
sudo hcitool lescan

# Using bluetoothctl
bluetoothctl
scan on
# Look for "WoHand" devices
```

The MAC address will be in format: `XX:XX:XX:XX:XX:XX`

### Driver Setup

1. Start the INDI server:
   ```bash
   indiserver indi_switchbot
   ```

2. Connect with an INDI client (KStars, INDI Control Panel, etc.)

3. Configure the Bluetooth address:
   - Navigate to "Device" tab
   - Enter your SwitchBot's MAC address
   - Click "Set"

4. Connect the driver

## Usage

### Control Tab

- **Press Button**: Sends a momentary press command
- **Turn On**: Switches device to ON state
- **Turn Off**: Switches device to OFF state

### Status Display

Shows current device state (On/Off/Unknown)

## Permissions

Bluetooth operations require elevated permissions. Options:

### Option 1: Run indiserver with sudo

```bash
sudo indiserver indi_switchbot
```

### Option 2: Grant capabilities to the binary

```bash
sudo setcap cap_net_raw,cap_net_admin+eip /usr/bin/indi_switchbot
```

### Option 3: Add user to bluetooth group

```bash
sudo usermod -a -G bluetooth $USER
# Logout and login for changes to take effect
```

## Troubleshooting

### Driver won't connect

- Verify Bluetooth adapter is working: `hciconfig`
- Check device address is correct
- Ensure SwitchBot is in range (< 10 meters)
- Verify SwitchBot batteries are charged

### "No Bluetooth adapter found"

```bash
# Check adapter status
hciconfig
sudo hciconfig hci0 up
```

### Permission denied errors

Run with sudo or configure capabilities as described above.

### Device not responding

- Move SwitchBot closer to computer
- Check battery level
- Try pressing physical button to wake device
- Ensure no other apps are connected to the device

## Technical Details

### Protocol

Uses SwitchBot BLE protocol via cpp-switchbot library:
- Service UUID: cba20d00-224d-11e6-9fb8-0002a5d5c51b
- Write characteristic: cba20002-224d-11e6-9fb8-0002a5d5c51b
- Commands:
  - Press: `0x570100`
  - On: `0x570101`
  - Off: `0x570102`

### Architecture

```
INDI Client (KStars/Ekos)
    ↓
INDI Server
    ↓
indi_switchbot driver
    ↓
cpp-switchbot library
    ↓
Bluetooth adapter
    ↓
SwitchBot device
```

## Integration with KStars/Ekos

The driver can be integrated into Ekos sequences and scripts for automation:

1. **Observatory Power On**: Use SwitchBot to turn on power strip
2. **Equipment Control**: Turn on/off individual equipment
3. **Safety Shutdowns**: Automated power-off on weather alerts

## Related Projects

- [cpp-switchbot](cpp-switchbot/): C++ library for SwitchBot control
- [INDI Library](https://github.com/indilib/indi): Instrument Neutral Distributed Interface

## License

GNU General Public License v2.0 - see LICENSE file

## Author

Rick Bassham

## Contributing

Contributions welcome! Please submit issues and pull requests on GitHub.

## Version History

### 1.0.0 (2025-02-01)
- Initial release
- Basic Bot control (press, on, off)
- BLE communication via cpp-switchbot
- Status monitoring
