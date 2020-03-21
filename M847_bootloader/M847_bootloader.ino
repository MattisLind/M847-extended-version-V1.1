// =============================================================================================================  
// Bootloader for Digital PDP8/e PDP8/f PDP8/m computers by Roland Huisman. MIT license
// V1.1 fixed false run state message
// V1.2 changed RX(2)8 boot address for RX(2)8 (RX01/RX02) bootloader
// V1.3 Expanded to currently maximum possible 63 programs, so users can just replace the line with their own boot code
// V1.4 - Added Kaleidoscope. 
//      - Removed delays and Added variable "SlowDown". So you can determine your own loading speed. 
//        It's set to a quite fast but still blinky rate. Search for unsigned long SlowDown which is currently set at 35 milliseconds.
//        You can change it if you want to speed up or slow down the loading. 0 gives the maximum load speed 200 gives a real slow loading.
// V1.5 Added TA8E TU60 bootstrap for Caps-8 and OS/8 setup cassettes
// V1.6 Added program 37, receive charachters on 03/04 serial port and place them into AC
// V1.7 - Updated BIN loader. You don't need to set the switches to 7777 when you want to use the serial port. 
//        It is now permanent set to use the 'low speed reader'. Which actually means that you can use the serial port at high speed.
//      - Besides the BIN loader at 03/04 there is now also a secondary BIN loader at 40/41. So if you have a slow dumb terminal at
//        03/04 you can upload BIN images trough the second serial port at high speed. When you start your program it wil
//        show up on your dumb terminal or TTY.
//      - Changed the order of the bootstraps to a more logical structure
// =============================================================================================================  



// =============================================================================================================  
//                                         Bootload programs
//
// Please note the data format. You can use the octal numbers after 0x....
// Please fill in all the numbers even if they are zero
// 
// The first number is the initial address to deposit from
// The second number is the EMA address (IF and DF)
// Then the complete program can be written
// The last number is the start address of the program
//
// Example: 
// We want a program located at address 200
// The EMA address is 0
// The data to deposit in memory is: 7001, 2300, 5201, 5200
// The startaddress is 200
//
// Should look like this: 
// const PROGMEM word Program_01[] = {0x0200, 0x0000, 0x7001, 0x2300, 0x5201, 0x5200, 0x0200};  //AC increment program
//
// 
// All Octal values are repesented as HEX numbers, and put as HEX into the MCP23017. 
// Due hardware the bits 15, 11, 7 and 3 are zero and just not used.
// Programnumbers are shown in Octal as well... The highest program number is currently 77 oct (so max 63 programs)
// Program_00 does not exist, this is used to load the selected default 
// 
//
// =============================================================================================================  




#include "programs.h"





// =============================================================================================================  
//                                         Includes
  #include <Wire.h>                             // I2C handler




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

  
void setup ()
{
// =============================================================================================================  
//                                         Pin definitions
  #define r_RUN                   8           // PB0
  #define r_SW                    9           // PB1
  #define Dip_1                   10          // PB2
  #define Dip_2                   11          // PB3
  #define Dip_3                   12          // PB4
  #define Dip_4                   13          // PB5

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

  pinMode (r_RUN                  , INPUT)  ; // read RUN signal from Omnibus
  pinMode (r_SW                   , INPUT)  ; // read SW signal from Omnibus
  pinMode (Dip_1                  , INPUT)  ; // Default bootprogam select for one time toggeling SW
  pinMode (Dip_2                  , INPUT)  ; // Default bootprogam select for one time toggeling SW 
  pinMode (Dip_3                  , INPUT)  ; // Default bootprogam select for one time toggeling SW
  pinMode (Dip_4                  , INPUT)  ; // Default bootprogam select for one time toggeling SW

  digitalWrite (r_RUN             , HIGH)   ; // turn on pull up
  digitalWrite (r_SW              , HIGH)   ; // turn on pull up
  digitalWrite (Dip_1             , HIGH)   ; // turn on pull up
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
}
//                                                  End setup
// =============================================================================================================




