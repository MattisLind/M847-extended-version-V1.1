#include "Wire.h"
#include <stdio.h>

void Wire::endTransmission() {
  fprintf (stderr, "Wire::endTransmission()\n");
}

void Wire::beginTransmission(int data) {
  fprintf (stderr, "Wire::beginTransmission()\n");
}

void Wire::begin() {
  fprintf (stderr, "Wire::begin()\n"); 
}

void Wire::write(int data) {
  fprintf (stderr, "Wire::write(%02X)\n", 0xff&data);
}
