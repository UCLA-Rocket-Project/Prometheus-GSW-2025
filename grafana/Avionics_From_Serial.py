from serial import Serial
import socket
import time

PORT = "/dev/cu.usbserial-210"  # Might need to change this based on your device
BAUDRATE = 115_200
ser = Serial(PORT, BAUDRATE, timeout=0.2)

UDPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
avionics_port = ("127.0.0.1", 4002)

# data format: A -00000001,-00000001,-00000001,-00000001,0.000000,0.000000,-0.000000,1.961988,0.000000,0.000000,0.000000,0.000000,0.000000,-1.000000,-1.000000 Z
def process_readings(readings: str) -> dict:
    data = readings.split(",")
    sensor_data = {
        "latitude": data[0],
        "longitude": data[1],
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
        "bmp_altitude": data[14]
    }
    return sensor_data

# TODO: add timestamp data later
def create_telegraf_string(processed_data: dict) -> str:
    measurement = "avionics_data"
    
    field_str = ",".join([f"{key}={value}" for key, value in processed_data.items()])
    
    return f"{measurement} {field_str} {time.time_ns()}"

# Read from Serial
def main():
    ser.reset_input_buffer()  # Flushes the receive buffer
    # read once to clear the buffer of useless values
    ser.read_until(b"Z\n")
    
    while True:
        pt_data = ser.read_until(b"Z\n").strip().decode()
        if pt_data.startswith("A ") and pt_data.endswith(" Z"):
            # remove the string checking characters
            readings = pt_data.split(" ")[1]
            processed_readings = process_readings(readings)

            processed_readings_telegraf_string = create_telegraf_string(processed_readings)
            print("Sent ", processed_readings_telegraf_string)

            # Send data via UDP
            UDPClientSocket.sendto(processed_readings_telegraf_string.encode(), avionics_port)

if __name__ == "__main__":
    main()