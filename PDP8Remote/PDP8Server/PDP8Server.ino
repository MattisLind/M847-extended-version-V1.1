

/*

Parts of this file is derived from original work by Roland Huismann. In particular those portions involved in commiunicating with the underlaying PDP-8 hardware
which is taken directly from his project.

For these portiosn this copyright apply:

Copyright 2019 Roland Huisman

Permission is hereby granted, free of charge, to any person obtaining a copy of this project and associated documentation files , to deal in the project without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the project, and to permit persons to whom the project is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the project.

THE PROJECT IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE PROJECT OR THE USE OR OTHER DEALINGS IN THE PROJECT.

The remaining portions. Notably the command dispatching and protocol stack is work by me, Mattis Lind. 


*/


// =============================================================================================================  
//                                         Includes

#include "I2C16.h"                             // I2C handler
#include "EEPROM_24XX1025.h"
#include "protocol.h"  


EEPROM_24XX1025 eeprom (0, 0);              // The EEPROM library will call begin and setSpeed of the I2c16 library  


// =============================================================================================================
//                                      400nS pulse to Initialize
void Trigger_Initialize ()
  {
    PORTC |= B00000001 ; // Turn on PC0
    __asm__("nop\n\t");  //
    __asm__("nop\n\t");  //
    __asm__("nop\n\t");  // Wait for 400nS
    __asm__("nop\n\t");  //
    __asm__("nop\n\t");  //
    __asm__("nop\n\t");  //
    PORTC &= B11111110 ; // Turn off PC0
  }


// =============================================================================================================
//                                      400nS pulse to Latch in address
void Trigger_Adres_Latch ()
  {
    PORTC |= B00000010 ; // Turn on PC1
    __asm__("nop\n\t");  //
    __asm__("nop\n\t");  //
    __asm__("nop\n\t");  // Wait for 400ns
    __asm__("nop\n\t");  //
    __asm__("nop\n\t");  //
    __asm__("nop\n\t");  //
    PORTC &= B11111101 ; // Turn off PC1
  }


// =============================================================================================================
//                                      400nS pulse to start memorycycle
void Trigger_Mem_Start ()
  {
    PORTC |= B00000100; // Turn on PC2
    __asm__("nop\n\t"); //
    __asm__("nop\n\t"); //
    __asm__("nop\n\t"); // Wait for 400ns
    __asm__("nop\n\t"); //
    __asm__("nop\n\t"); //
    __asm__("nop\n\t"); //
    PORTC &= B11111011; // Turn on PC2
  }




// =============================================================================================================  
//                                         Pin definitions
#define r_RUN                   8           // PB0
#define r_SW                    9           // PB1
#define Dip_1                   10          // PB2
#define Dip_2                   11          // PB3
#define Dip_3                   12          // PB4
#define Dip_4                   13          // PB5
#define BRK_DATA                10          // PB2

#define w_INITIALIZE_H          A0          // PC0 for 400nS pulse
#define w_PULSE_LA_H            A1          // PC1 for 400nS pulse
#define w_MEM_START             A2          // PC2 for 400nS pulse
#define w_STOP                  A3          // PC3
  
#define Set_Flip_Flop           2           // PD2 
#define Show_Data               3           // PD3 
#define Exam                    4           // PD4 
#define w_LA_ENABLE             5           // PD5
#define w_MS_IR_DISABLE         6           // PD6
#define w_KEY_CONTROL           7           // PD7
#define GPIOA  0x12
#define IOCON  0x0a
#define IODIRA 0x00


// =============================================================================================================  
//                                         Variables  
  volatile byte ProgramNumber             = 0x00;  // program to run
  volatile byte pulseState                = 0x00;  // Previous state of switch
  volatile unsigned long SwitchTimeOut    = 3000;  // This is the wait time (ms) after the last toggle. Then the program loads.
  volatile word ProgramLength             = 0x0000;// To calculate the amount of words in the programs
  volatile byte RunOnce                   = 0x00;  // just run Kitt once at turn on

  unsigned long SlowDown                  = 5;     // Blink delay in milli seconds. This slows down the loading of a bootstrap to give it a nice blinking effect.
                                                   // Placing a 0 loads the program at full speed, you can't really see a program to be loaded.
                                                   // Placing 50 will give a nice blinking effect but it slows down the program loading.



// =============================================================================================================
//                       Transfer the switchregisterdata to the MC23017 to deposit or load address
void SwitchRegister(word data)
{
  I2c16.write ((byte) 0x20, (word) ((GPIOA<<8) | (data & 0xff)), (byte) (data>>8) );  
}


// =============================================================================================================
//                                          Put PDP into Single Step mode
void SingleStep()
  {
    digitalWrite (w_STOP , HIGH);                     // Places the machine in Single step mode
  }


