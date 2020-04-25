# PDP8Remote

PDP8Remote is a software system in two parts. One is the firmware running on the AtMega328P microcontroller of the M847 Extended Edition board by Roland Huismann. This is called the PDP8Server. On the host there is a PDP8Client software that connects to the PDP8Server over serial port. The PDP8Server will take orders from the client. The orders can be for example LOAD ADDRESS and DEPOSIT. The PDP8Server expose all types operations that the M847 Extended Edition board is able to handle.

The PDP8Client program is able to parse BIN and RIM style of PDP-8 binaries into discrete operations orders whcih are sent to the PDP8Server for execution.

The communication takes place using a simple protocol with checksum checking and resending in case of checksum errors or not acknowledging receipt of a packet within time.

Below is a video showing PDP8Remote in operation. Klick on the picture to start playback of the video!

[![PDP8Remote in operation](http://img.youtube.com/vi/JFOzHFRHOXM/0.jpg)](http://www.youtube.com/watch?v=JFOzHFRHOXM)

## Software architecture

The software is partioned in the PDP8Server directory which include the code for the protocol. The PDP8Client also use the same files for the communiation protcol. For unit testing there is a separate test directory with a cople of test programs to run against the software. One usin a null modem cable and two serial ports to try to exercise as much of the code as possible. The other is more of a unit test for the protocol itself.

## Bugs

Occasionally I have seen that two or three words of the same value has been written after each other. It looks like it does the same DEPOSIT several times. I have not been able to track down it yet. But just to let any users know that it is a good preatice to verify that the last bytes transfered actually is written in to the correct addresses.

