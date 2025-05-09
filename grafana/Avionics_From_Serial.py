from serial import Serial
import socket
import time
import socketio
import csv

PORT = "/dev/cu.usbserial-10"  # Might need to change this based on your device
BAUDRATE = 115_200
ser = Serial(PORT, BAUDRATE, timeout=0.2)

WEBSOCKET_ADDRESS = "https://to-da-moon.onrender.com/"
LIVE_DATA_PUSH_CHANNEL = "live/receive-data-stream-from-mqtt"
LIVE_LATLNG_PUSH_CHANNEL = "live/receive-data-stream-from-mqtt-latlng"
sio = socketio.Client()

start_time = time.time_ns()

UDPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
avionics_port = ("127.0.0.1", 4002)

def format_data(sensor_data):
    hud_config = [
        {
            'dataName': 'Latitude',
            'dataCol': 1,
            'min': -90.0,
            'max': 90.0,
            'value': sensor_data['latitude'],
            'secondDisplayType': 'graph',
            'units': '°',
        },
        {
            'dataName': 'Longitude',
            'dataCol': 2,
            'min': -180.0,
            'max': 180.0,
            'value': sensor_data['longitude'],
            'secondDisplayType': 'graph',
            'units': '°',
        },
        {
            'dataName': 'Altitude (GPS)',
            'dataCol': 3,
            'min': -500.0,
            'max': 10000.0,
            'value': sensor_data['gps_altitude'],
            'secondDisplayType': 'graph',
            'units': 'm',
        },
        {
            'dataName': 'Heading',
            'dataCol': 4,
            'min': 0.0,
            'max': 360.0,
            'value': sensor_data['heading'],
            'secondDisplayType': 'graph',
            'units': '°',
        },
        {
            'dataName': 'Accel X',
            'dataCol': 5,
            'min': -20.0,
            'max': 20.0,
            'value': sensor_data['accelX'],
            'secondDisplayType': 'graph',
            'units': 'm/s²',
        },
        {
            'dataName': 'Accel Y',
            'dataCol': 6,
            'min': -20.0,
            'max': 20.0,
            'value': sensor_data['accelY'],
            'secondDisplayType': 'graph',
            'units': 'm/s²',
        },
        {
            'dataName': 'Accel Z',
            'dataCol': 7,
            'min': -20.0,
            'max': 20.0,
            'value': sensor_data['accelZ'],
            'secondDisplayType': 'graph',
            'units': 'm/s²',
        },
        {
            'dataName': 'Gyro X',
            'dataCol': 8,
            'min': -250.0,
            'max': 250.0,
            'value': sensor_data['gyroX'],
            'secondDisplayType': 'graph',
            'units': '°/s',
        },
        {
            'dataName': 'Gyro Y',
            'dataCol': 9,
            'min': -250.0,
            'max': 250.0,
            'value': sensor_data['gyroY'],  # Fixed typo
            'secondDisplayType': 'graph',
            'units': '°/s',
        },
        {
            'dataName': 'Gyro Z',
            'dataCol': 10,
            'min': -250.0,
            'max': 250.0,
            'value': sensor_data['gyroZ'],  # Fixed typo
            'secondDisplayType': 'graph',
            'units': '°/s',
        },
        {
            'dataName': 'Mag X',
            'dataCol': 11,
            'min': -100.0,
            'max': 100.0,
            'value': sensor_data['magX'],
            'secondDisplayType': 'graph',
            'units': 'µT',
        },
        {
            'dataName': 'Mag Y',
            'dataCol': 12,
            'min': -100.0,
            'max': 100.0,
            'value': sensor_data['magY'],
            'secondDisplayType': 'graph',
            'units': 'µT',
        },
        {
            'dataName': 'Mag Z',
            'dataCol': 13,
            'min': -100.0,
            'max': 100.0,
            'value': sensor_data['magZ'],
            'secondDisplayType': 'graph',
            'units': 'µT',
        },
        {
            'dataName': 'Pressure',
            'dataCol': 14,
            'min': 30.0,
            'max': 200.0,
            'value': sensor_data['pressure'],
            'secondDisplayType': 'graph',
            'units': 'kPa',
        },
        {
            'dataName': 'Altitude (BMP)',
            'dataCol': 15,
            'min': -500.0,
            'max': 10000.0,
            'value': sensor_data['bmp_altitude'],
            'secondDisplayType': 'graph',
            'units': 'm',
        }
    ]

    return hud_config



