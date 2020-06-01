# adrf6806
Analog Devices ADRF6806 control tool

This is a Linux command line tool that allows setting the PLL and mixer settings of the ADRF6806 demodulator chip, e.g. allowing to directly specify a tuning frequency. Hardware access takes place by bitbanging either an FTDI dongle (via libftdi), the Linux spidev or Raspberry Pi GPIOs (via WiringPi). Call the makefile either with "make adrf6806_ftdi" for the FTDI version, "make adrf8606_spidev" for spidev access or "make adrf6806" for the RPi version. Adapt hardware config in config_ftdi.h, config_spidev.h or config_rpi.h accordingly, if needed.

Contains a slightly modified copy of frap.c by David Eppstein and Arno Formella to find the best frac/mod ratio for the PLL's fractional mode.