// =============================================================================================================
//                                                 Main loop
void loop ()
{
  if (RunOnce==0x00){Kitt();}
  Serial.println   ("PDP8/E, PDP8/F, PDP8/M bootloader by Roland Huisman V1.6");
  Serial.println   ();
  Serial.print     ("Default program number by dipswitch ");
  Serial.println   (ReadDefaultProgramNumber(),HEX);

  Serial.print   ("Use SW switch to select program number please... ");
  ProgramNumber=(GetProgramNumber());
  Serial.println (ProgramNumber,HEX);
  Serial.println();

  if (ProgramNumber==0x00)
    {
      ProgramNumber = ReadDefaultProgramNumber();
        if (ProgramNumber == 0x00)
          {
            Serial.print("No default selected, no program will be loaded !");    
            Serial.println();  
          }
        if (ProgramNumber != 0x00)
        {
            Serial.print("Default program ");
            Serial.print(ProgramNumber,HEX);
            Serial.println(" will be loaded !");
            Serial.println();
        }
    }


  if (ProgramNumber==0x01) {LoadProgram(Program_01, sizeof(Program_01));}
  if (ProgramNumber==0x02) {LoadProgram(Program_02, sizeof(Program_02));}
  if (ProgramNumber==0x03) {LoadProgram(Program_03, sizeof(Program_03));}
  if (ProgramNumber==0x04) {LoadProgram(Program_04, sizeof(Program_04));}
  if (ProgramNumber==0x05) {LoadProgram(Program_05, sizeof(Program_05));}
  if (ProgramNumber==0x06) {LoadProgram(Program_06, sizeof(Program_06));}
  if (ProgramNumber==0x07) {LoadProgram(Program_07, sizeof(Program_07));}

  if (ProgramNumber==0x10) {LoadProgram(Program_10, sizeof(Program_10));}
  if (ProgramNumber==0x11) {LoadProgram(Program_11, sizeof(Program_11));}
  if (ProgramNumber==0x12) {LoadProgram(Program_12, sizeof(Program_12));}
  if (ProgramNumber==0x13) {LoadProgram(Program_13, sizeof(Program_13));}  
  if (ProgramNumber==0x14) {LoadProgram(Program_14, sizeof(Program_14));}
  if (ProgramNumber==0x15) {LoadProgram(Program_15, sizeof(Program_15));}
  if (ProgramNumber==0x16) {LoadProgram(Program_16, sizeof(Program_16));}  
  if (ProgramNumber==0x17) {LoadProgram(Program_17, sizeof(Program_17));}

  if (ProgramNumber==0x20) {LoadProgram(Program_20, sizeof(Program_20));}
  if (ProgramNumber==0x21) {LoadProgram(Program_21, sizeof(Program_21));}
  if (ProgramNumber==0x22) {LoadProgram(Program_22, sizeof(Program_22));}
  if (ProgramNumber==0x23) {LoadProgram(Program_23, sizeof(Program_23));}
  if (ProgramNumber==0x24) {LoadProgram(Program_24, sizeof(Program_24));}
  if (ProgramNumber==0x25) {LoadProgram(Program_25, sizeof(Program_25));}
  if (ProgramNumber==0x26) {LoadProgram(Program_26, sizeof(Program_26));}
  if (ProgramNumber==0x27) {LoadProgram(Program_27, sizeof(Program_27));}

  if (ProgramNumber==0x30) {LoadProgram(Program_30, sizeof(Program_30));}
  if (ProgramNumber==0x31) {LoadProgram(Program_31, sizeof(Program_31));}
  if (ProgramNumber==0x32) {LoadProgram(Program_32, sizeof(Program_32));}
  if (ProgramNumber==0x33) {LoadProgram(Program_33, sizeof(Program_33));}
  if (ProgramNumber==0x34) {LoadProgram(Program_34, sizeof(Program_34));}
  if (ProgramNumber==0x35) {LoadProgram(Program_35, sizeof(Program_35));}
  if (ProgramNumber==0x36) {LoadProgram(Program_36, sizeof(Program_36));}
  if (ProgramNumber==0x37) {LoadProgram(Program_37, sizeof(Program_37));}

  if (ProgramNumber==0x40) {LoadProgram(Program_40, sizeof(Program_40));}
  if (ProgramNumber==0x41) {LoadProgram(Program_41, sizeof(Program_41));}
  if (ProgramNumber==0x42) {LoadProgram(Program_42, sizeof(Program_42));}
  if (ProgramNumber==0x43) {LoadProgram(Program_43, sizeof(Program_43));}
  if (ProgramNumber==0x44) {LoadProgram(Program_44, sizeof(Program_44));}
  if (ProgramNumber==0x45) {LoadProgram(Program_45, sizeof(Program_45));}
  if (ProgramNumber==0x46) {LoadProgram(Program_46, sizeof(Program_46));}
  if (ProgramNumber==0x47) {LoadProgram(Program_47, sizeof(Program_47));}

  if (ProgramNumber==0x50) {LoadProgram(Program_50, sizeof(Program_50));}
  if (ProgramNumber==0x51) {LoadProgram(Program_51, sizeof(Program_51));}
  if (ProgramNumber==0x52) {LoadProgram(Program_52, sizeof(Program_52));}
  if (ProgramNumber==0x53) {LoadProgram(Program_53, sizeof(Program_53));}
  if (ProgramNumber==0x54) {LoadProgram(Program_54, sizeof(Program_54));}
  if (ProgramNumber==0x55) {LoadProgram(Program_55, sizeof(Program_55));}
  if (ProgramNumber==0x56) {LoadProgram(Program_56, sizeof(Program_56));}
  if (ProgramNumber==0x57) {LoadProgram(Program_57, sizeof(Program_57));}

  if (ProgramNumber==0x60) {LoadProgram(Program_60, sizeof(Program_60));}
  if (ProgramNumber==0x61) {LoadProgram(Program_61, sizeof(Program_61));}
  if (ProgramNumber==0x62) {LoadProgram(Program_62, sizeof(Program_62));}
  if (ProgramNumber==0x63) {LoadProgram(Program_63, sizeof(Program_63));}
  if (ProgramNumber==0x64) {LoadProgram(Program_64, sizeof(Program_64));}
  if (ProgramNumber==0x65) {LoadProgram(Program_65, sizeof(Program_65));}
  if (ProgramNumber==0x66) {LoadProgram(Program_66, sizeof(Program_66));}
  if (ProgramNumber==0x67) {LoadProgram(Program_67, sizeof(Program_67));}

  if (ProgramNumber==0x70) {LoadProgram(Program_70, sizeof(Program_70));}
  if (ProgramNumber==0x71) {LoadProgram(Program_71, sizeof(Program_71));}
  if (ProgramNumber==0x72) {LoadProgram(Program_72, sizeof(Program_72));}
  if (ProgramNumber==0x73) {LoadProgram(Program_73, sizeof(Program_73));}
  if (ProgramNumber==0x74) {LoadProgram(Program_74, sizeof(Program_74));}
  if (ProgramNumber==0x75) {LoadProgram(Program_75, sizeof(Program_75));}
  if (ProgramNumber==0x76) {LoadProgram(Program_76, sizeof(Program_76));}
  if (ProgramNumber==0x77) {LoadProgram(Program_77, sizeof(Program_77));}
  



// etc...


  Serial.println();

}
//                                                End of main loop
// =============================================================================================================






