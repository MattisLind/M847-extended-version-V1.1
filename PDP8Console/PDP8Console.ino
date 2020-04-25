/*

Parts of this file is derived from original work by Roland Huismann. In particular those portions involved in commiunicating with the underlaying PDP-8 hardware
which is taken directly from his project.

For these portiosn this copyright apply:

Copyright 2019 Roland Huisman

Permission is hereby granted, free of charge, to any person obtaining a copy of this project and associated documentation files , to deal in the project without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the project, and to permit persons to whom the project is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the project.

THE PROJECT IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE PROJECT OR THE USE OR OTHER DEALINGS IN THE PROJECT.

The remaining portions. Notably the command dispatching and command line processing is work by me, Mattis Lind.

*/


// =============================================================================================================  
//                                         Includes
  #include <Wire.h>                             // I2C handler

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



// =============================================================================================================  
//                                         Variables  
  const    byte GPIOA                     = 0x12;  // GPIOA adres in 16-bit mode, 2x 8 I/O ports.
  const    byte IOCON                     = 0x0A;  // IOCON adres in 16-bit mode, I/O Expander Configuration Register.
  const    byte IODIRA                    = 0x00;  // IODIRA adres in 16-bit mode, is het I/O Direction Register voor PortA.  
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
void SwitchRegister(word LoadData)
{
  word WordA = (LoadData)      & 0x00FF;
  word WordB = (LoadData >> 8) & 0x00FF;
  //Serial.println (WordA, HEX);
  //Serial.println (WordB, HEX);
  Wire.beginTransmission(0x20);
  Wire.write(GPIOA);                                  // gpioa
  Wire.write(byte(WordA)& 0xFF);                      // set A outputs to WordA
  Wire.write(byte(WordB)& 0xFF);                      // set B outputs to WordA
  Wire.endTransmission();
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
void AddresLoad()
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
  Wire.begin();                               // start Wire library as I2C-Bus Master

  Wire.beginTransmission(0x20);               // MCP23017 Address
  Wire.write(IOCON);                          // IOCON register
  Wire.write(byte(B01000000));                // Enable sequential addresses
  Wire.endTransmission();

  Wire.beginTransmission(0x20);
  Wire.write(IODIRA);                         // IODIRA register
  Wire.write(byte(0x00));                     // Write zeto's to outputs A
  Wire.write(byte(0x00));                     // Write zeto's to outputs B
  Wire.endTransmission();
 
 
// =============================================================================================================
//                                          Setup serial port for debugging
  Serial.begin (115200);
  digitalWrite (0                 , HIGH)   ; // turn on pull up on RX line
  Serial.println();
  Serial.print("PDP8CONSOLE> " );
}
//                                                  End setup
// =============================================================================================================


int cmdState;
int hexValue=0;
char tmp;
char cmd;
void printPrompt(char ch) {
  Serial.write(tmp);
  Serial.println();
  Serial.print("PDP8CONSOLE> " );
}

void printSR(int value) {
  Serial.write((0x07 & (value >> 12)) + 0x30);
  Serial.write((0x07 & (value >> 8)) + 0x30);
  Serial.write((0x07 & (value >> 4)) + 0x30);
  Serial.write((0x07 & value) + 0x30);  
}


void loop() {
  // put your main code here, to run repeatedly:
if (Serial.available()> 0) {
    tmp = Serial.read();
    if ((tmp >= 'a') && (tmp <= 'z')) {
      tmp &= ~0x20; // to upper case
    }
    switch (cmdState) {
      case 0:
        switch (tmp) {
          case 'C': // Continue / Run
            Serial.write(tmp);
            Serial.println();
            Continue();
            Serial.print("PDP8CONSOLE> " );
            break;
          case 'I': // Clear / Init
            Serial.write(tmp);
            Serial.println();
            Clear();
            Serial.print("PDP8CONSOLE> " );
            break;
          case 'R': // Run
            Serial.write(tmp);
            Serial.println();
            UndoSingleStep();
            Continue();
            Serial.print("PDP8CONSOLE> " );
            break;
          case 'H': // Halt
            printPrompt(tmp);
            SingleStep();
            break;
          case 'T': // Trace - single step
            printPrompt(tmp);
            SingleStep();
            Continue();
            UndoSingleStep();
            break;
          case 'E': // Load Extended Address
          case 'L': // Load Address
          case 'D': // Set SR
            Serial.write(tmp);
            Serial.write(" ");
            cmdState=1;
            cmd = tmp;
            break;
          case '?':
            Serial.println();
            Serial.println("PDP8CONSOLE HELP");
            Serial.println("=======================");
            Serial.println("? - HELP");
            Serial.println("L OOOO - Load address. For octal digits.");
            Serial.println("D OOOO - Deposit to memory given by Load address. Four octal digits");
            Serial.println("E OOOO - Extended address load.");
            Serial.println("R - Run.");
            Serial.println("H - Halt.");
            Serial.println("I - Initialize.");
            Serial.println("T - Trace.");
            Serial.println();
            Serial.print("PDP8CONSOLE> " );
            break;
          case '\r':
          case '\n': 
            Serial.println();
            Serial.print("PDP8CONSOLE> " );
            break;
        }
        break;
      case 1: 
        if ((tmp >= '0' ) && (tmp <= '7')) {
          Serial.write(tmp); // Echo character
          hexValue = (7 & tmp) << 12;
          cmdState = 2;
        }
        else {
          cmdState = 0;
          Serial.println("Invalid octal digit");
          Serial.print("PDP8CONSOLE> " );
        }
        break;        
      case 2:
        if ((tmp >= '0' ) && (tmp <= '7'))  {
          Serial.write(tmp); // Echo character
          hexValue |= (7 & tmp) << 8;
          cmdState = 3;
        }
        else {
          cmdState=0;
          Serial.println("Invalid octal digit");
          Serial.print("PDP8CONSOLE> " );
        }
      break;      
      case 3:
        if ((tmp >= '0' ) && (tmp <= '7'))  {
          Serial.write(tmp); // Echo character
          hexValue |= (7 & tmp) << 4;
          cmdState = 4;
        }
        else {
          cmdState=0;
          Serial.println("Invalid octal digit");
          Serial.print("PDP8CONSOLE> " );          
        }
      break;      
      case 4:
        if ((tmp >= '0' ) && (tmp <= '7'))  {
          Serial.write(tmp); // Echo character
          hexValue |= (7 & tmp);
          Serial.println();
          SwitchRegister(hexValue);
          switch (cmd) {
            case 'L':
              AddresLoad();
              break;
            case 'D':              
              Deposit();
              break;
            case 'E':
              ExtendedAddressLoad();
              break;
          }
        }
        else {
          Serial.println("Invalid octal digit");
        }
        cmdState = 0;
        Serial.print("PDP8CONSOLE> " );
      break;      

    }
  } 
}
