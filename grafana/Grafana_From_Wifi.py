import csv
import sys
import time
import socket
import os.path
import paho.mqtt.client as paho

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
    
    # Open the CSV file and set up the writer
    with open(path, mode='a', newline='') as file:  # Open the correct file here
        writer = csv.writer(file)
        
        # Write the header if the file is empty
        if file.tell() == 0:
            writer.writerow(['pt1', 'pt2', 'pt3', 'pt4', 'pt5', 'pt6', 'pt7', 'pt8', 'lc1', 'lc2', 'timestamp'])
        
        def parseInputAndGrafana(input):
            global flag
            # Read the data and stream
            if flag:
                try:
                    pt_data = input
                except KeyboardInterrupt:
                    # Handle KeyboardInterrupt to gracefully exit the program
                    print(f"\nProgram terminated by user. Wrote data to {path}\n")
                    flag = False
                else:
                    if pt_data.startswith('A') and pt_data.endswith('Z'):
                        
                        # Remove the 'A' at the start and 'Z' at the end
                        pt_data = pt_data[1:-1]
                        
                        # Add the timestamp at the end (you can modify this based on your needs)
                        # pt_data += " " + str(current_time)
                        
                        # Print the data for verification
                        print(pt_data)
                        
                        # Write data to CSV
                        writeToCSV(pt_data, writer)

                        split_pt_data = pt_data.split(",")
                        pt_data = ",".join(split_pt_data[:-1])
                        timestamp = int(split_pt_data[-1].split("=")[1]) * 1_000_000 + start_time_ns
                        pt_data += " " + str(timestamp)

                        print(pt_data)
                        
                        # Send data via UDP
                        UDPClientSocket.sendto(pt_data.encode(), pressure_transducer_port)
                        
                        # Sleep for a short duration before reading the next value
        
        def message_handling(client, userdata, msg):
            parseInputAndGrafana(msg.payload.decode())

        client = paho.Client()
        client.on_message = message_handling
        
        if client.connect("localhost", 1883, 60) != 0:
            print("Couldn't connect to the mqtt broker")
            sys.exit(1)

        client.subscribe("esp32/output")
        try:
            client.loop_forever()
        except KeyboardInterrupt:
            print(f"\nProgram terminated by user. Wrote data to {path}\n")
            flag = False

main()
