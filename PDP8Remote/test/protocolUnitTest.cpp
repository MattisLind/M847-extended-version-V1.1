/*

 Compile using c++ protocolUnitTest.cpp ../PDP8Server/protocol.cpp -o protocolUnitTest

 */

#include <assert.h>
#include <stdio.h>
#include "../PDP8Server/protocol.h"
#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

void processACmd(char, char, char);
void processBCmd(char, char, char);
void aToB(char);
void bToA(char);
typedef  void (Protocol::* TimeoutFn)();
void handleTimeout ( void (Protocol::* ) (), int);
void commandADone(int);
void commandBDone(int);


int test;


class Protocol protocolA(aToB, processACmd, handleTimeout, commandADone);

class Protocol protocolB(bToA, processBCmd, handleTimeout, commandBDone);


void processACmd(char command, char msb, char lsb) {
  printf ("A Received command %02X msb=%02X lsb=%02X", 0xff & command, 0xff & msb, 0xff & lsb);
}

char gCommand;
char gMsb;
char gLsb;

void processBCmd(char command, char msb, char lsb) {
  printf ("B Received command %02X msb=%02X lsb=%02X", 0xff & command, 0xff & msb, 0xff & lsb);

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
    protocolB.processProtocol(data);
    break;
  case 2:
    if (byteCnt==2) {
      data |= 0x20;
    }
    protocolB.processProtocol(data);
    byteCnt++;
    break;
  case 3:
    if (byteCnt!=2) {
      protocolB.processProtocol(data);
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
    protocolA.processProtocol(data);
    break;
  case 4:
    if (byteCnt != 0) {
      protocolA.processProtocol(data);
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


bool commandADoneV;

void commandADone(int status) {
  printf("A DONE\n"); 
  commandADoneV = true;
}

void commandBDone(int status) {
  printf("B DONE\n"); 
}

char testData [] =  {3,4};

int main () {
  test = 0;
  printf ("TEST %d A message with just the command.\n", test);
  protocolA.doCommand(1, NULL, 0, 1);
  assert(gCommand == 1);
  assert(gMsb==0);
  assert(gLsb==0);
  assert(commandADoneV == true);

  test = 1;
  printf ("TEST %d A message with two data bytes. \n", test);
  protocolA.doCommand(2,testData , 2, 1);
  assert(gCommand == 2);
  assert(gMsb==3);
  assert(gLsb==4);
  assert(commandADoneV == true);

  test = 2;  // mangle one byte
  byteCnt=0;
  printf ("TEST %d One bit in the message is wrong. \n", test);
  protocolA.doCommand(2,testData , 2, 1);
  assert(gCommand == 2);
  assert(gMsb==3);
  assert(gLsb==4);
  assert(commandADoneV == true);

  test = 3;  // lose one byte
  byteCnt=0;
  printf ("TEST %d One byte in the message is lost. \n", test);
  protocolA.doCommand(2,testData , 2, 1);
  assert(gCommand == 2);
  assert(gMsb==3);
  assert(gLsb==4);
  assert(commandADoneV == true);

  test = 4;  // lose the ACK
  byteCnt=0;
  printf ("TEST %d The first ACK is lost\n", test);
  protocolA.doCommand(2,testData , 2, 1);
  CALL_MEMBER_FN(protocolA,timeoutFn)();
  assert(gCommand == 2);
  assert(gMsb==3);
  assert(gLsb==4);
  assert(commandADoneV == true);
}
