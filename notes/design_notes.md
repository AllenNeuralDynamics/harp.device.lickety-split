## RP2040

### Oscillator
The RP2040 accepts either a crystal or CMOS-level clock input
I opted for a precision oscillator instead of a vanilla crystal.
To preserve USB Boot functionality (useful!), I kept the frequency at 12MHz (datasheet pg 12).

This [oscillator is shared](https://electronics.stackexchange.com/questions/27989/sharing-an-oscillator-between-two-ics) with the AD9833, and traces on the board are deliberately kept short.
(It would also have been valid to produce a 12-25MHz signal from the RP2040 to drive the AD9833.)

This oscillator requires a 0.01-0.1uF bypass capacitor placed very close to the pins.
An example layout is provided in the datasheet.


## USB Isolator
To disconnect the host PC's ground from this device's ground, I added a Full-Speed capable USB isolator.

The downside is that this device cannot be powered via USB from the host pc.


## Analog Front-End
The analog front end consists of a buffer followed by a bandpass filter, an amplification stage, and a positive voltage offset.
Because the excitation signal has high input impedance, the buffer op-amp was chosen to have low input capacitance since any capacitance will effectively form a voltage divider at 100KHz, reducing the input signal amplitude.
The bandpass filter removes unwanted signals (esp. DC and 50-or-60Hz noise).

The amplification stage puts the signal in the range readable by a 3.3V ADC, and the final voltage divider produces an offset to put the signal in the all-positive range.

Additionally, a low-impedance copy of the signal from the output of the bandpass filter is fed through a buffer and sent back along the cable shield to guard the signal.
(See the section below on signal guarding for the rationale.)
Because the bandpass filter introduces a 180-deg phase shift, the buffer inverts the signal again, such that the input signal and guard signal are both in phase.

# PCB Layout

## Layers

## Guarding

# Appendix

## Guarding and Shielding Basics
Because the sensing element carries the excitation signal with such a high impedance (1MOhm), the parasitic capacitance of the cable cannot be ignored.

Left unaccounted for, the 1MOhm series resistor and cable capacitance form a lowpass filter with a cutoff frequency in the 1-10KHz range.
(Standard coax cable has a parasitic capacitance of 7-30pF per foot.)
This filter attenuates the excitation signal, making readings at the far end of the cable impractical.

To remove this unwanted filter, the cable capacitance must be actively cancelled.
This can be done with a coax cable, where the inner conductor carries the high-impedance signal and the outer conductor carries a low-impedance copy of that signal.
Since the inner and outer conductors are driven to the same potential, the effects of the parasitic capacitor are removed.

This trick is called *guarding*, and it removes the parasitic effects of signal cables.
Guarding does not provide shielding, however.
And a guarded setup is susceptible to instability issues from external noise sources.
Consequently, this setup must additionally be shielded.

This entire setup can be accomplished practically with triaxial cable ([example datasheet](https://www.te.com/commerce/DocumentDelivery/DDEController?Action=srchrtrv&DocNm=7528A5314&DocType=Customer+Drawing&DocLang=English)) or coaxial cable that has been additionally shielded with braided shield sleeve.
Here the inner braid carries of low-impedance copy of the signal, and the outer braid is connected to ground at one end.


## References
* [All About USBC: Example Circuits](https://hackaday.com/2023/08/07/all-about-usb-c-example-circuits/)
  * contains schematic for a device to display itself as a full speed USB device.
* [Raspberry Pi Pico Datasheet](https://datasheets.raspberrypi.com/pico/pico-datasheet.pdf)
  * contains schematic for minimal board design.
* [AD9833 Signal Generator First Steps Part 1](https://daumemo.com/diy-ad9833-signal-generator-first-steps-part-1/)
  * Contains effect of load resistance on output voltage.
* [Hardware Design with the RP2040](https://datasheets.raspberrypi.com/rp2040/hardware-design-with-rp2040.pdf)
* [pico-examples: multicore\_runner\_queue.c](https://github.com/raspberrypi/pico-examples/blob/develop/multicore/multicore_runner_queue/multicore_runner_queue.c)
* [AN-347](https://www.analog.com/media/en/technical-documentation/application-notes/41727248AN_347.pdf)
* [Capacitor Impedance Calculator](https://www.allaboutcircuits.com/tools/capacitor-impedance-calculator/)
* [Pi Pico Pinout](https://www.raspberrypi-spy.co.uk/wp-content/uploads/2021/01/raspberry_pi_pico_pinout.png)
* [Reduced Printed Circuit Board Emissions with Low-Noise Design Practices](https://ntrs.nasa.gov/api/citations/20120009353/downloads/20120009353.pdf)
* [PCB Plane Capacitance](https://www.intel.com/content/www/us/en/docs/programmable/683073/current/plane-capacitance.html)
* [ADA4530 Datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/ADA4530-1.pdf)
  * contains useful info on guarding ring/plane pcb layout for high impedance signals.
* [Application Note 241 Working with High Impedance Op Amps](https://ti.com/lit/an/snoa664/snoa664.pdf?ts=1689978784221&ref_url=https%253A%252F%252Fwww.ti.com%252https://meettechniek.info/passive/capacitance.htmlFproduct%252FLM3900)
* [INA116 Datasheet: Ultra-Low Input Bias Current Instrumentation Amplifier](https://www.ti.com/lit/ds/symlink/ina116.pdf)
* [ADA4530-1R-EBZ User Guide](https://www.analog.com/media/en/technical-documentation/user-guides/ADA4530-1R-EBZ_UG-865.pdf)
* [Designing Gain and Offset in Thirty Seconds](https://www.ti.com/lit/an/sloa097/sloa097.pdf?ts=1693767373611)
* Design femptoampere circuits with low leakage [Part 1](https://www.edn.com/design-femtoampere-circuits-with-low-leakage-part-one/), [Part 2](https://www.edn.com/design-femtoampere-circuits-with-low-leakage-part-2-component-selection/), [Part 3](https://www.edn.com/design-femtoampere-circuits-with-low-leakage-part-3-low-current-design-techniques/)
* [Tektronix Low Level Measurements Handbook](https://www.tek.com/en/documents/product-article/keithley-low-level-measurements-handbook---7th-edition#C2section0)
* [Measuring Capacitance and ESR](https://meettechniek.info/passive/capacitance.html)
* Texas Instruments Video for OpAmps Driving [Capacitive Loads](https://www.youtube.com/watch?v=ER2x0djZ7oU)
* [Stack Exchange: How to Connect USB Connector Shield](https://electronics.stackexchange.com/questions/4515/how-to-connect-usb-connector-shield)
* [Stack Exchange: Measuring Feline Capacitance](https://electronics.stackexchange.com/questions/152090/measuring-feline-capacitance)
* [Stack Exchange: Explanation about OpAmp Bias Resistor and Bootstrapping](https://electronics.stackexchange.com/questions/586839/explanation-about-op-amp-bias-resistor-and-bootstrapping)
  * contains info on how to bias an AC input signal to ground without reducing input impedance.
* [Stack Exchange: How to Amplify and Offset the Voltage in an OpAmp](https://electronics.stackexchange.com/questions/93423/how-to-amplify-and-offset-the-voltage-in-an-opamp)
  * use a reference voltage and a differential amplifier to offset an input signal.
* [How to make arbitrary shape footprints in KiCAD](https://imgur.com/a/cwRWZnW)
### Reference Designs
* [ADS7049 Evaluation Module](https://www.ti.com/lit/ug/sbau382a/sbau382a.pdf?ts=1693010385528&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FADS7029-Q1) Reference Design
* * [Demo Manual DC2414A](https://www.analog.com/media/en/technical-documentation/user-guides/DC2414AF.PDF)
  * LTC6268 evaluation board
* [LT3030 Demo Circtuit](https://www.analog.com/media/en/technical-documentation/eval-board-schematic/DC1855A-2-SCH.pdf)
* [Programmable Gain Amplifiers](https://www.analog.com/media/en/training-seminars/design-handbooks/system-applications-guide/Section3.pdf)
  * Figure 3.6
