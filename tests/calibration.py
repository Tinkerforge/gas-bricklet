#!/usr/bin/env python
# -*- coding: utf-8 -*-

HOST = "localhost"
PORT = 4223
UID = "XYZ" 

from tinkerforge.ip_connection import IPConnection
from tinkerforge.bricklet_gas import BrickletGas

if __name__ == "__main__":
    ipcon = IPConnection() # Create IP connection
    gas = BrickletGas(UID, ipcon) # Create device object

    ipcon.connect(HOST, PORT) # Connect to brickd
    # Don't use device before ipcon is connected

    cal = gas.get_calibration()
    print("Calibration Before: " + str(cal))

    gas.set_calibration(107292, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 290)

    cal = gas.get_calibration()
    print("Calibration After: " + str(cal))

    raw_input("Press key to exit\n") # Use input() in Python 3
    ipcon.disconnect()
