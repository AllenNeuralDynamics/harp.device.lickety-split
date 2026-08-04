# This file was automatically generated and should not be edited directly.
# To make changes, edit the device metadata and regenerate the interface.

import enum
from typing import Any, ClassVar

import numpy as np
from harp.protocol import (
    AnonymousPayload,
    BitMask,
    PayloadType,
    RegisterBase,
    RegisterU8,
)
from harp.device import REGISTER_MAP as _CORE_REGISTER_MAP


class LickChannels(enum.IntFlag):
    """The channel of the lick detector."""

    CHANNEL0 = 0x1
    CHANNEL1 = 0x2


class LickStatePayload(AnonymousPayload[np.uint8]):
    """Represents the payload of the LickState register."""

    __value__: LickChannels = BitMask(enum=LickChannels)


class LickState(RegisterBase[LickChannels]):
    """Emits an event when the state of any lick detector changes. Value will be High when lick detected and Low otherwise."""

    address: ClassVar[int] = 32
    payload_type: ClassVar[PayloadType] = PayloadType.U8
    payload_class = LickStatePayload


class Channel0TriggerThreshold(RegisterU8):
    """Threshold value to detect the lick. Values below this threshold will be considered a detected lick."""

    address: ClassVar[int] = 33


class Channel0UntriggerThreshold(RegisterU8):
    """Threshold value to release the lick detection state. Values above this threshold will untrigger a detected lick."""

    address: ClassVar[int] = 34


REGISTER_MAP: dict[int, type[RegisterBase[Any]]] = {
    **_CORE_REGISTER_MAP,
    32: LickState,
    33: Channel0TriggerThreshold,
    34: Channel0UntriggerThreshold,
}

