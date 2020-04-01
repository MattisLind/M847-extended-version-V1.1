/*
  
 Command-line tool to control the PDP8Server on the Extended M847 board

 c++ PDP8Client.cpp ../PDP8Server/protocol.cpp -o PDP8Client
sudo ./PDP8Client -d /dev/ttyUSB0 -i ../../../Downloads/maindec-08-d1b1-pm -f rim
 

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
#include <sys/time.h>

int remoteState=0; 
int fd; // fd of the serial port is global

void processCmd(char command, char lsb, char msb) {
  // Nothing much shall come from the Server really.
}

void sendSerialChar(char ch) {
  fprintf (stderr, "S%02X\n", 0xff & ch);
  write (fd, &ch, 1);
}

typedef  void (Protocol::* TimeoutFn)();

void handleTimeout ( void (Protocol::* ) (), int);

void commandDone(int status) {
  fprintf (stderr, "commandDone status=%d remoteState = %d\n", status, remoteState);
  if (remoteState == 0 && status == 0) {
    remoteState = 1; // When in state 0 and receiving an ack it means that we goa a positive response on the NOP
  }
  if (remoteState == 2 && status ==0) {
    remoteState = 1;
  }
				      
}

class Protocol protocolHandler(sendSerialChar, processCmd, handleTimeout, commandDone);


unsigned long getTimeInUs () {
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return  1000000 * tv.tv_sec + tv.tv_usec;
}

unsigned long then;
unsigned long timeout;

TimeoutFn timeoutFn;

void handleTimeout ( void (Protocol::* t) (), int ms) {
  fprintf (stderr, "handletimeout %d ms\n", ms);
  then = getTimeInUs();
  fprintf (stderr, "then = %lu\n", then);
  timeout = ms * 1000;
  timeoutFn = t;
}

#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

void  checkIfTimeout() {
  unsigned long now  = getTimeInUs();
  fprintf (stderr, "now = %lu now-then=%lu timeout=%lu now-then> timeout %d \n", now, now-then, timeout, (now-then)>timeout);
  if (timeout >= 0) {
    if ((now - then) > timeout) {
      fprintf (stderr, "timeout occured\n");
      timeout = -1;
      CALL_MEMBER_FN(protocolHandler,timeoutFn)();
    } 
  }
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
#define LOAD_FIELD 3
#define START 4


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
int state = 0;
unsigned char msb; 
int last_address=-2;

bool processRimChar (unsigned char ch) {
  int address; 
  bool ret = false;
  fprintf (stderr, "processRimChar START %02X state=%d\n", 0xff & ch, state);
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
    }  // this state eats TRAILER
    break;
  case 2: // low address
    state = 3;
    address = msb << 6;
    address |= (ch & 0x3f);
    fprintf(stderr, "address=%04X last_address=%04X\n", address, last_address);
    if (address != last_address) {
      protocolHandler.doCommand(LOAD_ADDRESS, msb, ch & 0x3f, 10);
      ret = true;
      last_address = address;
    } 

    break;
  case 3: // high data
    state = 4; 
    msb = 0x3f & ch;
    break;
  case 4: // low data
    state = 6;
    protocolHandler.doCommand(DEPOSIT, msb, ch & 0x3f, 10);
    last_address++;
    ret = true;
    break;
  case 5:
    break;
  case 6:
    if ((ch & 0x80) == 0x80) {
      state = 5;
    } else if ((ch & 0x40) == 0x40) {
      state = 2;
      msb = 0x3f & ch;
    }  
    break;
  }
  fprintf (stderr, "processRimChar END %02X state=%d\n", 0xff & ch, state);
  return ret;
}


bool startDirectly;

bool processBinChar (unsigned char ch) {
  bool ret = false;
  switch (state) {
  case 0:
    if (ch == 0x80) {
      state = 1;
    }    
    startDirectly = false;
    break;
  case 1:
    msb = 0x3f & ch;
    if ((ch & CC_ORIGIN) == CC_ORIGIN) {
      state = 2;
      startDirectly = true;
    } else if ((ch & CC_FIELD) == CC_FIELD) {
      char tmp = ch & 0x3f;
      protocolHandler.doCommand(LOAD_FIELD, &tmp, 1 , 10);
      ret = true;
    } else {
      state = 3;
      startDirectly = false;
    }
    break;
  case 2:
    state = 6; 
    protocolHandler.doCommand(LOAD_ADDRESS, msb, ch & 0x3f, 10);
    ret = true;
    break;
  case 3: // low data
    state = 6;
    protocolHandler.doCommand(DEPOSIT, msb, ch & 0x3f, 10);
    ret = true;
    break;
  case 5: // done
    if (startDirectly) {
      protocolHandler.doCommand(START, (char *) NULL, 1, 10);
      ret = true;
    }
    break;
  case 6:
    msb = 0x3f & ch;
    if ((ch & CC_ORIGIN) == CC_ORIGIN) {
      state = 2;
      startDirectly = true;
    } else if ((ch & CC_FIELD) == CC_FIELD) {
      char tmp = ch & 0x3f;
      protocolHandler.doCommand(LOAD_FIELD, &tmp, 1 , 10);
      ret = true;
    } else if ((ch & 0x80) == 0x80) {
      state = 5;
    } else {
      state = 3;
      startDirectly = false;
    }
    break;    
  }
  return ret;
}


bool processRawChar (unsigned char ch) {
  bool ret;
  switch (state) {
  case 0:
    msb = 0x3f & ch;
    break;
  case 1:
    protocolHandler.doCommand(DEPOSIT, msb, ch & 0x3f, 10);
    ret = true;
    break;

  }
  return ret;
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
  bool format,ret;
  fd = -1;
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

  format = bin || raw || rim;
  if ((input_file == NULL) || (fd == -1) || (!format)) {
    fprintf (stderr, "Usage: %s [-i input-file -d serial-device -s start-address -f file-format (rim/bin/raw) -r run-address)]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  // send NOP to PDP8Server and wait for it to change state to connected
  fprintf (stderr, "before do-loop 1\n");
  protocolHandler.doCommand(0x00, (char *) NULL, 0, 110);
  fprintf (stderr, "after-do-loop 1\n");
  remoteState = 0;
  state = 0;
  do {
    unsigned char buf[80];
    int rdlen;
    char tmp;
    unsigned long now;
    fprintf (stderr, "do-loop 1 state = %d\n", state);
    checkIfTimeout();
    if (serial_available()) {
      fprintf (stderr, "do-loop 2 state = %d\n", state);
      // process data coming from the PDP8Server
      read(fd, &tmp, 1);
      protocolHandler.processProtocol(tmp);
    }
    switch (remoteState) {
    case 0:
      // Connecting - waiting
      fprintf (stderr, "do-loop 3 state=%d\n", state);
      now = time(NULL);
      if ((now-last_time) > 1) {
	fprintf(stderr, ".");
	last_time = now;
	//fprintf (stderr, "do-loop 4\n");
	wait_count++;
      }
      break;
    case 1:
      if ((ch=fgetc(input_file)) != EOF) {
	fprintf (stderr, "Read char from file %02X\n", ch);
	// read one character from the file and process it according to filetype
	if (rim) {
	  ret = processRimChar(ch);
	} else if (bin) {
	  ret = processBinChar(ch);	  
	} else if (raw) {
	  ret = processRawChar(ch);
	}
      }
      if (ret) {
	remoteState=2;
      }
      break;
    case 2:
      break;
    case 3:
      // set the starting address for raw mode
      protocolHandler.doCommand(LOAD_ADDRESS, 0x3f & (current_address >> 6), 0x3f & current_address, 110);
      state = 5;
      break;
	
    }
    fprintf(stderr, "before loop end %d wait_count %d state=%d\n", ch, wait_count, state);
  } while ((ch != EOF) && (wait_count < 100) && (state != 5));
  close(fd);
  fclose(input_file);
}


