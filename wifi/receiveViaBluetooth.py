import asyncio
from bleak import BleakClient, BleakScanner
from typing import Optional

async def scan_for_devices(timeout: int = 5):
    """Scan for BLE devices for the specified timeout period."""
    print(f"Scanning for BLE devices for {timeout} seconds...")
    devices = await BleakScanner.discover(timeout=timeout)
    
    if not devices:
        print("No devices found. Make sure Bluetooth is enabled and devices are in range.")
        return None
        
    print(f"Found {len(devices)} devices:")
    for i, device in enumerate(devices):
        if device.name:
            print(f"{i+1}. {device.name} ({device.address})")
    
    return devices

def notification_handler(sender: str, data: bytearray):
    """Callback for handling incoming notifications."""
    try:
        message = data.decode()
        print(f"\n[Notification] From {sender}: {message}")
    except UnicodeDecodeError:
        print(f"\n[Notification] From {sender}: (non-decodable data) {data}")

async def connect_and_receive(address: str, char_uuid: str):
    """Connect to a BLE device and listen for notifications on a given characteristic."""
    print(f"Attempting to connect to device: {address}")
    
    try:
        async with BleakClient(address) as client:
            print(f"Connected: {client.is_connected}")
            
            # Discover services and characteristics
            print("Discovering services...")
            for service in client.services:
                print(f"Service: {service.uuid}")
                for char in service.characteristics:
                    print(f"  Characteristic: {char.uuid}")
                    print(f"    Properties: {char.properties}")
            
            if not char_uuid:
                print("No characteristic UUID provided. Cannot subscribe to notifications.")
                return
            
            # Subscribe to notifications
            print(f"Subscribing to notifications from: {char_uuid}")
            await client.start_notify(char_uuid, notification_handler)

            print("Listening for notifications... Press Ctrl+C to stop.")
            while True:
                await asyncio.sleep(1)
                
    except Exception as e:
        print(f"Connection or subscription failed: {e}")

async def main():
    # Scan for devices
    devices = await scan_for_devices()
    if devices is None or len(devices) == 0:
        return
    
    # Let user select a device
    choice = -1
    while choice < 1 or choice > len(devices):
        try:
            choice = int(input(f"Select a device (1-{len(devices)}): "))
        except ValueError:
            print("Please enter a valid number")
    
    selected_device = devices[choice-1]
    print(f"Selected: {selected_device.name or 'Unknown'} ({selected_device.address})")
    
    # Let user enter a characteristic UUID to listen for notifications
    char_uuid = input("Enter characteristic UUID to receive notifications from: ")
    
    # Connect and receive
    await connect_and_receive(selected_device.address, char_uuid)

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\nStopped by user.")
