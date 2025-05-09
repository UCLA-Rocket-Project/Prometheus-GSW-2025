import csv
import sys
import time
import socket
import os.path
import json
import paho.mqtt.client as paho
import socketio

WEBSOCKET_ADDRESS = "http://localhost:3001/"
LIVE_DATA_PUSH_CHANNEL = "live/receive-data-stream-from-mqtt"
sio = socketio.Client()
PORT = 'COM5'  # windows
#PORT = '/dev/tty.usbserial-0001' # mac
BAUDRATE = 115200  # Might need to change this based on your device
# ser = Serial(PORT, BAUDRATE, timeout=0.1)
UDPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
pressure_transducer_port = ("127.0.0.1", 4001)
flag = True
start_time_ns = time.time_ns()
fileName = 'sensor_data'
path = fileName+'.csv'
fileCt = 0
while os.path.exists(path):
    path = fileName+'_'+str(fileCt)+'.csv'
    fileCt += 1

def convert_to_dict(arr):
    result = {}
    for item in arr:
        key, value = item.split('=')
        try:
            result[key] = float(value)
        except ValueError:
            result[key] = value
    return result


def format_data_for_websockets(sensor_string):
        sensor_data = convert_to_dict(sensor_string)
        hud_config = [
        {
            'dataName': 'Pressure Transducer 1',
            'dataCol': 1,
            'min': -1000.0,
            'max': 1000.0,
            'value': 500,
            'secondDisplayType': 'graph',
            'units': '°',
        },
        {
    'dataName': 'Pressure Transducer 2',
    'dataCol': 1,
    'min': -1000.0,
    'max': 1000.0,
    'value': 500,
    'secondDisplayType': 'graph',
    'units': '°',
},
        {
    'dataName': 'Pressure Transducer 3',
    'dataCol': 1,
    'min': -1000.0,
    'max': 1000.0,
    'value': 500,
    'secondDisplayType': 'graph',
    'units': '°',
},
        {
    'dataName': 'Pressure Transducer 4',
    'dataCol': 1,
    'min': -1000.0,
    'max': 1000.0,
    'value': 500,
    'secondDisplayType': 'graph',
    'units': '°',
},
        {
    'dataName': 'Pressure Transducer 5',
    'dataCol': 1,
    'min': -1000.0,
    'max': 1000.0,
    'value': 500,
    'secondDisplayType': 'graph',
    'units': '°',
},
        {
    'dataName': 'Pressure Transducer 6',
    'dataCol': 1,
    'min': -1000.0,
    'max': 1000.0,
    'value': 500,
    'secondDisplayType': 'graph',
    'units': '°',
},
 {
    'dataName': 'Load Cell 1',
    'dataCol': 1,
    'min': -1000.0,
    'max': 1000.0,
    'value': 500,
    'secondDisplayType': 'graph',
    'units': '°',
},
 {
    'dataName': 'Load Cell 2',
    'dataCol': 1,
    'min': -1000.0,
    'max': 1000.0,
    'value': 500,
    'secondDisplayType': 'graph',
    'units': '°',
},
        ]
        return hud_config

def send_to_websocket(data, channel=LIVE_DATA_PUSH_CHANNEL):
    try:
        sio.emit(channel, data)
        print(f"WebSocket Data Sent to {channel}:", data, "\n\n")
    except Exception as e:
        print("WebSocket Emit Error:", e)

def writeToCSV(pt_data, writer):
    # Split pt_data into its components
    # pt_data_parts = pt_data.split()  # Split the string by spaces
    # # The last part is the timestamp, which has no key label
    # timestamp = pt_data_parts[-1]  # Last element is the timestamp
    # Extract sensor values from the remaining parts (all except the last part)
    sensor_values = pt_data.split(" ")[1].split(",")
    print(sensor_values) #this one does
    # Extract each sensor value by splitting on the '=' character
    pt1 = sensor_values[0].split("=")[1]
    pt2 = sensor_values[1].split("=")[1]
    pt3 = sensor_values[2].split("=")[1]
    pt4 = sensor_values[3].split("=")[1]
    pt5 = sensor_values[4].split("=")[1]
    pt6 = sensor_values[5].split("=")[1]
    pt7 = sensor_values[6].split("=")[1]
    pt8 = sensor_values[7].split("=")[1]
    lc1 = sensor_values[8].split("=")[1]
    lc2 = sensor_values[9].split("=")[1]
    time_since_start = sensor_values[10].split("=")[1]
    # Write data to CSV
    writer.writerow([pt1, pt2, pt3, pt4, pt5, pt6, pt7, pt8, lc1, lc2, int(float(time_since_start) * 1_000_000) + start_time_ns])
