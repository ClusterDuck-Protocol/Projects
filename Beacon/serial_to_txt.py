import serial
import re
import csv
from time import gmtime, strftime

# Set up the serial monitor
ser = serial.Serial('/dev/cu.wchusbserial52780149881', 115200)
ser.flushInput()

# Regular expressions to capture the required fields
rssi_snr_pattern = r'rssi: ([\-\d\.]+) snr: ([\d\.]+)'
sduid_pattern = r'sduid: (.+)'
muid_pattern = r'muid: (.+)'
data_pattern = r'data: (.+)'
hops_pattern = r'hops:\s*(\d+)'
gps_pattern = r'GPS:\s*Lat:\s*([\d\.\-]+)\s*Lng:\s*([\d\.\-]+)\s*Alt:\s*([\d\.\-]+)\s*Time:\s*([\d:]+)'

# CSV file setup
csv_file = 'clusterdata.csv'
headers = ['timestamp', 'rssi', 'snr', 'sduid', 'muid', 'data', 'hops', 'latitude', 'longitude', 'altitude', 'gps_time']

# Check if the file exists; if not, create it and write the headers
if not os.path.exists(csv_file):
    with open(csv_file, 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(headers)  # Write headers if the file doesn't exist

def save_to_csv_file(output_file, data_dict):
    """
    Save extracted data into a CSV file.

    Parameters:
    -----------
    output_file: The file to save data to
    data_dict: A dictionary containing extracted values

    Return:
    -------
    Nothing
    """
    with open(output_file, 'a', newline='') as f:
        writer = csv.writer(f)
        row = [
            data_dict.get('timestamp', ''),
            data_dict.get('rssi', ''),
            data_dict.get('snr', ''),
            data_dict.get('sduid', ''),
            data_dict.get('muid', ''),
            data_dict.get('data', ''),
            data_dict.get('hops', ''),
            data_dict.get('latitude', ''),
            data_dict.get('longitude', ''),
            data_dict.get('altitude', ''),
            data_dict.get('gps_time', '')
        ]
        writer.writerow(row)


try:
    # Buffer to hold the extracted data
    data_buffer = {}

    while True:
        # Read and decode a line from the serial port
        line = ser.readline().decode('utf-8').strip()
        print(f"DEBUG: {line}")  # Debug: print the line being processed

        # Match patterns and update the data buffer
        rssi_snr_match = re.search(rssi_snr_pattern, line)
        sduid_match = re.search(sduid_pattern, line)
        muid_match = re.search(muid_pattern, line)
        data_match = re.search(data_pattern, line)
        hops_match = re.search(hops_pattern, line)
        gps_match = re.search(gps_pattern, line)

        if rssi_snr_match:
            data_buffer['rssi'] = rssi_snr_match.group(1).strip()
            data_buffer['snr'] = rssi_snr_match.group(2).strip()
            print(f"DEBUG: RSSI: {data_buffer['rssi']}, SNR: {data_buffer['snr']}")

        if sduid_match:
            data_buffer['sduid'] = sduid_match.group(1).strip()
            print(f"DEBUG: SDUID: {data_buffer['sduid']}")

        if muid_match:
            data_buffer['muid'] = muid_match.group(1).strip()
            print(f"DEBUG: MUID: {data_buffer['muid']}")

        if data_match:
            data_buffer['data'] = data_match.group(1).strip()
            print(f"DEBUG: Data: {data_buffer['data']}")

        if hops_match:
            data_buffer['hops'] = hops_match.group(1)
            print(f"DEBUG: Hops: {data_buffer['hops']}")

        if gps_match:
            data_buffer['latitude'] = gps_match.group(1)
            data_buffer['longitude'] = gps_match.group(2)
            data_buffer['altitude'] = gps_match.group(3)
            data_buffer['gps_time'] = gps_match.group(4)
            print(f"DEBUG: GPS - Latitude: {data_buffer['latitude']}, Longitude: {data_buffer['longitude']}, Altitude: {data_buffer['altitude']}, Time: {data_buffer['gps_time']}")

        # Save data when all fields are captured
        if 'rssi' in data_buffer and 'sduid' in data_buffer and 'muid' in data_buffer and \
           'data' in data_buffer and 'hops' in data_buffer and 'latitude' in data_buffer:
            # Add a timestamp
            data_buffer['timestamp'] = strftime("%Y-%m-%d %H:%M:%S", gmtime())

            # Save to CSV
            save_to_csv_file(csv_file, data_buffer)
            print("DEBUG: Data saved:", data_buffer)

            # Clear the buffer for the next set of data
            data_buffer = {}

except KeyboardInterrupt:
    print("Program terminated by user")
finally:
    ser.close()  # Always close the serial port
