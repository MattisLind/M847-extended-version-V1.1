

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
    txEven = false; // next packet is an odd packet.
  } 
  sum = 0x3f & command;
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
  printf ("processProtocol: tmp=%02X data=%02X ", 0xff & tmp, 0xff & data);
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
        printf("sum=%02X ", 0xff & sum);
        if ((0x3f & sum) == 0) {
          sendAck(rxEven);
	  if (rxEven) {
	    if ((command & 0x20) == 1) {
	      processCmd (command, dataBuf[0], dataBuf[1]);
	      rxEven = false;
	    }
	  } else {
	    if ((command & 0x20) == 0) {
	      processCmd (command, dataBuf[0], dataBuf[1]);           
	      rxEven = true;
	    }
	  }
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
      switch (data) {
        case 0x00: // NAK received
          printf ("numResend= %d", numResend);
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
	  }
        case 0x01: // odd ACK received
	  if (!txEven) { // if we sent an odd packet we should receive an odd ack
	    sendLen=0;
	    commandDone(0);
	    sendingInProgress=false;
	  }

          break; 
      }
      break;
  }  
}