def main():
    global flag
    flag = True
    try:
        sio.connect(WEBSOCKET_ADDRESS, wait_timeout=10)
    except Exception as e:
        print("WebSocket Connection Error:", e)
        return
    while True:
        time.sleep(1)
        send_to_websocket(json.dumps([
        {
            'dataName': 'Pressure Transducer 1',
            'dataCol': 1,
            'min': -1000.0,
            'max': 1000.0,
            'value': 500,
            'secondDisplayType': 'graph',
            'units': '°',
        },
        {
    'dataName': 'Pressure Transducer 2',
    'dataCol': 1,
    'min': -1000.0,
    'max': 1000.0,
    'value': 500,
    'secondDisplayType': 'graph',
    'units': '°',
},
        {
    'dataName': 'Pressure Transducer 3',
    'dataCol': 1,
    'min': -1000.0,
    'max': 1000.0,
    'value': 500,
    'secondDisplayType': 'graph',
    'units': '°',
},
        {
    'dataName': 'Pressure Transducer 4',
    'dataCol': 1,
    'min': -1000.0,
    'max': 1000.0,
    'value': 500,
    'secondDisplayType': 'graph',
    'units': '°',
},
        {
    'dataName': 'Pressure Transducer 5',
    'dataCol': 1,
    'min': -1000.0,
    'max': 1000.0,
    'value': 500,
    'secondDisplayType': 'graph',
    'units': '°',
},
        {
    'dataName': 'Pressure Transducer 6',
    'dataCol': 1,
    'min': -1000.0,
    'max': 1000.0,
    'value': 500,
    'secondDisplayType': 'graph',
    'units': '°',
},
 {
    'dataName': 'Load Cell 1',
    'dataCol': 1,
    'min': -1000.0,
    'max': 1000.0,
    'value': 500,
    'secondDisplayType': 'graph',
    'units': '°',
},
 {
    'dataName': 'Load Cell 2',
    'dataCol': 1,
    'min': -1000.0,
    'max': 1000.0,
    'value': 500,
    'secondDisplayType': 'graph',
    'units': '°',
}]
        ))
    # Open the CSV file and set up the writer
    # with open(path, mode='a', newline='') as file:  # Open the correct file here
    #     writer = csv.writer(file)
    #     # Write the header if the file is empty
    #     if file.tell() == 0:
    #         writer.writerow(['pt1', 'pt2', 'pt3', 'pt4', 'pt5', 'pt6', 'pt7', 'pt8', 'lc1', 'lc2', 'timestamp'])
    #     def parseInputAndGrafana(input):
    #         global flag
    #         # Read the data and stream
    #         if flag:
    #             try:
    #                 pt_data = input
    #             except KeyboardInterrupt:
    #                 # Handle KeyboardInterrupt to gracefully exit the program
    #                 print(f"\nProgram terminated by user. Wrote data to {path}\n")
    #                 flag = False
    #             else:
    #                 if pt_data.startswith('A') and pt_data.endswith('Z'):
    #                     # Remove the 'A' at the start and 'Z' at the end
    #                     pt_data = pt_data[1:-1]
    #                     # Add the timestamp at the end (you can modify this based on your needs)
    #                     # pt_data += " " + str(current_time)
    #                     # Print the data for verification
    #                     print(pt_data)
    #                     forwebsockets = pt_data
    #                     # Write data to CSV
    #                     writeToCSV(pt_data, writer)
    #                     split_pt_data = pt_data.split(",")
    #                     pt_data = ",".join(split_pt_data[:-1])
    #                     timestamp = int(split_pt_data[-1].split("=")[1]) * 1_000_000 + start_time_ns
    #                     pt_data += " " + str(timestamp)
    #                     print(pt_data)
    #                     # Send data via UDP
    #                     UDPClientSocket.sendto(pt_data.encode(), pressure_transducer_port)
    #                     # Sleep for a short duration before reading the next value
    #                     print(pt_data.encode())
    #                     try:
    #                         send_to_websocket(format_data_for_websockets(forwebsockets))
    #                     except Exception as e:
    #                         # Optionally log this
    #                         print(f"WebSocket send failed: {e}")
    #                         pass
    #     def message_handling(client, userdata, msg):
    #         parseInputAndGrafana(msg.payload.decode())
    #     #client = paho.Client(client_id="unique_client_id", protocol=paho.MQTTv311)
    #     client = paho.Client()
    #     client.on_message = message_handling
    #     if client.connect("localhost", 1883, 60) != 0:
    #         print("Couldn't connect to the mqtt broker")
    #         sys.exit(1)
    #     client.subscribe("esp32/output")
    #     try:
    #         client.loop_forever()
    #     except KeyboardInterrupt:
    #         print(f"\nProgram terminated by user. Wrote data to {path}\n")
    #         flag = False
main()






