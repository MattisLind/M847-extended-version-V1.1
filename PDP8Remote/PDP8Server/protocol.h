#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#define ACK 0xc1
#define NAK 0xc0


class Protocol {
    void (* sendByte) (char);
    void (* processCmd) (char, char, char);
    void (* requestTimeout) ( void (Protocol::*) (), int);
    void (* commandDone) (int);
    char protocolState; 
    char dataBuf [2];
    char sendBuf[2];
    int sendLen;
    int numResend;
    int maxResend;
    unsigned char sum;
    char command;
    char cnt;
    char data; 
    bool sendingInProgress;
    void sendNak();
    void sendAck();
    void sendCommand();
    void timeOut ();
  public:
    bool doCommand (char command, char * data, char len ,int maxRes);
    Protocol ( void (* s) (char), void (* p) (char, char, char), void (*) ( void (Protocol::*) (), int ), void (*) (int) ); 
    void processProtocol(char tmp);
};

#endif