// =============================================================================================================
//                  Count amount of switch pulses for program number and show in address LEDs
byte GetProgramNumber()
{
  byte ToggledProgramNumber    = 0x00;
  byte OctalProgramNumber      = 0x00; // highest valid program number is 0x77
  unsigned long currentMillis  = millis();
  unsigned long previousMillis = millis();
  
  while( (ToggledProgramNumber==0) | (currentMillis-previousMillis<SwitchTimeOut) )
    {
    if ((digitalRead(r_SW) == LOW) && (pulseState == false))
      {
        pulseState = true;
        ToggledProgramNumber++;
        currentMillis = millis();
        previousMillis = millis();
        
        if(ToggledProgramNumber>1)
          {
            OctalProgramNumber = ToggledProgramNumber-1;
            OctalProgramNumber = ((OctalProgramNumber<<1)&0x70) | (OctalProgramNumber & 0x07); // ToggledProgramNumber transfer to octal
            SingleStep();
            SwitchRegister(OctalProgramNumber);
            AddresLoad();
          }
      } 
    
    else if (digitalRead(r_SW) == HIGH) 
      {
        pulseState = false;
        currentMillis = millis();        
      }
      delay(20);
    }
  SwitchRegister(0x0000);
  Serial.println();
  UndoSingleStep();
  Serial.println();
  return (OctalProgramNumber);
}











