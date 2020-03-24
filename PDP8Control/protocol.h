#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#define ACK 0xc1
#define NAK 0xc0


class Protocol {
    void (* sendByte) (char);
    void (* processCmd) (char, char, char);
    char protocolState = 0; 
    char dataBuf [2];
    unsigned char sum;
    char command;
    char cnt;
    char data; 
    void sendNak();
    void sendAck();
  public:
    void doCommand (char command, char * data, char len);
    Protocol ( void (* s) (char), void (* p) (char, char, char)); 
    void processProtocol(char tmp);
};

#endif
