/*

 Compile using c++ protocolUnitTest.cpp ../PDP8Server/protocol.cpp RingBuffer.cpp -o protocolUnitTest

 */

#include <assert.h>
#include <stdio.h>
#include "../PDP8Server/protocol.h"
#include "RingBuffer.h"

#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

void processACmd(char, char, char);
void processBCmd(char, char, char);
void aToB(char);
void bToA(char);
typedef  void (Protocol::* TimeoutFn)();
void handleTimeout ( void (Protocol::* ) (), int);
void commandADone(int);
void commandBDone(int);

class RingBuffer aToBBuffer; 
class RingBuffer bToABuffer; 

int test;


class Protocol protocolA(aToB, processACmd, handleTimeout, commandADone);

class Protocol protocolB(bToA, processBCmd, handleTimeout, commandBDone);


void processACmd(char command, char msb, char lsb) {
  printf ("A Received command %02X msb=%02X lsb=%02X", 0xff & command, 0xff & msb, 0xff & lsb);
}

char gCommand;
char gMsb;
char gLsb;
int processBCmdCnt;

void processBCmd(char command, char msb, char lsb) {
  printf ("B Received command %02X msb=%02X lsb=%02X\n", 0xff & command, 0xff & msb, 0xff & lsb);
  processBCmdCnt++;
  printf("Incremented processBCmdCnt = %d\n", processBCmdCnt);
  gCommand = command;
  gMsb = msb;
  gLsb = lsb;
  
}

int byteCnt;

void aToB(char data) {
  printf ("aToB:%02X \n", 0xff & data);

  switch (test) {
  case 0:
  case 1:
  case 4:
  case 5:
  case 100:
    aToBBuffer.writeBuffer(data);
    break;
  case 2:
    if (byteCnt==2) {
      data |= 0x20;
    }
    aToBBuffer.writeBuffer(data);
    byteCnt++;
    break;
  case 3:
    if (byteCnt!=2) {
      aToBBuffer.writeBuffer(data);
    }
    byteCnt++;
    break;
  case 6:
    if (byteCnt==0) {
      aToBBuffer.writeBuffer(data^0x80);
    } else {
      aToBBuffer.writeBuffer(data);
    }
    byteCnt++;
    break;
  case 7:
    if (byteCnt==1) {
      aToBBuffer.writeBuffer(data^0x80);
    } else {
      aToBBuffer.writeBuffer(data);
    }
    byteCnt++;
    break;
  case 8:
    if (byteCnt==3) {
      aToBBuffer.writeBuffer(data^0x80);
    } else {
      aToBBuffer.writeBuffer(data);
    }
    byteCnt++;
    break;
  }
}

void bToA(char data) {
  printf ("bToA:%02X \n", 0xff & data);

  switch (test) {
  case 0:
  case 1:
  case 2:
  case 3:
  case 100:
  case 6:
  case 7:
  case 8:
    bToABuffer.writeBuffer(data);
    break;
  case 4:
    if (byteCnt != 0) {
      bToABuffer.writeBuffer(data);
    }
    byteCnt++;
    break;
  case 5:
    if (byteCnt != 0) {
      bToABuffer.writeBuffer(data);
    } else {
      bToABuffer.writeBuffer(data^0x20);
    }
    byteCnt++;
    
  }


}

TimeoutFn timeoutFn;
int timeout;

void handleTimeout ( void (Protocol::* t) (), int ms) {
  timeoutFn = t;
  timeout = ms;
  printf("Timeout called.\n");
}


int commandADoneV;

void commandADone(int status) {
  printf("A DONE statue =%d\n", status); 
  commandADoneV++;
}

void commandBDone(int status) {
  printf("B DONE\n"); 
}

char testData [] =  {3,4};