// =============================================================================================================
//                             Read dipswitches for the default boot loader to load
byte ReadDefaultProgramNumber ()
{
  byte DefaultProgramNumber = PINB;
  DefaultProgramNumber = DefaultProgramNumber >> 2;       // move bits to the right
  DefaultProgramNumber = ~DefaultProgramNumber & 0x0F;    // invert reading and wipe out extra bits
  DefaultProgramNumber = ((DefaultProgramNumber<<1)&0x70) | (DefaultProgramNumber & 0x07); // dipswitch transfer to octal
  return DefaultProgramNumber;
}





// =============================================================================================================
//                                            Load program
void LoadProgram(const word TheProgram[], int ProgramLength)
  {
  SingleStep();
  Serial.println();
  int i=0;
  ProgramLength = ProgramLength/2;         // calculate programlength in words
  for (i = 0; i < ProgramLength; i = i + 1) 
    {
      //Serial.println(pgm_read_word_near(TheProgram+i),HEX);
      SwitchRegister(pgm_read_word_near(TheProgram+i));   
      if (i==0){AddresLoad();}
      if (i==1){ExtendedAddressLoad();}
      if (i==ProgramLength-1) {AddresLoad();}
      if ((i!=0)&(i!=1)&(i!=ProgramLength-1)) {Deposit();}
      delay(SlowDown);
    }
  SwitchRegister(0x0000);
  Serial.println();
  Serial.println();
  UndoSingleStep();
  Clear();
  Continue();
  }



// =============================================================================================================
//                       Transfer the switchregisterdata to the MC23017 to deposit or load address
void SwitchRegister(word LoadData)
{
  Serial.print ("Set Switch Register to ");  
  Serial.print (LoadData,HEX);
  Serial.print (" "); 
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
    Serial.println("Go into Single Step mode");
    digitalWrite (w_STOP , HIGH);                     // Places the machine in Single step mode
  }


// =============================================================================================================
//                                          Get PDP out of Single Step mode
void UndoSingleStep()
  {
    Serial.println("Undo Single Step mode");
    digitalWrite (w_STOP , LOW);                     // Get the machine out of Single step mode
  }



// =============================================================================================================
//                                                Deposit
void Deposit()
  {
    Serial.println("Deposit");
    digitalWrite (Set_Flip_Flop     , HIGH)    ;
    digitalWrite (w_KEY_CONTROL     , HIGH)    ; 
    digitalWrite (Show_Data         , HIGH)    ; 
    digitalWrite (w_MS_IR_DISABLE   , HIGH)    ;
    Trigger_Mem_Start ();
    digitalWrite (Set_Flip_Flop     , LOW)     ; 
    digitalWrite (w_KEY_CONTROL     , LOW)     ;
    digitalWrite (Show_Data         , LOW)     ;
    digitalWrite (w_MS_IR_DISABLE   , LOW)     ;
    
  }



// =============================================================================================================
//                                               Address Load
void AddresLoad()
  {
    Serial.println("Addres Load");
    digitalWrite (w_LA_ENABLE       , HIGH)    ; // get machine ready to receive an address 
    digitalWrite (w_MS_IR_DISABLE   , HIGH)    ; // get machine ready to receive an address 
    digitalWrite (Set_Flip_Flop     , HIGH)    ; // get machine ready to receive an address 
    digitalWrite (Show_Data         , HIGH)    ;                                            
    digitalWrite (Exam              , HIGH)    ;                                            
    Trigger_Adres_Latch ();
    digitalWrite (w_LA_ENABLE       , LOW)     ; // get machine out of address latch mode
    digitalWrite (w_MS_IR_DISABLE   , LOW)     ; // get machine out of address latch mode
    digitalWrite (Set_Flip_Flop     , LOW)     ; // get machine out of address latch mode
    digitalWrite (Show_Data         , LOW)     ;
    digitalWrite (Exam              , LOW)     ;
  }




