#!/usr/bin/env python3
from pyharp.device import Device, DeviceMode
from pyharp.messages import HarpMessage
from pyharp.messages import MessageType
from pyharp.messages import CommonRegisters as Regs
from struct import *
import os
from time import sleep, perf_counter


# Open the device and print the info on screen
# Open serial connection and save communication to a file
if os.name == 'posix': # check for Linux.
    #device = Device("/dev/harp_device_00", "ibl.bin")
    device = Device("/dev/ttyACM0", "ibl.bin")
else: # assume Windows.
    device = Device("COM95", "ibl.bin")

# Read encoder and torque raw measurements.
try:
        on_threshold = device.send(HarpMessage.ReadU8(33).frame)
        off_threshold = device.send(HarpMessage.ReadU8(34).frame)
        print(f"on threshold: {on_threshold}")
        print(f"off_threshold: {off_threshold}")
        print()
finally:
    # Close connection
    device.disconnect()
