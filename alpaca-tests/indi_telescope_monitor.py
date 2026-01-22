#!/usr/bin/env python3
"""
INDI Telescope Monitor - Simple Python test client

This client connects to an INDI telescope driver and displays
real-time status updates including position, state, tracking, etc.

Usage:
    python3 indi_telescope_monitor.py [--host HOST] [--port PORT] [--device DEVICE]
"""

import sys
import time
import socket
import xml.etree.ElementTree as ET
from datetime import datetime
import argparse

class INDITelescopeMonitor:
    def __init__(self, host='localhost', port=7624, device='alpaca_telescope'):
        self.host = host
        self.port = port
        self.device_name = device
        self.sock = None
        self.connected = False
        self.device_connected = False
        
        # Telescope state
        self.current_ra = 0.0
        self.current_dec = 0.0
        self.target_ra = 0.0
        self.target_dec = 0.0
        self.current_alt = 0.0
        self.current_az = 0.0
        self.is_tracking = False
        self.is_parked = False
        self.track_mode = "UNKNOWN"
        self.telescope_state = "UNKNOWN"
        self.has_coords = False
        
        self.buffer = b''
        
    def connect(self):
        """Connect to INDI server"""
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect((self.host, self.port))
            self.sock.settimeout(0.1)  # Non-blocking with timeout
            self.connected = True
            print(f"Connected to INDI server at {self.host}:{self.port}")
            
            # Request properties from our device
            self.send_message(f'<getProperties version="1.7" device="{self.device_name}"/>\n')
            
            return True
        except Exception as e:
            print(f"Failed to connect: {e}")
            return False
    
    def disconnect(self):
        """Disconnect from INDI server"""
        if self.sock:
            self.sock.close()
            self.sock = None
        self.connected = False
        print("Disconnected from INDI server")
    
    def send_message(self, message):
        """Send XML message to INDI server"""
        if self.sock:
            self.sock.sendall(message.encode('utf-8'))
    
    def receive_data(self):
        """Receive and process data from INDI server"""
        try:
            data = self.sock.recv(4096)
            if not data:
                return False
            
            self.buffer += data
            
            # Process complete XML messages
            while True:
                # Find complete XML element
                start = self.buffer.find(b'<')
                if start == -1:
                    self.buffer = b''
                    break
                
                # Look for various closing tags
                end_tags = [b'</setNumberVector>', b'</setSwitchVector>', 
                           b'</setTextVector>', b'</setBLOBVector>',
                           b'</defNumberVector>', b'</defSwitchVector>',
                           b'</defTextVector>', b'</defBLOBVector>',
                           b'</message>', b'</delProperty>']
                
                end = -1
                for tag in end_tags:
                    pos = self.buffer.find(tag, start)
                    if pos != -1:
                        if end == -1 or pos < end:
                            end = pos + len(tag)
                
                if end == -1:
                    # No complete message yet
                    break
                
                # Extract and process message
                message = self.buffer[start:end]
                self.buffer = self.buffer[end:]
                
                try:
                    self.process_message(message.decode('utf-8'))
                except Exception as e:
                    pass  # Ignore parsing errors
            
            return True
            
        except socket.timeout:
            return True  # Timeout is OK, just means no data
        except Exception as e:
            print(f"Error receiving data: {e}")
            return False
    
    def process_message(self, xml_str):
        """Process incoming XML message"""
        try:
            root = ET.fromstring(xml_str)
            
            # Check device name
            device = root.get('device', '')
            if device != self.device_name:
                return
            
            prop_name = root.get('name', '')
            state = root.get('state', '')
            
            # Connection status
            if prop_name == 'CONNECTION':
                for elem in root.findall('.//oneSwitch'):
                    if elem.get('name') == 'CONNECT' and elem.text == 'On':
                        if not self.device_connected:
                            self.device_connected = True
                            print("\n=== TELESCOPE CONNECTED ===")
                            self.display_status()
            
            # Equatorial coordinates
            elif prop_name == 'EQUATORIAL_EOD_COORD':
                for elem in root.findall('.//oneNumber'):
                    name = elem.get('name')
                    if name == 'RA':
                        self.current_ra = float(elem.text)
                        self.has_coords = True
                    elif name == 'DEC':
                        self.current_dec = float(elem.text)
                
                self.telescope_state = state
                self.display_status()
            
            # Target coordinates
            elif prop_name == 'TARGET_EOD_COORD':
                for elem in root.findall('.//oneNumber'):
                    name = elem.get('name')
                    if name == 'RA':
                        self.target_ra = float(elem.text)
                    elif name == 'DEC':
                        self.target_dec = float(elem.text)
            
            # Horizontal coordinates
            elif prop_name == 'HORIZONTAL_COORD':
                for elem in root.findall('.//oneNumber'):
                    name = elem.get('name')
                    if name == 'AZ':
                        self.current_az = float(elem.text)
                    elif name == 'ALT':
                        self.current_alt = float(elem.text)
                self.display_status()
            
            # Tracking state
            elif prop_name == 'TELESCOPE_TRACK_STATE':
                for elem in root.findall('.//oneSwitch'):
                    if elem.get('name') == 'TRACK_ON' and elem.text == 'On':
                        self.is_tracking = True
                    elif elem.get('name') == 'TRACK_OFF' and elem.text == 'On':
                        self.is_tracking = False
                self.display_status()
            
            # Tracking mode
            elif prop_name == 'TELESCOPE_TRACK_MODE':
                for elem in root.findall('.//oneSwitch'):
                    if elem.text == 'On':
                        self.track_mode = elem.get('name', 'UNKNOWN')
                self.display_status()
            
            # Park state
            elif prop_name == 'TELESCOPE_PARK':
                for elem in root.findall('.//oneSwitch'):
                    if elem.get('name') == 'PARK' and elem.text == 'On':
                        self.is_parked = True
                    elif elem.get('name') == 'UNPARK' and elem.text == 'On':
                        self.is_parked = False
                self.display_status()
            
            # Messages
            elif root.tag == 'message':
                message = root.get('message', '')
                timestamp = root.get('timestamp', datetime.now().isoformat())
                print(f"[MSG {timestamp}] {message}")
                
        except ET.ParseError:
            pass  # Ignore incomplete XML
        except Exception as e:
            pass  # Ignore other errors
    
    def display_status(self):
        """Display current telescope status"""
        if not self.device_connected:
            return
        
        print("\n" + "="*50)
        print("  TELESCOPE STATUS MONITOR")
        print("="*50)
        
        # Coordinates
        print("\n📍 COORDINATES:")
        if self.has_coords:
            # Convert RA from hours to HMS
            ra_hours = int(self.current_ra)
            ra_minutes = int((self.current_ra - ra_hours) * 60)
            ra_seconds = ((self.current_ra - ra_hours) * 60 - ra_minutes) * 60
            
            # Convert Dec to DMS
            dec_sign = '+' if self.current_dec >= 0 else '-'
            abs_dec = abs(self.current_dec)
            dec_degrees = int(abs_dec)
            dec_minutes = int((abs_dec - dec_degrees) * 60)
            dec_seconds = ((abs_dec - dec_degrees) * 60 - dec_minutes) * 60
            
            print(f"  Current Position:")
            print(f"    RA  = {ra_hours:02d}h {ra_minutes:02d}m {ra_seconds:05.2f}s  "
                  f"({self.current_ra:.6f} hours = {self.current_ra * 15.0:.6f}°)")
            print(f"    Dec = {dec_sign}{dec_degrees:02d}° {dec_minutes:02d}' {dec_seconds:05.2f}\"  "
                  f"({self.current_dec:.6f}°)")
            print(f"    Alt = {self.current_alt:.2f}°")
            print(f"    Az  = {self.current_az:.2f}°")
        else:
            print("  No coordinate data available yet")
        
        # State
        print("\n⚙️  STATE:")
        status_map = {
            'Idle': 'IDLE',
            'Ok': 'OK', 
            'Busy': 'SLEWING',
            'Alert': 'ALERT'
        }
        
        if self.is_parked:
            status = "PARKED"
        elif self.telescope_state == 'Busy':
            status = "SLEWING"
        elif self.is_tracking:
            status = "TRACKING"
        else:
            status = status_map.get(self.telescope_state, self.telescope_state)
        
        print(f"  Status: {status}")
        
        # Tracking
        print("\n🎯 TRACKING:")
        print(f"  Enabled: {'YES' if self.is_tracking else 'NO'}")
        print(f"  Mode:    {self.track_mode}")
        
        # Park
        print("\n🅿️  PARK:")
        print(f"  Parked:  {'YES' if self.is_parked else 'NO'}")
        
        print("="*50)
    
    def run(self):
        """Main loop"""
        if not self.connect():
            return
        
        print(f"Monitoring telescope: {self.device_name}")
        print("Press Ctrl+C to exit\n")
        
        try:
            while True:
                if not self.receive_data():
                    print("Connection lost")
                    break
                time.sleep(0.1)
        except KeyboardInterrupt:
            print("\n\nShutting down...")
        finally:
            self.disconnect()

def main():
    parser = argparse.ArgumentParser(
        description='INDI Telescope Monitor - Display real-time telescope status'
    )
    parser.add_argument('--host', default='localhost',
                      help='INDI server host (default: localhost)')
    parser.add_argument('--port', type=int, default=7624,
                      help='INDI server port (default: 7624)')
    parser.add_argument('--device', default='alpaca_telescope',
                      help='Telescope device name (default: alpaca_telescope)')
    
    args = parser.parse_args()
    
    print("="*50)
    print("  INDI Telescope Monitor")
    print("="*50)
    print(f"Connecting to: {args.host}:{args.port}")
    print(f"Telescope:     {args.device}")
    print("Press Ctrl+C to exit")
    print("="*50 + "\n")
    
    monitor = INDITelescopeMonitor(args.host, args.port, args.device)
    monitor.run()

if __name__ == '__main__':
    main()
