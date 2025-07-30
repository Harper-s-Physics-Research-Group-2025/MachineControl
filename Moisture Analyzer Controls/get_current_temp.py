import serial
import time

#1. Set COM port and baud rate for serial communication
ser = serial.Serial('COM10', 9600, timeout=1)  
# print(repr(ser.readline()))

#2. Read bit data from the Moisture analyzer
response= ser.read_until(b'\r\n') 

#3. Create an object for storing parsed data
dict_data = {} 

#4. The parsing process
data_array = str(response)[2:].split("  ")
# print(data_array, response)
if(response != b''):
    for i in data_array:
        index = i.find('\\') 
        if i != '':
            data = i[:index if index != -1 else len(i) ]
            key = data[:data.find(':')]
            #print(key, data)
            if key in dict_data:
                dict_data[key].append(data[data.find(':') + 1:].strip()) 
            dict_data.setdefault(key, [data[data.find(':') + 1:].strip()])
            
    #5. Print out moisture content
    #print(dict_data)
    print(dict_data['Current TEMP'][0][:])
else:
    raise Exception("The Moisture Analyzer is not operating right now")

#6. Close  serial port
ser.close()