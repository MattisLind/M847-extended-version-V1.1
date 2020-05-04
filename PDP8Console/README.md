# PDP8Console

The PDP8Console is a quick tool I came up with during the debugging of the M847 Extended Edition hardware where the BRK DATA signal misbahved. It turned out to be a difference between different CPU types. But I though that the tool might be nice to have.

![PDP8Console](https://i.imgur.com/YOpf1co.png)

It allow you to do all the operations that the M847 Extended Edition board is able to perform dirctly from command line via a serial port into the board. Notably it doesn't allow you to examine the memory neither do it allow view internal registers and stae since it requires mor wires and input gatig. There is also a problem that it for some operations interact with how a real programmers console is using the buses.

## Libraries taken from others

These libraies are taken from others. I hav enot written them myself.

The EEPROM_24XX1025 library is written by Thomas Backman [github link](https://github.com/exscape/electronics/tree/master/Arduino/Libraries/EEPROM/EEPROM_24XX1025)

The I2C16 library is originally written but modified for the EEPROM_24XX1025 library by Thomas Backman. [github link](https://github.com/exscape/electronics/tree/master/Arduino/Libraries/EEPROM/I2C16).