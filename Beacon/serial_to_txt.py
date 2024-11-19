# Imports
import serial
import json
import re
from time import gmtime, strftime
import argparse

# # Parse command-line arguments
# parser = argparse.ArgumentParser(description="Serial port data logger")
# parser.add_argument("serial_port", type=str, help="Serial port to connect to (e.g., /dev/cu.usbserial-52780144351)")
# # parser.add_argument("output_file", type=str, help="Output text file to save data")
# args = parser.parse_args()

# Set up the serial monitor
ser = serial.Serial('/dev/cu.wchusbserial57700020231', 115200)
ser.flushInput()

# Regular expressions to capture the required fields
rssi_snr_pattern = r'rssi: ([\-\d\.]+) snr: ([\d\.]+)'
sduid_pattern = r'sduid: (.+)'
muid_pattern = r'muid: (.+)'
data_pattern = r'data: (.+)'

def save_to_text_file(output_file, json_str):
    """
    Function to save extracted data into a text file.

    Parameters:
    -----------
    output_file: The file to save data to
    rssi: The Received Signal Strength Indicator
    snr: The Signal-to-Noise Ratio
    fe: Frequency error
    size: Size of packet
    json_data: JSON data consisting of the payload data

    Return:
    -------
    Nothing
    """
    with open(output_file, 'a') as f:
        timestamp = strftime("%Y-%m-%d %H:%M:%S", gmtime())
        data_line = f"{timestamp}, {json_str}\n"
        f.write(data_line)

try:
    finish = False
    json_lines = []
    while True:
        line = ser.readline().decode('utf-8').strip()
        print(line)
        # Find rssi and snr
        rssi_snr_match = re.search(rssi_snr_pattern, line)
        sduid_match = re.search(sduid_pattern, line)
        muid_match = re.search(muid_pattern, line)
        data_match = re.search(data_pattern, line)
        
        if rssi_snr_match:
            rssi = rssi_snr_match.group(1).strip()
            snr = rssi_snr_match.group(2).strip()
            print(f"RSSI: {rssi}, SNR: {snr}")
            json_lines.append(rssi)
            json_lines.append(snr)
        elif sduid_match:
            sduid = sduid_match.group(1).strip()
            print(f"SDUID: {sduid}")
            json_lines.append(sduid)
        elif muid_match:
            muid = muid_match.group(1).strip()
            print(f"MUID: {muid}")
            json_lines.append(muid)     
        elif data_match:
            data = data_match.group(1).strip()
            print(f"Data: {data}")
            json_lines.append(data)
            finish = True
        
        if finish:
            json_str = ','.join(json_lines)
            # Save data to the text file
            save_to_text_file('clusterdata.txt', json_str)
            finish = False
            json_lines = []
except KeyboardInterrupt:
    print("Program terminated by user")
finally:
    ser.close()  # Always close the serial port