// =============================================================================================================
//                                          Get PDP out of Single Step mode
void UndoSingleStep()
  {
    digitalWrite (w_STOP , LOW);                     // Get the machine out of Single step mode
  }



// =============================================================================================================
//                                                Deposit
void Deposit()
  {
    digitalWrite (Set_Flip_Flop     , HIGH)    ;
    digitalWrite (w_KEY_CONTROL     , HIGH)    ; 
    digitalWrite (Show_Data         , HIGH)    ; 
    digitalWrite (w_MS_IR_DISABLE   , HIGH)    ;
    digitalWrite (BRK_DATA          , HIGH)    ; // BRK DATA has to be high.    
    Trigger_Mem_Start ();
    digitalWrite (Set_Flip_Flop     , LOW)     ; 
    digitalWrite (w_KEY_CONTROL     , LOW)     ;
    digitalWrite (Show_Data         , LOW)     ;
    digitalWrite (w_MS_IR_DISABLE   , LOW)     ;
    digitalWrite (BRK_DATA          , LOW)     ; // BRK DATA has to be high.    
    
  }



// =============================================================================================================
//                                               Address Load
void AddressLoad()
  {
    digitalWrite (w_LA_ENABLE       , HIGH)    ; // get machine ready to receive an address 
    digitalWrite (w_MS_IR_DISABLE   , HIGH)    ; // get machine ready to receive an address 
    digitalWrite (Set_Flip_Flop     , HIGH)    ; // get machine ready to receive an address 
    digitalWrite (Show_Data         , HIGH)    ;                                            
    digitalWrite (Exam              , HIGH)    ; 
    digitalWrite (BRK_DATA          , LOW)     ; // BRK DATA has to be high.                                           
    Trigger_Adres_Latch ();
    digitalWrite (w_LA_ENABLE       , LOW)     ; // get machine out of address latch mode
    digitalWrite (w_MS_IR_DISABLE   , LOW)     ; // get machine out of address latch mode
    digitalWrite (Set_Flip_Flop     , LOW)     ; // get machine out of address latch mode
    digitalWrite (Show_Data         , LOW)     ;
    digitalWrite (Exam              , LOW)     ;
    digitalWrite (BRK_DATA          , HIGH)    ; // BRK DATA has to be high.    
  }




// =============================================================================================================
//                                       Extended Memory Address Load
void ExtendedAddressLoad()
  {
    digitalWrite (w_LA_ENABLE       , HIGH)    ; // get machine ready to receive an extended address  
    digitalWrite (w_KEY_CONTROL     , HIGH)    ; // get machine ready to receive an extended address  
    digitalWrite (Set_Flip_Flop     , HIGH)    ; // get machine ready to receive an extended address  
    digitalWrite (Show_Data         , HIGH)    ;                                                      
    Trigger_Adres_Latch ();
    digitalWrite (w_LA_ENABLE       , LOW)     ; // get machine out of extended address latch mode
    digitalWrite (w_KEY_CONTROL     , LOW)     ; // get machine out of extended address latch mode
    digitalWrite (Set_Flip_Flop     , LOW)     ; // get machine out of extended address latch mode
    digitalWrite (Show_Data         , LOW)     ;
  }




// =============================================================================================================
//                                                 Clear
void Clear()
  {
    if (digitalRead(r_RUN) == HIGH)         // check if machine is not running
      {
        Trigger_Initialize ();              // give 400ns positive pulse on Initialize_H
      }
    if (digitalRead(r_RUN) == LOW)          // Warning, machine seems to run ! Not cleared
      {
        Serial.print("ERROR !! Machine not cleared due RUN state !");
      }
    Serial.println();  
  }

  


// =============================================================================================================
//                                      Continue, set machine in RUN mode
void Continue()
  {
    if (digitalRead(r_RUN) == LOW)          // Warning, machine seems to run ! Not started
      {
        Serial.print("ERROR !! Machine not started due RUN state !");
      }
    if (digitalRead(r_RUN) == HIGH)         // check if machine is not running
      {
        Trigger_Mem_Start ();               // give 400ns positive pulse on Initialize_H
      }
  }






