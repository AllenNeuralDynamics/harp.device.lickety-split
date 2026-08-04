# /// script
# requires-python = ">=3.12"
# dependencies = [
#     "harp>=0.5.0rc1",
# ]
#
# [tool.uv]
# prerelease = "allow"
# ///

from harp.device import Device, WhoAmI
from harp.serial import open_serial_device
from lickety_split import Channel0TriggerThreshold, Channel0UntriggerThreshold

COM_PORT = "/dev/ttyACM0"

# Read raw thresholds.
with open_serial_device(Device, port=COM_PORT, baudrate=1_000_000) as dev:
    print(dev.read(WhoAmI).parsed)
    trigger_threshold = dev.read(Channel0TriggerThreshold).parsed
    untrigger_threshold = dev.read(Channel0UntriggerThreshold).parsed
    print(f"Trigger threshold: {trigger_threshold}, "
          f"Untrigger_threshold: {untrigger_threshold}")
