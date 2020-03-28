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
#include "../PDP8Server/protocol.h"

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
  if (state == 0 && status == 0) {
    state = 1; // When in state 0 and receiving an ack it means that we goa a positive response on the NOP
  }

}

class Protocol protocolHandler(sendSerialChar, processCmd, handleTimeout, commandDone);

// Define control codes and bit masks
#define CC_LEAD         0x80
#define CC_TRAIL        0x80
#define CC_ORIGIN       0x40
#define CC_FIELD        0xC0
#define CC_FIELD_MASK   0x1C
#define CC_DATA_MASK    0x3F
#define CC_CONTROL_MASK 0xC0

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

enum captureState_e {
  CS_START = 0,
  CS_LEAD_IN,
  CS_DATA_H,
  CS_DATA_L,
  CS_ADDRESS_H,
  CS_ADDRESS_L,
  CS_TRAIL,
  CS_DONE
};

void capture_raw(FILE *f, enum captureState_e *state, unsigned char c)
{
  // Just capture every byte recived until time out. Leave CS_START state to allow time out
  switch (*state) {
  case CS_START:
    *state = CS_LEAD_IN;
  case CS_LEAD_IN:
  case CS_DATA_H:
  case CS_DATA_L:
  case CS_ADDRESS_H:
  case CS_ADDRESS_L:
  case CS_TRAIL:
  case CS_DONE:
    fputc(c, f);
    break;
  }
}

void capture_rim(FILE *f, enum captureState_e *state, unsigned char c)
{
  static int leadinCount = 0;
  
  switch (*state) {
  case CS_START:
    if (c == CC_LEAD)
      leadinCount++;
    *state = CS_LEAD_IN;
    break;

  case CS_LEAD_IN:
    if (c == CC_LEAD) {
      leadinCount++;
    } else if (((c & CC_CONTROL_MASK) == CC_ORIGIN && leadinCount > 7)){
      while (leadinCount--) { 
	fputc(CC_LEAD, f);
      }
      fputc(c, f);
      *state = CS_DATA_H;
    } else {
      leadinCount = 0;
    }
    break;

  case CS_DATA_H:
    if (c == 0x80) {
      *state = CS_TRAIL;
    }
    fputc(c, f);
    break;

  case CS_TRAIL:
    if (c != CC_TRAIL) {
      *state = CS_DONE;
    } else {
      fputc(c, f);
    }
    break;

  case CS_DONE:
    break;

  case CS_DATA_L:
  case CS_ADDRESS_H:
  case CS_ADDRESS_L:
    break;
  }
} 

void capture_bin(FILE *f, enum captureState_e *state, unsigned char c)
{
  static int leadinCount = 0;
  static int csum = 0;
  static int c1 = 0;
  static int c2 = 0;
  
  switch (*state) {
  case CS_START:
    if (c == CC_LEAD)
      leadinCount++;
    *state = CS_LEAD_IN;
    break;

  case CS_LEAD_IN:
    if (c == CC_LEAD) {
      leadinCount++;
    } else if (((c & CC_CONTROL_MASK) == CC_ORIGIN && leadinCount > 7) || 
	       ((c & CC_CONTROL_MASK) == CC_FIELD && leadinCount > 7))
      {
        while (leadinCount--) { 
          fputc(CC_LEAD, f);
        }
        
        // CC_FIELD is NOT used for checksum
        if ((c & CC_CONTROL_MASK) == CC_ORIGIN) {
          csum += c;
        }
        
        fputc(c, f);
        *state = CS_DATA_L;
      } else {
      leadinCount = 0;
    }
    break;

  case CS_DATA_L:
    fputc(c, f);
      
    // CC_TRAIL or CC_FIELD is not used in checksum calculation
    if ( !(c & 0x80)) { 
      csum += c;
      c2 = c1;
      c1 = c;
    }

    if (c == 0x80) {
      int checksum = (c2 & 0x3f) << 6 | (c1 & 0x3f);
      csum = (csum - c1 - c2) & 0xfff;
        
      // Verify C-SUM
      if (csum  == checksum){
	printf("Checksum OK!: %4o\n", checksum);
      } else {
	printf("Checksum FAIL!: calc %4o <-> recv %4o\n", csum, checksum);
      }
      *state = CS_TRAIL;
    }
    break;

  case CS_TRAIL:
    if (c != CC_TRAIL) {
      *state = CS_DONE;
    } else {
      fputc(c, f);
    }
    break;

  case CS_DONE:
    break;
  }
}

int open_serial (char * device) {
      fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
      if (fd < 0) return -1;

      /* Baudrate 115200, 8 bits, no parity, 1 stop bit */
      if (set_interface_attribs(fd, B115200) < 0) {
	exit(EXIT_FAILURE);
      }  
}

int start_address;
int current_address;
int run_address;
int state;
int tmp; 

void processRimChar (char ch) {
  int t = ch;
  switch (state) {
  case 0:
    if (ch == 0x80) {
      state = 1;
    }
    break;
  case 1: // high address
    if ((ch & 0x40) == 0x40) {
      state = 2;
      tmp = (0x3f & t) << 6;
    } 
    break;
  case 2: // low address
    state = 3; 
    tmp |= 0x3f & ch;
    if (++current_address != tmp) {
      doLoadAddress(tmp);
    }
    break;
  case 3: // high data
    state = 4; 
    tmp = (0x3f & t) << 6;
    break;
  case 4: // low data
    state = 1;
    tmp = 0x3f & t;
    deposit(tmp);
    break;
  }
}


void processBinChar (char ch) {

}


void processRawChar (char ch) {
  int t = ch;
  switch (state) {
  case 0:
    tmp = (0x3f & t) << 6;
    break;
  case 1:
    tmp = 0x3f & t;
    deposit(tmp);
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
  enum captureState_e state = CS_START;
  bool time_out = false;
  bool bin = false, rim = false, raw = false;
  int state=0;
  int ch
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
      if (sscanf(otarg, "%4o",&start_address) != 1) {
	fprintf(stderr, "Failed to parse the start address given. Requires an octal number.\n");
	exit(EXIT_FAILURE);
      } 
      break;
    case 'd':
      if (open_serial(optarg)) {
	fprintf(stderr, "Error opening device %s: %s\n", portname, strerror(errno));
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
      if (sscanf(otarg, "%4o",&run_address) != 1) {
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
  protocolHandler.doCommand(0x00, NULL, 0); 
  state = 0;
  do {
    unsigned char buf[80];
    int rdlen;
    char tmp;
    if (serial_available()) {
      // process data coming from the PDP8Server
      read(fd, &tmp, 1);
      protocolHandler.processProtocol(tmp);
    }
    switch (state) {
    case 0:
      // Connecting - waiting 
      break;
    case 1:
      if ((ch=fgetc(input_file)) != FEOF) {
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
    case 2:
      break;
    }
  } while (ch != FEOF && !connect_timeout);
  close(fd);
  fclose(input_file);
}


      for (p = buf; rdlen-- > 0; p++) {
        if (bin) capture_bin(fCapture, &state, *p);
        if (rim) capture_rim(fCapture, &state, *p);
        if (raw) capture_raw(fCapture, &state, *p);
      }
      time_out = false;
    } else if (rdlen == 0) {
      time_out = true;
    } else {
      fprintf(stderr, "Error from read: %d: %s\n", rdlen, strerror(errno));
      time_out = true;
    }
