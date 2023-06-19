## RP2040

### Oscillator
The RP2040 accepts either a crystal or CMOS-level clock input
I opted for a precision oscillator instead of a vanilla crystal.
To preserve USB Boot functionality (useful!), I kept the frequency at 12MHz (datasheet pg 12).

This oscillator requires a 0.01-0.1uF bypass capacitor placed very close to the pins.
An example layout is provided in the datasheet.


## USB Isolator
To disconnect the host PC's ground from this device's ground, I added a Full-Speed capable USB isolator.

The downside is that this device requires external power before it will communicate with the host pc.


## 
