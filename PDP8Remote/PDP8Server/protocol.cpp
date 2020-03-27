


#include "protocol.h"

Protocol::Protocol ( void (* s) (char), void (* p) (char, char, char), void (* r ) ( void (Protocol::*) (), int ), void (* c) (int) ) {
   sendByte = s; processCmd = p; requestTimeout = r; commandDone = c;
   protocolState = 0;
}

void Protocol::sendNak() {
  sendByte(0xc0);
}

void Protocol::sendAck() {  
  sendByte(0xc1);
}

void Protocol::timeOut() {
  if (sendLen>0) {
    if (numResend < maxResend) {
      sendCommand();  // resend if timeout      
    } else {
      commandDone(1); 
    }
  }
}

void Protocol::sendCommand() {
  int i;
  for (i=0; i< sendLen; i++) {
    sendByte(sendBuf[i]);  
  }
  numResend++;
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
  sendingInProgress = true;
  sendBuf[0] = command & 0x3f;
  sum = 0x3f & command;
  for (i=0; i<len; i++) {
    sum+= 0x3f & data[i];
    sendBuf[i+1]=  (data[i] & 0x3f ) | 0x40;
  }
  sendBuf[i+1] = (~sum & 0x3f) | 0x80;
  sendLen = len+2;
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
      switch (data) {
        case 0x00: // NAK received
          if (numResend < maxResend) {
            sendCommand();  // resend if timeout      
          } else {
            commandDone(1); 
          }
          break;
        case 0x01: // ACK received
          break;
          sendLen=0;
          commandDone(0); 
      }
      break;
  }  
}
