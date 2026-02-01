#!/bin/bash

# SwitchBot BLE Control Script
# Usage: ./switchbot_ble.sh <MAC_ADDRESS> <COMMAND>
# Commands: on, off, press

if [ $# -ne 2 ]; then
    echo "Usage: $0 <MAC_ADDRESS> <COMMAND>"
    echo "Example: $0 E1:3D:05:06:25:90 press"
    echo "Commands: on, off, press"
    exit 1
fi

MAC=$1
COMMAND=$2

# SwitchBot BLE Protocol Commands
case $COMMAND in
    press)
        CMD_HEX="570100"
        ;;
    on)
        CMD_HEX="570101"
        ;;
    off)
        CMD_HEX="570102"
        ;;
    *)
        echo "Error: Invalid command '$COMMAND'"
        echo "Valid commands: on, off, press"
        exit 1
        ;;
esac

echo "Sending '$COMMAND' command to $MAC..."

# Try using gatt tool via expect
bluetoothctl << EOF
power on
scan off
connect $MAC
select-attribute /org/bluez/hci0/dev_${MAC//:/_}/service0010/char0016
write 0x$CMD_HEX
disconnect
quit
EOF

echo "Command sent"
