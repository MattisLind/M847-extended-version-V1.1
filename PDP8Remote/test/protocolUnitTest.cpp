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

  switch (test) {
  case 0:
    break;
  case 1:
    break;
  }

}
void processBCmd(char command, char msb, char lsb) {
  printf ("B Received command %02X msb=%02X lsb=%02X", 0xff & command, 0xff & msb, 0xff & lsb);

  switch (test) {
  case 0:
    assert(command == 1);
    assert(msb==0);
    assert(lsb==0);
    break;
  case 1:
  case 2:
  case 3:
    assert(command == 2);
    assert(msb==3);
    assert(lsb==4);
    break;
  }


}

int byteCnt;

void aToB(char data) {
  printf ("aToB:%02X ", 0xff & data);

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
  printf ("bToA:%02X ", 0xff & data);

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


void commandADone(int status) {
  switch (test) {
  case 0:
    break;
  case 1:
    break;
  }

  printf("A DONE"); 
}

void commandBDone(int status) {
  switch (test) {
  case 0:
    break;
  case 1:
    break;
  }
  printf("B DONE"); 
}

char testData [] =  {3,4};

int main () {
  test = 0;
  printf ("TEST %d \n", test);
  protocolA.doCommand(1, NULL, 0, 1);
  test = 1;
  printf ("TEST %d \n", test);
  protocolA.doCommand(2,testData , 2, 1);
  test = 2;  // mangle one byte
  byteCnt=0;
  printf ("TEST %d \n", test);
  protocolA.doCommand(2,testData , 2, 1);
  test = 3;  // lose one byte
  byteCnt=0;
  printf ("TEST %d \n", test);
  protocolA.doCommand(2,testData , 2, 1);
  test = 4;  // lose one byte
  byteCnt=0;
  printf ("TEST %d \n", test);
  protocolA.doCommand(2,testData , 2, 1);
  CALL_MEMBER_FN(protocolA,timeoutFn)();
}
