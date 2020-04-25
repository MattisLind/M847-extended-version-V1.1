# PDP8Remote

PDP8Remote is a software system in two parts. One is the firmware running on the AtMega328P microcontroller of the M847 Extended Edition board by Roland Huismann. This is called the PDP8Server. On the host there is a PDP8Client software that connects to the PDP8Server over serial port. The PDP8Server will take orders from the client. The orders can be for example LOAD ADDRESS and DEPOSIT. The PDP8Server expose all types operations that the M847 Extended Edition board is able to handle.

The PDP8Client program is able to parse BIN and RIM style of PDP-8 binaries into discrete operations orders whcih are sent to the PDP8Server for execution.

The communication takes place using a simple protocol with checksum checking and resending in case of checksum errors or not acknowledging receipt of a packet within time.

[![PDP8Remote in operation](http://img.youtube.com/vi/JFOzHFRHOXM/0.jpg)](http://www.youtube.com/watch?v=JFOzHFRHOXM)
