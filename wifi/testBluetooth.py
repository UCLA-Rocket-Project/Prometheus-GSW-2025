import socketio
import asyncio

# Global Socket.IO async client
sio = socketio.AsyncClient()

WEBSOCKET_ADDRESS = "http://localhost:3000"
LIVE_DATA_PUSH_CHANNEL = "live/receive-data-stream-from-mqtt"

async def notification_handler():
    try:
        if sio.connected:
            await sio.emit(LIVE_DATA_PUSH_CHANNEL, "Your message here")
            print("Data sent")
        else:
            print("Socket is not connected.")
    except Exception as e:
        print(f"Error sending data: {e}")

async def main():
    try:
        await sio.connect(WEBSOCKET_ADDRESS)
        print("Connected to WebSocket.")
        
        await notification_handler()

        # Optional: stay connected or disconnect
        await asyncio.sleep(1)
        await sio.disconnect()
        print("Disconnected.")
    except Exception as e:
        print(f"Connection error: {e}")

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\nStopped by user.")
