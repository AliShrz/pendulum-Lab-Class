

#%%
import serial
import time
import pandas as pd

raw_data = []
data = pd.DataFrame(columns=['xAccl', 'yAccl', 'zAccl'])
try:
    ser = serial.Serial(port='COM3', baudrate=115200, timeout=1)
    print(f"Serial port {ser.port} opened successfully.")
    
    
    start_time = time.time()
    collection_time = 45
    current_time = time.time() 
    while start_time + collection_time > current_time:
        try:
            current_time = time.time()
            line = ser.readline().decode('utf-8').strip()
            raw_data.append([line, current_time])
            if line == "exit":
                loop = False
        except UnicodeDecodeError:
            print("Error decoding serial data.")
    ser.close()
    print("Serial port closed.")
except serial.SerialException as e:
    print(f"Error opening serial port: {e}")
#%%
# Remove the first two entries of raw_data
if len(raw_data) > 2:
    raw_data = raw_data[2:]
#%%
# Extract data from raw_data and populate the DataFrame
for entry in raw_data:

    try:
        sensor_data = eval(entry[0])  # Convert string to dictionary
        timestamp = entry[1]
        data = pd.concat([data, pd.DataFrame([{
                    'xAccl': sensor_data['xAccl'],
                    'yAccl': sensor_data['yAccl'],
                    'zAccl': sensor_data['zAccl'],
                    'time': timestamp
                }])], ignore_index=True)
    except (SyntaxError, KeyError) as e:
        print(f"Error processing entry {entry}: {e}")

# Display the resulting DataFrame
# Save the DataFrame to a CSV file
data.to_csv('sensor_data_800ms.csv', index=False)
#%%
import matplotlib.pyplot as plt

# Plot the accelerometer data
plt.figure(figsize=(10, 6))

# Filter data for the first 10 seconds
filtered_data = data[data['time'] <= data['time'].min() + 10]

plt.plot(filtered_data['time'], filtered_data['xAccl'], label='xAccl', color='r')
plt.plot(filtered_data['time'], filtered_data['yAccl'], label='yAccl', color='g')
plt.plot(filtered_data['time'], filtered_data['zAccl'], label='zAccl', color='b')

plt.title('Accelerometer Data Over First 10 Seconds')
plt.xlabel('Time (s)')
plt.ylabel('Acceleration')
plt.legend()
plt.grid()
plt.show()

