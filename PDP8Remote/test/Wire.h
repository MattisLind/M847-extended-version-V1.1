#ifndef _WIRE_H_
#define _WIRE_H_


class Wire {
  
 public:
  void beginTransmission(int);
  void write(int);
  void endTransmission();
  void begin ();
};


#endif
