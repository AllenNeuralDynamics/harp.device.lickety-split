# PIO SPI Streaming Program

## Delay Following Instructions
Instructions can introduce an additional cycle delay without costing program space by filling your program with "nops."
These delays are indicated by an integer surrounded by brackets at the end of an instruction.

## PIO Programming Semantics
PIO programs can either set output pins manually as one instruction or set them
concurrently with another instruction.
This is called "side-setting."
Side-setting is either compulsory (i.e: you *must* side-set a pin at every instruction) or optional (i.e: you have the option of side-setting a pin at any instructrion).
To do this requires that you flag that a specified number of pins are dedicated for side-setting and whether or not side-setting is optional at the top of your pio program.

Side-setting isn't a free lunch.
It will cost you bits from your delay instructions.
In other words, devoting one pin for side-setting cuts your maximum delay from 15 to 7 additional cycles.

## Starting Values
The scracth registers are [explicitly not set](https://github.com/raspberrypi/pico-feedback/issues/277) to any value at the start of a PIO program.
This means they can be set by the CPU *before launching the pio program*, and it's a useful feature to seed a program with a particular starting value.
To do so, you can use the `pio_sm_exec` function ([docs](https://www.raspberrypi.com/documentation/pico-sdk/hardware.html#rpip8ceec9883325e993eb43)).
You will need to additionally create the instruction code, but there are plenty of useful helper functions to do this in the Pico SDK.
These helper functions all take on the form `pio_encode_<instruction_name>` ([docs](https://www.raspberrypi.com/documentation/pico-sdk/hardware.html#pio_instructions)).