void setup ()
{

  pinMode (r_RUN                  , INPUT)  ; // read RUN signal from Omnibus
  pinMode (r_SW                   , INPUT)  ; // read SW signal from Omnibus
  pinMode (Dip_1                  , OUTPUT)  ; // Default bootprogam select for one time toggeling SW
  pinMode (Dip_2                  , INPUT)  ; // Default bootprogam select for one time toggeling SW 
  pinMode (Dip_3                  , INPUT)  ; // Default bootprogam select for one time toggeling SW
  pinMode (Dip_4                  , INPUT)  ; // Default bootprogam select for one time toggeling SW

  digitalWrite (r_RUN             , HIGH)   ; // turn on pull up
  digitalWrite (r_SW              , HIGH)   ; // turn on pull up
  digitalWrite (Dip_1             , LOW)   ; // turn on pull up
  digitalWrite (Dip_2             , HIGH)   ; // turn on pull up
  digitalWrite (Dip_3             , HIGH)   ; // turn on pull up
  digitalWrite (Dip_4             , HIGH)   ; // turn on pull up

  digitalWrite (w_INITIALIZE_H    , LOW)    ; // Write zero before initializing output
  digitalWrite (w_PULSE_LA_H      , LOW)    ; // Write zero before initializing output
  digitalWrite (w_MEM_START       , LOW)    ; // Write zero before initializing output
  digitalWrite (w_STOP            , LOW)    ; // Write zero before initializing output

  pinMode (w_INITIALIZE_H         , OUTPUT) ; // Set to output mode
  pinMode (w_PULSE_LA_H           , OUTPUT) ; // Set to output mode
  pinMode (w_MEM_START            , OUTPUT) ; // Set to output mode
  pinMode (w_STOP                 , OUTPUT) ; // Set to output mode

  digitalWrite (Set_Flip_Flop     , LOW)    ; // Write zero before initializing output
  digitalWrite (Show_Data         , LOW)    ; // Write zero before initializing output
  digitalWrite (Exam              , LOW)    ; // Write zero before initializing output
  digitalWrite (w_LA_ENABLE       , LOW)    ; // Write zero before initializing output
  digitalWrite (w_MS_IR_DISABLE   , LOW)    ; // Write zero before initializing output
  digitalWrite (w_KEY_CONTROL     , LOW)    ; // Write zero before initializing output
  
  pinMode (Set_Flip_Flop          , OUTPUT) ; // Set to output mode
  pinMode (Show_Data              , OUTPUT) ; // Set to output mode
  pinMode (Exam                   , OUTPUT) ; // Set to output mode
  pinMode (w_LA_ENABLE            , OUTPUT) ; // Set to output mode
  pinMode (w_MS_IR_DISABLE        , OUTPUT) ; // Set to output mode
  pinMode (w_KEY_CONTROL          , OUTPUT) ; // Set to output mode




// =============================================================================================================
//                                          Start I2C bus
  I2c16.write(0x20, (IOCON<<8) | B01000000);
  I2c16.write(0x20, (IODIRA<<8), 0x00); 
 
 
// =============================================================================================================
//                                          Setup serial port for debugging
  Serial.begin (115200);
  digitalWrite (0                 , HIGH)   ; // turn on pull up on RX line
}
//                                                  End setup
// =============================================================================================================


word make12BitWord (char msb,  char lsb) {
  word tmp, ret;
  tmp = msb & 0x38;
  ret = tmp << 9;
  tmp = 0x07 & msb;
  ret |= tmp << 8;
  tmp = 0x38 & lsb;
  ret |= tmp << 1;
  tmp = 0x07 & lsb;
  ret |= tmp;
  return ret;
}

void processCmd(char command, char msb, char lsb) {
  word octalValue = make12BitWord(msb, lsb); 
  switch (command) {
    case 0: // NOP
      break;
    case 1:  //Load address
      SwitchRegister(octalValue);
      AddressLoad();
      break;
    case 2: // Deposit
      SwitchRegister(octalValue);
      Deposit();
      break;
    case 3: // Extended address load
      SwitchRegister(octalValue);
      ExtendedAddressLoad();
      break;
    case 4: // Initialize
      Clear();
      break;
    case 5: // Run
      UndoSingleStep();
      Continue();
      break;
    case 6: // Trace
      SingleStep();
      Continue();
      UndoSingleStep();
      break;
  }
}

void sendByte(char data) {
  Serial.write(data);
}

typedef  void (Protocol::* TimeoutFn)();

TimeoutFn timeoutFn;
int  timeout;
unsigned long previousMillis;
void handleTimeout ( void (Protocol::* t) (), int ms) {

  timeoutFn = t;
  previousMillis = millis();
}


#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))


void commandDone(int status) {

}


class Protocol protocol (sendByte, processCmd, handleTimeout, commandDone); 

void checkTimeout() {
  if (timeout > 0) {
    if ((unsigned long)(millis() - previousMillis) >= timeout) {
      timeout = -1;
      CALL_MEMBER_FN(protocol,timeoutFn)();
    }
  }  
}

void loop() {
  char tmp;
  checkTimeout();
  if (Serial.available()> 0) {
    tmp = Serial.read();
    protocol.processProtocol (tmp); 
  }
}