int main () {
  bool res;

  aToBBuffer.initBuffer();
  bToABuffer.initBuffer(); 
 

  printf ("\n-----------------------------------------\n");
  test = 0;
  printf ("TEST %d A message with just the command.\n", test);
  gCommand = 0;
  gMsb = 0;
  gLsb = 0;
  commandADoneV=0;
  processBCmdCnt=0;
  res = protocolA.doCommand(1, NULL, 0, 1);

  while (!aToBBuffer.isBufferEmpty() || !bToABuffer.isBufferEmpty()) {
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
  }


  assert(gCommand == 1);
  assert(commandADoneV == 1);
  assert(res == true);
  assert(processBCmdCnt == 1);

  printf ("\n-----------------------------------------\n");
  test = 1;
  printf ("TEST %d A message with two data bytes. \n", test);
  gCommand = 0;
  gMsb = 0;
  gLsb = 0;
  processBCmdCnt=0;
  commandADoneV=0;
  res = protocolA.doCommand(2,testData , 2, 1);

  while (!aToBBuffer.isBufferEmpty() || !bToABuffer.isBufferEmpty()) {
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
  }


  assert(gCommand == 2);
  assert(gMsb==3);
  assert(gLsb==4);
  assert(commandADoneV == 1);
  assert(processBCmdCnt == 1);
  assert(res == true);

  printf ("\n-----------------------------------------\n");
  test = 100;
  printf ("TEST %d A message with just the command.\n", test);
  gCommand = 0;
  gMsb = 0;
  gLsb = 0;
  commandADoneV=0;
  processBCmdCnt=0;
  res = protocolA.doCommand(1, NULL, 0, 1);

  while (!aToBBuffer.isBufferEmpty() || !bToABuffer.isBufferEmpty()) {
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
  }


  assert(gCommand == 1);
  assert(commandADoneV == 1);
  assert(res == true);
  assert(processBCmdCnt == 1);


  printf ("\n-----------------------------------------\n");
  test = 2;  // mangle one byte
  gCommand = 0;
  gMsb = 0;
  gLsb = 0;
  byteCnt=0;
  processBCmdCnt=0;
  commandADoneV=0;
  printf ("TEST %d One bit in the message is wrong. \n", test);
  res = protocolA.doCommand(2,testData , 2, 1);

  while (!aToBBuffer.isBufferEmpty() || !bToABuffer.isBufferEmpty()) {
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
  }



  assert(res == true);
  assert(gCommand == 2);
  assert(gMsb==3);
  assert(gLsb==4);
  assert(commandADoneV == 1);
  assert(processBCmdCnt == 1);


  printf ("\n-----------------------------------------\n");
  test = 100;
  printf ("TEST %d A message with just the command.\n", test);
  gCommand = 0;
  gMsb = 0;
  gLsb = 0;
  commandADoneV=0;
  processBCmdCnt=0;
  res = protocolA.doCommand(1, NULL, 0, 1);

  while (!aToBBuffer.isBufferEmpty() || !bToABuffer.isBufferEmpty()) {
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
  }



  assert(gCommand == 1);
  assert(commandADoneV == 1);
  assert(res == true);
  assert(processBCmdCnt == 1);


  printf ("\n-----------------------------------------\n");
  test = 3;  // lose one byte
  gCommand = 0;
  gMsb = 0;
  gLsb = 0;
  byteCnt=0;
  processBCmdCnt=0;
  commandADoneV=0;
  printf ("TEST %d One byte in the message is lost. \n", test);
  res = protocolA.doCommand(2,testData , 2, 1);

  while (!aToBBuffer.isBufferEmpty() || !bToABuffer.isBufferEmpty()) {
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
  }



  assert(gCommand == 2);
  assert(gMsb==3);
  assert(gLsb==4);
  assert(commandADoneV == 1);
  assert(processBCmdCnt == 1);
  assert(res == true);

  printf ("\n-----------------------------------------\n");
  test = 100;
  printf ("TEST %d A message with just the command.\n", test);
  gCommand = 0;
  gMsb = 0;
  gLsb = 0;
  commandADoneV=0;
  processBCmdCnt=0;
  res = protocolA.doCommand(1, NULL, 0, 1);

  while (!aToBBuffer.isBufferEmpty() || !bToABuffer.isBufferEmpty()) {
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
  }


  assert(gCommand == 1);
  assert(commandADoneV == 1);
  assert(res == true);
  assert(processBCmdCnt == 1);


  printf ("\n-----------------------------------------\n");
  test = 4;  // lose the ACK
  gCommand = 0;
  gMsb = 0;
  gLsb = 0;
  byteCnt=0;
  processBCmdCnt=0;
  commandADoneV=0;
  printf ("TEST %d The first ACK is lost\n", test);
  res = protocolA.doCommand(2,testData , 2, 1);

  while (!aToBBuffer.isBufferEmpty() || !bToABuffer.isBufferEmpty()) {
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
  }


  assert(res == true);
  CALL_MEMBER_FN(protocolA,timeoutFn)();

  while (!aToBBuffer.isBufferEmpty() || !bToABuffer.isBufferEmpty()) {
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
  }


  assert(gCommand == 2);
  assert(gMsb==3);
  assert(gLsb==4);
  assert(commandADoneV == 1);
  assert(processBCmdCnt == 1);

  printf ("\n-----------------------------------------\n");
  test = 100;
  printf ("TEST %d A message with just the command.\n", test);
  gCommand = 0;
  gMsb = 0;
  gLsb = 0;
  commandADoneV=0;
  processBCmdCnt=0;
  res = protocolA.doCommand(1, NULL, 0, 1);

  while (!aToBBuffer.isBufferEmpty() || !bToABuffer.isBufferEmpty()) {
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
  }

  assert(gCommand == 1);
  assert(commandADoneV == 1);
  assert(res == true);
  assert(processBCmdCnt == 1);


  printf ("\n-----------------------------------------\n");
  test = 5;  // An ack becomes an oposite ack
  gCommand = 0;
  gMsb = 0;
  gLsb = 0;
  byteCnt=0;
  processBCmdCnt=0;
  commandADoneV=0;
  printf ("TEST %d an ack becomes an oposite ack.\n", test);
  res = protocolA.doCommand(2,testData , 2, 1);

  while (!aToBBuffer.isBufferEmpty() || !bToABuffer.isBufferEmpty()) {
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
  }


  assert(res == true);
  CALL_MEMBER_FN(protocolA,timeoutFn)();

  while (!aToBBuffer.isBufferEmpty() || !bToABuffer.isBufferEmpty()) {
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
  }

  assert(gCommand == 2);
  assert(gMsb==3);
  assert(gLsb==4);
  assert(commandADoneV == 1);
  assert(processBCmdCnt == 1);


  printf ("\n-----------------------------------------\n");
  test = 100;
  printf ("TEST %d A message with just the command.\n", test);
  gCommand = 0;
  gMsb = 0;
  gLsb = 0;
  commandADoneV=0;
  processBCmdCnt=0;
  res = protocolA.doCommand(1, NULL, 0, 1);

  while (!aToBBuffer.isBufferEmpty() || !bToABuffer.isBufferEmpty()) {
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
  }


  assert(gCommand == 1);
  assert(commandADoneV == 1);
  assert(res == true);
  assert(processBCmdCnt == 1);

  printf ("\n-----------------------------------------\n");
  test = 6;  // A high bit is changed in the command word
  gCommand = 0;
  gMsb = 0;
  gLsb = 0;
  byteCnt=0;
  processBCmdCnt=0;
  commandADoneV=0;
  printf ("TEST %d a high bit is changed in the command word.\n", test);
  res = protocolA.doCommand(2,testData , 2, 1);
  assert(res == true);

  while (!aToBBuffer.isBufferEmpty() || !bToABuffer.isBufferEmpty()) {
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
  }


  CALL_MEMBER_FN(protocolA,timeoutFn)();

  while (!aToBBuffer.isBufferEmpty() || !bToABuffer.isBufferEmpty()) {
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
  }


  assert(gCommand == 2);
  assert(gMsb==3);
  assert(gLsb==4);
  assert(commandADoneV == 1);
  assert(processBCmdCnt == 1);


  printf ("\n-----------------------------------------\n");
  test = 100;
  printf ("TEST %d A message with just the command.\n", test);
  gCommand = 0;
  gMsb = 0;
  gLsb = 0;
  commandADoneV=0;
  processBCmdCnt=0;
  res = protocolA.doCommand(1, NULL, 0, 1);

  while (!aToBBuffer.isBufferEmpty() || !bToABuffer.isBufferEmpty()) {
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
  }


  assert(gCommand == 1);
  assert(commandADoneV == 1);
  assert(res == true);
  assert(processBCmdCnt == 1);





  printf ("\n-----------------------------------------\n");
  test = 7;  // A high bit is changed in the command word
  gCommand = 0;
  gMsb = 0;
  gLsb = 0;
  byteCnt=0;
  processBCmdCnt=0;
  commandADoneV=0;
  printf ("TEST %d a high bit is changed in the data word.\n", test);
  res = protocolA.doCommand(2,testData , 2, 1);
  assert(res == true);

  while (!aToBBuffer.isBufferEmpty() || !bToABuffer.isBufferEmpty()) {
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
  }


  CALL_MEMBER_FN(protocolA,timeoutFn)();

  while (!aToBBuffer.isBufferEmpty() || !bToABuffer.isBufferEmpty()) {
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
  }


  assert(gCommand == 2);
  assert(gMsb==3);
  assert(gLsb==4);
  assert(commandADoneV == 1);
  assert(processBCmdCnt == 1);


  printf ("\n-----------------------------------------\n");
  test = 100;
  printf ("TEST %d A message with just the command.\n", test);
  gCommand = 0;
  gMsb = 0;
  gLsb = 0;
  commandADoneV=0;
  processBCmdCnt=0;
  res = protocolA.doCommand(1, NULL, 0, 1);

  while (!aToBBuffer.isBufferEmpty() || !bToABuffer.isBufferEmpty()) {
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
  }


  assert(gCommand == 1);
  assert(commandADoneV == 1);
  assert(res == true);
  assert(processBCmdCnt == 1);



  printf ("\n-----------------------------------------\n");
  test = 8;  // A high bit is changed in the sum word
  gCommand = 0;
  gMsb = 0;
  gLsb = 0;
  byteCnt=0;
  processBCmdCnt=0;
  commandADoneV=0;
  printf ("TEST %d a high bit is changed in the sum word.\n", test);
  res = protocolA.doCommand(2,testData , 2, 1);
  assert(res == true);

  while (!aToBBuffer.isBufferEmpty() || !bToABuffer.isBufferEmpty()) {
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
  }


  CALL_MEMBER_FN(protocolA,timeoutFn)();

  while (!aToBBuffer.isBufferEmpty() || !bToABuffer.isBufferEmpty()) {
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
  }


  assert(gCommand == 2);
  assert(gMsb==3);
  assert(gLsb==4);
  assert(commandADoneV == 1);
  assert(processBCmdCnt == 1);


  printf ("\n-----------------------------------------\n");
  test = 100;
  printf ("TEST %d A message with just the command.\n", test);
  gCommand = 0;
  gMsb = 0;
  gLsb = 0;
  commandADoneV=0;
  processBCmdCnt=0;
  res = protocolA.doCommand(1, NULL, 0, 1);

  while (!aToBBuffer.isBufferEmpty() || !bToABuffer.isBufferEmpty()) {
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
    if (!aToBBuffer.isBufferEmpty()) {
      protocolB.processProtocol(aToBBuffer.readBuffer());    
    }
    if (!bToABuffer.isBufferEmpty()) {
      protocolA.processProtocol(bToABuffer.readBuffer());    
    }
  }


  assert(gCommand == 1);
  assert(commandADoneV == 1);
  assert(res == true);
  assert(processBCmdCnt == 1);




}
