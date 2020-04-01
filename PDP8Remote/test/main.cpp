
#ifdef LINUX
/*

  Implement wrapper for Arduino environment for
  Serial and Wifi so we could have the same code in Linux as well as ESP32.


  Compile on Macos: c++ -DLINUX -DMACOS -I. main.cpp Serial.cpp ../PDP8Server/protocol.cpp Wire.cpp
  Compile on Linux: c++ -DLINUX  -I. main.cpp Serial.cpp ../PDP8Server/protocol.cpp Wire.cpp

  Run as sudo to be able to open the serial port.

*/
#include "Serial.h"
#include "Wire.h"
#include <stdio.h>

typedef unsigned char byte;
typedef unsigned short word;

#define A0 0
#define A1 1
#define A2 2
#define A3 3
#define HIGH 1
#define LOW 0

#define OUTPUT 0
#define INPUT 1

#define B00000100 4
#define B11111101 253
#define B00000010 2
#define B00000001 1
#define B11111110 254
#define B11111011 251
#define B01000000 128

class Port {
  const char * designator;
  public:
  Port& operator= (unsigned char);
  Port& operator&= (unsigned char);
  Port& operator|= (unsigned char);
  Port (const char *);
};

Port::Port( const char * designator) {
  Port::designator = designator;
}

Port& Port::operator= (unsigned char byte) {
  fprintf (stderr, "%s = %02X\n", designator, byte);
  return *this;
}
Port& Port::operator&= (unsigned char byte) {
  fprintf (stderr, "%s |= %02X\n", designator, byte);
  return *this;
}
Port& Port::operator|= (unsigned char byte) {
  fprintf (stderr, "%s &= %02X\n", designator, byte);
  if (byte == 0x02) fprintf(stderr, "LOAD ADDRESS\n");
  if (byte == 0x04) fprintf(stderr, "START MEMORY\n");
  return *this;
}

class Port  PORTA("PORTA");
class Port  PORTB("PORTB");
class Port  PORTC("PORTC");
class Port  PORTD("PORTD");


void digitalWrite(word port, word value) {
  fprintf(stderr, "Wrote pin %d = %d \n", port, value);
}
word digitalRead(word port) {
  fprintf (stderr, "Reading in %d\n", port);
  return 0;
}
void pinMode(byte port, byte mode) {
  fprintf (stderr, "Setting pinMode pin %d to %d \n", port, mode);
}

void setup();
void loop();

unsigned long millis() {
  return 0L;
}


class Serial Serial ("/dev/ttyUSB1");

class Wire Wire;

int main (int argc, char ** argv) {

  setup();
  do {
    loop();
  }
  while (1);
  return 0;
}


#include "../PDP8Server/PDP8Control.ino"

#endif