// =============================================================================================================
//                                       Extended Memory Address Load
void ExtendedAddressLoad()
  {
    Serial.println("Extended Memory Address Load");
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
    Serial.print("Clear machine");
    if (digitalRead(r_RUN) == HIGH)         // check if machine is not running
      {
        Trigger_Initialize ();              // give 400ns positive pulse on Initialize_H
        Serial.print(" >> Machine cleared");
      }
    if (digitalRead(r_RUN) == LOW)          // Warning, machine seems to run ! Not cleared
      {
        Serial.print(" >> ERROR !! Machine not cleared due RUN state !");
      }
    Serial.println();  
  }

  


// =============================================================================================================
//                                      Continue, set machine in RUN mode
void Continue()
  {
    Serial.print("Continue, set machine in RUN mode");
    if (digitalRead(r_RUN) == LOW)          // Warning, machine seems to run ! Not started
      {
        Serial.print(" >> ERROR !! Machine not started due RUN state !");
      }
    if (digitalRead(r_RUN) == HIGH)         // check if machine is not running
      {
        Trigger_Mem_Start ();               // give 400ns positive pulse on Initialize_H
        Serial.print(" >> Running");
      }
    Serial.println();  
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



// =============================================================================================================
//                                           Kitt scanner at turn on PDP
void Kitt()
  {
    SingleStep();
    RunOnce++;
    unsigned long scanspeed = 35;

        SwitchRegister(0x0001);
        AddresLoad();
        delay(scanspeed);
        SwitchRegister(0x0003);
        AddresLoad();
        delay(scanspeed);
        SwitchRegister(0x0007);
        AddresLoad();
        delay(scanspeed);    
        SwitchRegister(0x0016);
        AddresLoad();
        delay(scanspeed);        
        SwitchRegister(0x0034);
        AddresLoad();
        delay(scanspeed);    
        SwitchRegister(0x0070);
        AddresLoad();
        delay(scanspeed);
        SwitchRegister(0x0160);
        AddresLoad();
        delay(scanspeed);
        SwitchRegister(0x0340);
        AddresLoad();
        delay(scanspeed);
        SwitchRegister(0x0700);
        AddresLoad();
        delay(scanspeed);
        SwitchRegister(0x1600);
        AddresLoad();
        delay(scanspeed);
        SwitchRegister(0x3400);
        AddresLoad();
        delay(scanspeed);
        SwitchRegister(0x7000);
        AddresLoad();
        delay(scanspeed);
        SwitchRegister(0x6000);
        AddresLoad();
        delay(scanspeed);
        SwitchRegister(0x4000);
        AddresLoad();
        delay(scanspeed);
          // and back
        SwitchRegister(0x6000);
        AddresLoad();
        delay(scanspeed);
        SwitchRegister(0x7000);
        AddresLoad();
        delay(scanspeed);
        SwitchRegister(0x3400);
        AddresLoad();
        delay(scanspeed);
        SwitchRegister(0x1600);
        AddresLoad();
        delay(scanspeed);
        SwitchRegister(0x0700);
        AddresLoad();
        delay(scanspeed);
        SwitchRegister(0x0340);
        AddresLoad();
        delay(scanspeed);
        SwitchRegister(0x0160);
        AddresLoad();
        delay(scanspeed);
        SwitchRegister(0x0070);
        AddresLoad();
        delay(scanspeed);
        SwitchRegister(0x0034);
        AddresLoad();
        delay(scanspeed);
        SwitchRegister(0x0016);
        AddresLoad();
        delay(scanspeed);
        SwitchRegister(0x0007);
        AddresLoad();
        delay(scanspeed);
        SwitchRegister(0x0003);
        AddresLoad();
        delay(scanspeed);
        SwitchRegister(0x0001);
        AddresLoad();
        delay(scanspeed);
        SwitchRegister(0x0000);
        AddresLoad();
        delay(scanspeed);
    UndoSingleStep();
  }
