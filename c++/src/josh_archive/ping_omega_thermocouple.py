import serial
import time

port = serial.Serial('COM6', 19200, timeout=1)
commands = ["VAL?", "T1?", "T2?", "T1-T2?", "ID?", "*IDN?", "TEMP?", "READ", "HELP", "DUMP"]

for cmd in commands:
    full_cmd = cmd + '\r'
    port.write(full_cmd.encode())
    time.sleep(0.5)
    response = port.read_all().decode(errors='ignore')
    print(f"> {cmd}\n{response}\n{'-'*20}")

port.close()
