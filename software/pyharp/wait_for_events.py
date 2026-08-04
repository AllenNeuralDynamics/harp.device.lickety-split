# /// script
# requires-python = ">=3.12"
# dependencies = [
#     "harp>=0.5.0rc1",
#     "numpy>=2.5.1",
# ]
#
# [tool.uv]
# prerelease = "allow"
# ///

from harp.device import Device, WhoAmI
from harp.protocol import ParsedHarpMessage
from harp.serial import open_serial_device
from lickety_split import LickState
import numpy as np

COM_PORT = "/dev/ttyACM0"

def print_lick_state(msg: ParsedHarpMessage[np.uint8]) -> None:
    print(f"new lick state: {msg.parsed}, time: {msg.timestamp}")

with open_serial_device(Device, port=COM_PORT, baudrate=1_000_000) as dev:
    print(dev.read(WhoAmI).parsed)
    lick_state_subscription = dev.subscribe(LickState, print_lick_state)

    input("Listening for events. Press Enter to stop.\n")
    lick_state_subscription.unsubscribe()
