/*
  
 Command-line tool to control the PDP8Server on the Extended M847 board

 RIM, BIN prsing is from code written by Anders Sandahl.

 */


#include <errno.h>
#include <fcntl.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <poll.h>
#include "../PDP8Server/protocol.h"

int remoteState=0; 
int fd; // fd of the serial port is global

void processCmd(char command, char lsb, char msb) {
  // Nothing much shall come from the Server really.
}

void sendSerialChar(char ch) {
  write (fd, &ch, 1);
}

typedef  void (Protocol::* TimeoutFn)();

void handleTimeout ( void (Protocol::* ) (), int);

void commandDone(int status) {
  if (remoteState == 0 && status == 0) {
    remoteState = 1; // When in state 0 and receiving an ack it means that we goa a positive response on the NOP
  }

}

class Protocol protocolHandler(sendSerialChar, processCmd, handleTimeout, commandDone);


void handleTimeout ( void (Protocol::* t) (), int ms) {

}



// Define control codes and bit masks
#define CC_LEAD         0x80
#define CC_TRAIL        0x80
#define CC_ORIGIN       0x40
#define CC_FIELD        0xC0
#define CC_FIELD_MASK   0x1C
#define CC_DATA_MASK    0x3F
#define CC_CONTROL_MASK 0xC0

#define NOP 0
#define LOAD_ADDRESS 1
#define DEPOSIT 2

int set_interface_attribs(int fd, int speed)
{
  struct termios tty;

  if (tcgetattr(fd, &tty) < 0) {
    fprintf(stderr, "Error from tcgetattr: %s\n", strerror(errno));
    return -1;
  }

  cfsetospeed(&tty, (speed_t)speed);
  cfsetispeed(&tty, (speed_t)speed);
  tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
  tty.c_cflag &= ~CSIZE;
  tty.c_cflag |= CS8;         /* 8-bit characters */
  tty.c_cflag &= ~PARENB;     /* no parity bit */
  tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
  tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

  /* setup for non-canonical mode */
  tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
  tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
  tty.c_oflag &= ~OPOST;

  // Read with timeout, 1 s

  tty.c_cc[VMIN] = 0;
  tty.c_cc[VTIME] = 10;

  if (tcsetattr(fd, TCSANOW, &tty) != 0) {
    fprintf(stderr, "Error from tcsetattr: %s\n", strerror(errno));
    return -1;
  }
  return 0;
}


int open_serial (char * device) {
      fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
      if (fd < 0) return -1;

      /* Baudrate 115200, 8 bits, no parity, 1 stop bit */
      if (set_interface_attribs(fd, B115200) < 0) {
        return -1;
      }  
      return 0;
}

int start_address;
int current_address;
int run_address;
int state;
unsigned char msb; 

void processRimChar (unsigned char ch) {
  switch (state) {
  case 0:
    if (ch == 0x80) {
      state = 1;
    }
    break;
  case 1: // high address
    if ((ch & 0x40) == 0x40) {
      state = 2;
      msb = 0x3f & ch;
    } else if ((ch & 0x80) == 0x80) {
      state = 5;
    }
    break;
  case 2: // low address
    state = 3; 
    protocolHandler.doCommand(LOAD_ADDRESS, msb, ch & 0x3f, 10);
    break;
  case 3: // high data
    state = 4; 
    msb = 0x3f & ch;
    break;
  case 4: // low data
    state = 1;
    protocolHandler.doCommand(DEPOSIT, msb, ch & 0x3f, 10);
    break;
  case 5:
    break;
  }
}


void processBinChar (unsigned char ch) {
  switch (state) {
  case 0:
    if (ch == 0x80) {
      state = 1;
    }    
    break;
  case 1:
    msb = 0x3f & ch;
    if ((ch & 0x40) == 0x40) {
      state = 2;
    } 
    else if ((ch & 0x80) == 0x80) {
      state = 5;
    } else {
      state = 3;
    }
    break;
  case 2:
    state = 1; 
    protocolHandler.doCommand(LOAD_ADDRESS, msb, ch & 0x3f, 10);
    break;
  case 3: // low data
    state = 1;
    protocolHandler.doCommand(DEPOSIT, msb, ch & 0x3f, 10);
    break;
  case 5: // done
    break;
  }
}


void processRawChar (unsigned char ch) {
  switch (state) {
  case 0:
    msb = 0x3f & ch;
    break;
  case 1:
    protocolHandler.doCommand(DEPOSIT, msb, ch & 0x3f, 10);
    break;

  }
}


int serial_available () {
  int ret;
  struct pollfd pollfd;
  pollfd.fd = fd;
  pollfd.events = POLLIN;
  ret = poll(&pollfd, 1 , 10);
  return ret; 
} 



int main(int argc, char **argv)
{

  int wlen;
  FILE *input_file;
  bool bin = false, rim = false, raw = false;
  time_t last_time = time(NULL);    
  int wait_count=0;
  int ch;
  int opt;
  while ((opt = getopt(argc, argv, "r:s:d:f:i:")) != -1) {
    switch (opt) {
    case 'i':
      input_file = fopen (optarg, "r");
      if (input_file == NULL) {
	perror ("Failure opening file.");
	exit(EXIT_FAILURE);
      }
      break;
    case 's':
      if (sscanf(optarg, "%4o",&start_address) != 1) {
	fprintf(stderr, "Failed to parse the start address given. Requires an octal number.\n");
	exit(EXIT_FAILURE);
      } 
      break;
    case 'd':
      if (open_serial(optarg)) {
	fprintf(stderr, "Error opening device %s: %s\n", optarg, strerror(errno));
	exit(EXIT_FAILURE);	
      }
      break;
    case 'f':
      if (strncmp(optarg, "rim", 3)==0) {
	rim = true;
      }
      else if (strncmp(optarg, "bin", 3)==0) {
	bin = true;
      }
      else if (strncmp(optarg, "raw", 3)==0) {
	raw = true;
      } 
      else { 
	fprintf (stderr, "Format given is invalid. Can be wither rim, bin or raw.\n");
      }
      break;
    case 'r':
      if (sscanf(optarg, "%4o",&run_address) != 1) {
	fprintf(stderr, "Failed to parse the start address given. Requires an octal number.\n");
	exit(EXIT_FAILURE);
      } 
      break;
    default: /* '?' */
      fprintf(stderr, "Usage: %s [-i input-file -d serial-device -s start-address -f file-format (rim/bin/raw) -r run-address)]\n", argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  // send NOP to PDP8Server and wait for it to change state to connected  
  protocolHandler.doCommand(0x00, (char *) NULL, 0, 110); 
  remoteState = 0;
  do {
    unsigned char buf[80];
    int rdlen;
    char tmp;

    if (serial_available()) {
      // process data coming from the PDP8Server
      read(fd, &tmp, 1);
      protocolHandler.processProtocol(tmp);
    }
    switch (remoteState) {
    case 0:
      // Connecting - waiting
      if ((time(NULL)-last_time) > 1) {
	printf(".");
	wait_count++;
      }
      break;
    case 1:
      if ((ch=fgetc(input_file)) != EOF) {
	// read one character from the file and process it according to filetype
	if (rim) {
	  processRimChar(ch);
	} else if (bin) {
	  processBinChar(ch);	  
	} else if (raw) {
	  processRawChar(ch);
	}
      }      
      break;
    }
  } while ((ch != EOF) && (wait_count < 100) && (state != 5));
  close(fd);
  fclose(input_file);
}