# data format: A -00000001,-00000001,-00000001,-00000001,0.000000,0.000000,-0.000000,1.961988,0.000000,0.000000,0.000000,0.000000,0.000000,-1.000000,-1.000000 Z
def process_readings(readings: str) -> dict:
    data = readings.split(",")
    sensor_data = {
        "latitude": int(data[0]) * 10**-7,
        "longitude": int(data[1]) * 10**-7,
        "gps_altitude": data[2],
        "heading": data[3],
        "accelX": data[4],
        "accelY": data[5],
        "accelZ": data[6],
        "gyroX": data[7],
        "gyroY": data[8],
        "gyroZ": data[9],
        "magX": data[10],
        "magY": data[11],
        "magZ": data[12],
        "pressure": data[13],
        "bmp_altitude": data[14],
        "time_since_start": data[15],
        "rssi": data[16]
    }
    # sensor_data = {
    #     "latitude": 35.3426,  # UCLA's latitude
    #     "longitude": -117.7989,  # UCLA's longitude
    #     "gps_altitude": 71.3,  # meters
    #     "heading": 45.0,  # degrees
    #     "accelX": 0.0,  # m/s^2
    #     "accelY": 0.0,  # m/s^2
    #     "accelZ": -9.81,  # m/s^2 (gravity)
    #     "gyroX": 0.0,  # degrees/s
    #     "gyroY": 0.0,  # degrees/s
    #     "gyroZ": 0.0,  # degrees/s
    #     "magX": 0.0,  # μT
    #     "magY": 0.0,  # μT
    #     "magZ": 0.0,  # μT
    #     "pressure": 101.325,  # kPa (sea level standard)
    #     "bmp_altitude": 71.3,  # meters (matching GPS altitude)
    #     "time_since_start": 7.0
    # }
    return sensor_data

# TODO: add timestamp data later
def create_telegraf_string(processed_data: dict) -> str:
    measurement = "avionics_data"
    
    field_str = ",".join([f"{key}={value}" for key, value in processed_data.items()])
    
    return f"{measurement} {field_str} {str(start_time + int(processed_data['time_since_start']) * 10**6)}"

def send_to_websocket(data, channel=LIVE_DATA_PUSH_CHANNEL):
    try:
        sio.emit(channel, data)
        print(f"WebSocket Data Sent to {channel}:", data, "\n\n")
    except Exception as e:
        print("WebSocket Emit Error:", e)
def extract_latlng_forwebsocket(sensor_data):
    targetLatLng = {
        "latitude": sensor_data['latitude'],
        "longitude":sensor_data['longitude']
    }
    return targetLatLng

# Read from Serial
def main():
    try:
        sio.connect(WEBSOCKET_ADDRESS, wait_timeout=10)
    except Exception as e:
        print("WebSocket Connection Error:", e)
        return
    
    ser.reset_input_buffer()  # Flushes the receive buffer
    # read once to clear the buffer of useless values
    ser.read_until(b"Z\n")

    # Define CSV file name
    csv_filename = "sensor_data_log.csv"

    # Define the fieldnames from the keys of processed_readings
    fieldnames = [
        "latitude", "longitude", "gps_altitude", "heading",
        "accelX", "accelY", "accelZ",
        "gyroX", "gyroY", "gyroZ",
        "magX", "magY", "magZ",
        "pressure", "bmp_altitude", "time_since_start","rssi"
    ]

    # Open CSV file in write mode with newline='' to avoid blank lines on Windows
    with open(csv_filename, mode='w', newline='') as csvfile:
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader() 

        while True:
            try:
                pt_data = ""

                try:
                    pt_data = ser.read_until(b"Z\n").strip().decode()
                except UnicodeDecodeError as e:
                    pass
                
                if pt_data.startswith("A ") and pt_data.endswith(" Z"):
                    print("Received ", pt_data, "\n\n")

                    # remove the string checking characters
                    readings = pt_data.split(" ")[1]
                    try:
                        processed_readings = process_readings(readings)
                    except Exception:
                        pass
                    # Write to CSV
                    writer.writerow(processed_readings)


                    processed_readings_telegraf_string = create_telegraf_string(processed_readings)
                    print("Sent ", processed_readings_telegraf_string)
                    print("\n\n")

                    # Send data via UDP
                    UDPClientSocket.sendto(processed_readings_telegraf_string.encode(), avionics_port)
                    try:
                        send_to_websocket(format_data(processed_readings))
                        send_to_websocket(extract_latlng_forwebsocket(processed_readings),LIVE_LATLNG_PUSH_CHANNEL)
                    except Exception as e:
                        # Optionally log this
                        print(f"WebSocket send failed: {e}")
                        pass
            except Exception as e:
                print(f"ERROR, passing now: {e}")
                pass
       
    

        # while True: 
        #     print(create_telegraf_string(process_readings("sheesh")))
        #     writer.writerow(process_readings("sheesh"))
        #     UDPClientSocket.sendto(create_telegraf_string(process_readings("sheesh")).encode(), avionics_port)
        #     time.sleep(1)
        #     try:
        #         send_to_websocket(format_data(process_readings("sheesh")))
        #         send_to_websocket(extract_latlng_forwebsocket(process_readings("sheesh")),LIVE_LATLNG_PUSH_CHANNEL)
        #     except Exception as e:
        #             # Optionally log this
        #             print(f"WebSocket send failed: {e}")
        #             pass


if __name__ == "__main__":
    main()