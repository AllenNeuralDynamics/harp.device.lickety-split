## RP2040

### Oscillator
The RP2040 accepts either a crystal or CMOS-level clock input
I opted for a precision oscillator instead of a vanilla crystal.
To preserve USB Boot functionality (useful!), I kept the frequency at 12MHz (datasheet pg 12).

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
* Design femptoampere circuits with low leakage [Part 1](https://www.edn.com/design-femtoampere-circuits-with-low-leakage-part-one/), [Part 2](https://www.edn.com/design-femtoampere-circuits-with-low-leakage-part-2-component-selection/), [Part 3](https://www.edn.com/design-femtoampere-circuits-with-low-leakage-part-3-low-current-design-techniques/)
* [Tektronix Low Level Measurements Handbook](https://www.tek.com/en/documents/product-article/keithley-low-level-measurements-handbook---7th-edition#C2section0)
* [Demo Manual DC2414A](https://www.analog.com/media/en/technical-documentation/user-guides/DC2414AF.PDF)
  * LTC6268 evaluation board
* [Measuring Capacitance and ESR](https://meettechniek.info/passive/capacitance.html)
* Texas Instruments Video for OpAmps Driving [Capacitive Loads](https://www.youtube.com/watch?v=ER2x0djZ7oU)
* [Stack Exchange: Measuring Feline Capacitance](https://electronics.stackexchange.com/questions/152090/measuring-feline-capacitance)
* [Stack Exchange: Explanation about OpAmp Bias Resistor and Bootstrapping](https://electronics.stackexchange.com/questions/586839/explanation-about-op-amp-bias-resistor-and-bootstrapping)
  * contains info on how to bias an AC input signal to ground without reducing input impedance.
