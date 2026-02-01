#!/usr/bin/env python3
"""
SwitchBot BLE Controller using Python
Requires: pip install bluepy
"""

import sys
import struct
from bluepy import btle

# SwitchBot Service and Characteristic UUIDs
SWITCHBOT_SERVICE_UUID = "cba20d00-224d-11e6-9fb8-0002a5d5c51b"
SWITCHBOT_CHAR_WRITE_UUID = "cba20002-224d-11e6-9fb8-0002a5d5c51b"

def send_command(mac_address, command):
    """
    Send command to SwitchBot device
    Commands:
      - 'on':    Turn on (0x570101)
      - 'off':   Turn off (0x570102)
      - 'press': Press button (0x570100)
    """
    commands = {
        'press': bytes([0x57, 0x01, 0x00]),
        'on':    bytes([0x57, 0x01, 0x01]),
        'off':   bytes([0x57, 0x01, 0x02]),
    }
    
    if command not in commands:
        print(f"Error: Unknown command '{command}'")
        print(f"Valid commands: {', '.join(commands.keys())}")
        return False
    
    try:
        print(f"Connecting to {mac_address}...")
        device = btle.Peripheral(mac_address, addrType=btle.ADDR_TYPE_RANDOM)
        
        print("Finding SwitchBot service...")
        service = device.getServiceByUUID(SWITCHBOT_SERVICE_UUID)
        
        print("Finding write characteristic...")
        char = service.getCharacteristics(SWITCHBOT_CHAR_WRITE_UUID)[0]
        
        print(f"Sending '{command}' command...")
        char.write(commands[command], withResponse=True)
        
        device.disconnect()
        print(f"Success! Sent '{command}' command to SwitchBot")
        return True
        
    except btle.BTLEException as e:
        print(f"Bluetooth error: {e}")
        return False
    except Exception as e:
        print(f"Error: {e}")
        return False

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python3 switchbot_ble.py <MAC_ADDRESS> <COMMAND>")
        print("Example: python3 switchbot_ble.py E1:3D:05:06:25:90 press")
        print("Commands: press, on, off")
        sys.exit(1)
    
    mac = sys.argv[1]
    cmd = sys.argv[2].lower()
    
    success = send_command(mac, cmd)
    sys.exit(0 if success else 1)
