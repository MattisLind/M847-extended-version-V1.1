


#include "protocol.h"

Protocol::Protocol ( void (* s) (char), void (* p) (char, char, char)) {
   sendByte = s; processCmd = p;
}

void Protocol::sendNak() {
  sendByte(0xc0);
}

void Protocol::sendAck() {  
  sendByte(0xc1);
}


void Protocol::doCommand(char command, char * data, char len) {
  char sum, i;
  sendByte (command & 0x3f);
  sum = 0x3f & command;
  for (i=0; i<len; i++) {
    sum+= 0x3f & data[i];
    sendByte ((data[i] & 0x3f ) | 0x40);
  }
  sendByte ( (~sum & 0x3f) | 0x80);
}

void Protocol::processProtocol(char tmp) {
  data = tmp & 0x3f;
  switch (tmp & 0xc0) {
    case 0x00:
      // We got a command
      protocolState = 1;
      command = data;
      sum = data;
      cnt = 0;
      break;
    case 0x40:
      // we got a data byte
      if (protocolState == 1) {
        if (cnt < 2) { 
          dataBuf[cnt] = data;
          sum += data;
        } else {
          protocolState = 0;
          sendNak();
        }          
      } else {
        protocolState = 0;
        sendNak();        
      }
      break;
    case 0x80:
      if (protocolState == 1) {
        sum += data;
        if (sum == 0) {
          sendAck();  
          processCmd (command, dataBuf[0], dataBuf[1]);           
        } else {
          protocolState = 0;
          sendNak();                    
        }
      } else {
        protocolState = 0;
        sendNak();         
      }
      break;
    case 0xc0:
      break;
  }  
}
