import serial
import time

ser = serial.Serial('COM10', 9600, timeout=1)  # Adjust COM port and baud rate
# print(repr(ser.readline()))
response= ser.read_until(b'\r\n')
#print(type(response), response)
dict_data = {}

while response != b'':
    response = ser.read_until(b'\r\n')
    #print(response == b'', type(response), response )
    time.sleep(5)
    data_array = str(response)[2:].split("  ")
    #print(data_array)
    for i in data_array:
        index = i.find('\\') 
        if i != '':
            data = i[:index if index != -1 else len(i) ]
            key = data[:data.find(':')]
            #print(key, data)
            if key in dict_data:
                dict_data[key].append(data[data.find(':') + 1:]) 
            dict_data.setdefault(key, [])
            
    print(dict_data)