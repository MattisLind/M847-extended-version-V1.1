#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#define ACK 0xc1
#define NAK 0xc0
#define MAX_PACKET 128

class Protocol {
    void (* sendByte) (char);
    void (* processCmd) (char, char *, char);
    void (* requestTimeout) ( void (Protocol::*) (), int);
    void (* commandDone) (int);
    char protocolState; 
    char dataBuf [MAX_PACKET];
    char sendBuf[MAX_PACKET+2];
    int sendLen;
    int numResend;
    int maxResend;
    bool txEven;
    bool rxEven;
    unsigned char sum;
    char command;
    char cnt;
    char data; 
    bool sendingInProgress;
    void sendNak();
    void sendAck(bool);
    void sendCommand();
    void timeOut ();
  public:
    bool doCommand (char command, char * data, char len ,int maxRes);
    bool doCommand (char command, char msb, char lsb ,int maxRes);
    Protocol ( void (* s) (char), void (* p) (char, char *, char), void (*) ( void (Protocol::*) (), int ), void (*) (int) ); 
    void processProtocol(char tmp);
};

#endif
