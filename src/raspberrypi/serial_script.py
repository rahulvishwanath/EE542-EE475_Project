#! python3

import serial

try:
    ser = serial.Serial(
       port = "/dev/ttyAMA0",
       baudrate = 115200,
       parity = serial.PARITY_NONE,
       stopbits = serial.STOPBITS_ONE,
       bytesize = serial.EIGHTBITS,
       timeout = 1)
    print("Setup successful")
    while (True):
        if ser.in_waiting > 0:
            data = ser.readline().decode().strip()
            print(f"Received: {data}")
except KeyboardInterrupt:
    print("Exiting...")

