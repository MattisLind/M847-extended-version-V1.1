

#include <stdio.h>
#include "protocol.h"

Protocol::Protocol ( void (* s) (char), void (* p) (char, char, char), void (* r ) ( void (Protocol::*) (), int ), void (* c) (int) ) {
   sendByte = s; processCmd = p; requestTimeout = r; commandDone = c;
   protocolState = 0;
   txEven = true;
   rxEven = true;
}

void Protocol::sendNak() {
  sendByte(0xc0);
}

void Protocol::sendAck(bool even) {  
  sendByte(0xc1 | (even?0x20:0x00));
}

void Protocol::timeOut() {
  if (sendLen>0) {
    if (numResend < maxResend) {
      sendCommand();  // resend if timeout      
    } else {
      commandDone(1); 
      sendingInProgress=false;
    }
  }
}

void Protocol::sendCommand() {
  int i;
  numResend++;
  for (i=0; i< sendLen; i++) {
    sendByte(sendBuf[i]);  
  }
}

bool doCommand (char command, char msb, char lsb ,int maxRes) {
  char data[2];
  data[0] = msb;
  data[1] = lsb;
  doCommand(command, data, 2, maxRes); 
}

bool Protocol::doCommand(char command, char * data, char len, int maxRes) {
  char sum, i;
  if (len > 2) { 
    return false;
  }
  if (sendingInProgress) {
    return false; 
  }
  maxResend = maxRes;
  numResend = -1;
  sendingInProgress = true;
  sendBuf[0] = command & 0x1f;
  if (txEven) {
    sendBuf[0] |= 0x20; // set the even bit
  }
  sum = 0x3f & sendBuf[0];
  for (i=0; i<len; i++) {
    sum+= 0x3f & data[i];
    sendBuf[i+1]=  (data[i] & 0x3f ) | 0x40;
  }
  sendBuf[i+1] = (-sum & 0x3f) | 0x80;
  sendLen = len+2;
  sendCommand();
  requestTimeout(&Protocol::timeOut, 500);
  return true;
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
          cnt++;
          sum += data;
        } else {
          protocolState = 0;
        }          
      } else {
        protocolState = 0;
      }
      break;
    case 0x80:
      if (protocolState == 1) {
        sum += data;
        if ((0x3f & sum) == 0) {
	  if ((command & 0x20) == 0x20) {
	    sendAck(true);
	  } else {
	    sendAck(false);
	  }
	  if ((rxEven && (command & 0x20)) || (!rxEven && ((command & 0x20) == 0x00) )) {
	    processCmd (0x1f & command, dataBuf[0], dataBuf[1]);
	    rxEven = !rxEven;
	  }
        } else {
          protocolState = 0;
	  sendNak();
        }
      } else {
        protocolState = 0;
      }
      break;
    case 0xc0:
      switch (data) {
        case 0x00: // NAK received
          if (numResend < maxResend) {
            sendCommand();  // resend if timeout      
          } else {
            commandDone(1);
	    sendingInProgress=false;
          }
          break;
        case 0x21: // even ACK received
	  if (txEven) { // If we sent an even pack we should receive an even ack
	    sendLen=0;
	    commandDone(0);
	    sendingInProgress=false;
	    txEven = false;
	  }
	  break;
        case 0x01: // odd ACK received
	  if (!txEven) { // if we sent an odd packet we should receive an odd ack
	    sendLen=0;
	    commandDone(0);
	    sendingInProgress=false;
	    txEven = true;
	  }

          break; 
      }
      break;
  }  
}
