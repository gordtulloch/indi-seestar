# SwitchBot Bluetooth Control

## Overview

This directory contains tools to control SwitchBot devices via Bluetooth Low Energy (BLE) instead of the cloud API. This is useful when:
- Cloud service is not enabled on your device
- You want local control without internet
- Faster response times

## Your Device

Device ID: **E13D05062590**  
Bluetooth MAC: **E1:3D:05:06:25:90**  
Type: Bot (Seestar S30)

## Quick Start - Using gatttool

The simplest method is using `gatttool` directly:

```bash
# Press the button
sudo gatttool -b E1:3D:05:06:25:90 -t random --char-write-req -a 0x0016 -n 570100

# Turn ON
sudo gatttool -b E1:3D:05:06:25:90 -t random --char-write-req -a 0x0016 -n 570101

# Turn OFF
sudo gatttool -b E1:3D:05:06:25:90 -t random --char-write-req -a 0x0016 -n 570102
```

**Note**: Add `-t random` for random address type (required for some SwitchBot devices)

## Using the C++ Program

```bash
# Build if not already built
./build.sh

# Send command (run with sudo for Bluetooth access)
sudo ./build/test_bot_ble E1:3D:05:06:25:90 press
sudo ./build/test_bot_ble E1:3D:05:06:25:90 on
sudo ./build/test_bot_ble E1:3D:05:06:25:90 off
```

## Troubleshooting

### Device not responding
1. Make sure Bluetooth is enabled: `sudo systemctl start bluetooth`
2. Check if device is in range (BLE range is typically 10-30 meters)
3. Try resetting Bluetooth: `sudo systemctl restart bluetooth`
4. Make sure no other app is connected to the device

### Connection timeout
- SwitchBot devices can only handle one BLE connection at a time
- Close the SwitchBot app on your phone before using BLE commands
- Device may be in sleep mode - press the physical button to wake it

### Permission denied
- BLE commands require root access: use `sudo`
- Or add your user to the `bluetooth` group:
  ```bash
  sudo usermod -a -G bluetooth $USER
  # Log out and back in for changes to take effect
  ```

## SwitchBot BLE Protocol

### Service UUID
`cba20d00-224d-11e6-9fb8-0002a5d5c51b`

### Write Characteristic
`cba20002-224d-11e6-9fb8-0002a5d5c51b` (Handle: 0x0016)

### Commands
- **Press**: `0x570100`
- **Turn ON**: `0x570101`
- **Turn OFF**: `0x570102`

## Finding Device MAC Address

Your device MAC address can be derived from the device ID:
- Device ID: `E13D05062590`
- MAC Address: `E1:3D:05:06:25:90`

Or scan for devices:
```bash
# Using hcitool
sudo hcitool lescan

# Using bluetoothctl
bluetoothctl
> scan on
# Look for devices starting with "WoHand" (SwitchBot's manufacturer name)
```

## Alternative: Using bluetoothctl

```bash
bluetoothctl
> power on
> scan on
# Wait to see your device
> scan off
> connect E1:3D:05:06:25:90
# Once connected:
> menu gatt
> list-attributes
> select-attribute /org/bluez/hci0/dev_E1_3D_05_06_25_90/service0010/char0016
> write 0x570100  # for press
> disconnect
> quit
```

## Cloud API vs Bluetooth

| Feature | Cloud API | Bluetooth |
|---------|-----------|-----------|
| Range | Unlimited (internet) | 10-30 meters |
| Speed | ~1-2 seconds | <500ms |
| Requires | Cloud service enabled | Physical proximity |
| Setup | Token + Secret | MAC address |
| Concurrent | Multiple apps | One connection |

