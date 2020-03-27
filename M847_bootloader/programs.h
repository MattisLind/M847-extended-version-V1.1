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

// ================================== FROM HERE THE BOOTSTRAPS CAN BE ENTERED =================================

// The program numbers in group 0x and 1x can be set by the dipstitches as default bootloader.
// This means, when you toggle the SW switch just once down and up again, the delected default is loaded.
// If an other program is desired to load, just toggle the SW switch more often and watch the address lights for the program number.
  
  //group 0x
  // Program 0 does not exist, this is preseved to load a default trough the dip switches
  const PROGMEM word Program_01[] = {0x0017, 0x0000, 0x0017, 0x7240, 0x3017, 0x1045, 0x6416, 0x6411, 0x5024, 0x6402, 0x6401, 0x5027, 0x6406, 0x7106, 0x7006, 0x7510, 0x5000, 0x7006, 0x6401, 0x5037, 0x6404, 0x3417, 0x5026, 0x7605, 0x0100, 0x0020}; // OS8 disk server 40/41
  const PROGMEM word Program_02[] = {0x0024, 0x0000, 0x7126, 0x1060, 0x6751, 0x7201, 0x4053, 0x4053, 0x7104, 0x6755, 0x5054, 0x6754, 0x7450, 0x7610, 0x5046, 0x1060, 0x7041, 0x1061, 0x3060, 0x5024, 0x6751, 0x4053, 0x3002, 0x2050, 0x5047, 0x0000, 0x6753, 0x5033, 0x6752, 0x5453, 0x7024, 0x6030, 0x0033}; //RX8 RX01 bootloader (M8357 RX8E card at 675x)
  const PROGMEM word Program_03[] = {0x0020, 0x0000, 0x1061, 0x1046, 0x0060, 0x3061, 0x7327, 0x1061, 0x6751, 0x7301, 0x4053, 0x4053, 0x7004, 0x6755, 0x5054, 0x6754, 0x7450, 0x5020, 0x1061, 0x6751, 0x1061, 0x0046, 0x1032, 0x3060, 0x0360, 0x4053, 0x3002, 0x2050, 0x5047, 0x0000, 0x6753, 0x5033, 0x6752, 0x5453, 0x0420, 0x0020, 0x0033}; //RX28 RX02 bootloader (M8357 RX8E card at 675x)
  const PROGMEM word Program_04[] = {0x0023, 0x0000, 0x6007, 0x6751, 0x6745, 0x5025, 0x7200, 0x6733, 0x5031, 0x0023}; // RK05 (RK8E) bootloader MI8-ED RK8
  const PROGMEM word Program_05[] = {0x7300, 0x0000, 0x1312, 0x4312, 0x4312, 0x6773, 0x5303, 0x6777, 0x3726, 0x2326, 0x5303, 0x5732, 0x2000, 0x1300, 0x6774, 0x6771, 0x5315, 0x6776, 0x0331, 0x1327, 0x7640, 0x5315, 0x2321, 0x5712, 0x7354, 0x7756, 0x7747, 0x0077, 0x7400, 0x7300}; //TU56 (TD8E) MI8-EH
  const PROGMEM word Program_06[] = {0x7554, 0x0000, 0x7600, 0x6774, 0x1374, 0x6766, 0x6771, 0x5360, 0x7240, 0x1354, 0x3773, 0x1354, 0x3772, 0x1375, 0x6766, 0x5376, 0x7754, 0x7755, 0x0600, 0x0220, 0x6771, 0x5376, 0x7777, 0x7554}; // TU56 TC08 dec tape MI8-EC
  const PROGMEM word Program_07[] = {0x4000, 0x0000, 0x1237, 0x1206, 0x6704, 0x6706, 0x6703, 0x5204, 0x7264, 0x6702, 0x7610, 0x3211, 0x3636, 0x1205, 0x6704, 0x6706, 0x6701, 0x5216, 0x7002, 0x7430, 0x1636, 0x7022, 0x3636, 0x7420, 0x2236
                                    , 0x2235, 0x5215, 0x7346, 0x7002, 0x3235, 0x5201, 0x7737, 0x3557, 0x7730, 0x4000}; //  TU60 (TA8E) CAPS-8 or OS/8 setup cassettes


  //group 1x
  const PROGMEM word Program_10[] = {0x7737, 0x0000, 0x6014, 0x0776, 0x7326, 0x1337, 0x2376, 0x5340, 0x6011, 0x5356, 0x3361, 0x1361, 0x3371, 0x1345, 0x3357, 0x1345, 0x3367, 0x6032, 0x6031, 0x5357, 0x6036, 0x7106, 0x7006, 0x7510, 0x5374, 0x7006, 0x6031, 0x5367, 0x6034, 0x7420, 0x3776, 0x3376, 0x5356, 0x0000, 0x7737}; // PC04 PC8E papertape MI8-EA
  const PROGMEM word Program_11[] = {0x7756, 0x0000, 0x6032, 0x6031, 0x5357, 0x6036, 0x7106, 0x7006, 0x7510, 0x5357, 0x7006, 0x6031, 0x5367, 0x6034, 0x7420, 0x3776, 0x3376, 0x5356, 0x7756}; // RIM loader  03/04
  const PROGMEM word Program_12[] = {0x7612, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x3212, 0x4260, 0x1300, 0x7750, 0x5237, 0x2212, 0x7040, 0x5227
                                    ,0x1212, 0x7640, 0x5230, 0x1214, 0x0274, 0x1341, 0x7510, 0x2226, 0x7750, 0x5626, 0x1214, 0x0256, 0x1257, 0x3213, 0x5230, 0x0070, 0x6201, 0x0000, 0x0000, 0x6031, 0x5262, 0x6036, 0x3214, 0x1214
                                    ,0x5660, 0x6011, 0x5270, 0x6016, 0x5265, 0x0300, 0x4343, 0x7041, 0x1215, 0x7402, 0x6032, 0x6014, 0x6214, 0x1257, 0x3213, 0x7240, 0x7700, 0x1353, 0x1352, 0x3261, 0x4226, 0x5313, 0x3215, 0x1213
                                    ,0x3336, 0x1214, 0x3376, 0x4260, 0x3355, 0x4226, 0x5275, 0x4343, 0x7420, 0x5336, 0x3216, 0x1376, 0x1355, 0x1215, 0x5315, 0x6201, 0x3616, 0x2216, 0x7600, 0x5332, 0x0000, 0x1376, 0x7106, 0x7006
                                    ,0x7006, 0x1355, 0x5743, 0x5262, 0x0006, 0x0000, 0x0000, 0x6032, 0x6031, 0x5357, 0x6036, 0x7106, 0x7006, 0x7510, 0x5357, 0x7006, 0x6031, 0x5367, 0x6034, 0x7420, 0x3776, 0x3376, 0x5356, 0x7776, 0x5301, 0x7777}; //BIN LOADER 03/04
  const PROGMEM word Program_13[] = {0x7612, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x3212, 0x4260, 0x1300, 0x7750, 0x5237, 0x2212, 0x7040, 0x5227
                                    ,0x1212, 0x7640, 0x5230, 0x1214, 0x0274, 0x1341, 0x7510, 0x2226, 0x7750, 0x5626, 0x1214, 0x0256, 0x1257, 0x3213, 0x5230, 0x0070, 0x6201, 0x0000, 0x0000, 0x6401, 0x5262, 0x6406, 0x3214, 0x1214
                                    ,0x5660, 0x6011, 0x5270, 0x6016, 0x5265, 0x0300, 0x4343, 0x7041, 0x1215, 0x7402, 0x6402, 0x6014, 0x6214, 0x1257, 0x3213, 0x7240, 0x7700, 0x1353, 0x1352, 0x3261, 0x4226, 0x5313, 0x3215, 0x1213
                                    ,0x3336, 0x1214, 0x3376, 0x4260, 0x3355, 0x4226, 0x5275, 0x4343, 0x7420, 0x5336, 0x3216, 0x1376, 0x1355, 0x1215, 0x5315, 0x6201, 0x3616, 0x2216, 0x7600, 0x5332, 0x0000, 0x1376, 0x7106, 0x7006
                                    ,0x7006, 0x1355, 0x5743, 0x5262, 0x0006, 0x0000, 0x0000, 0x6402, 0x6401, 0x5357, 0x6406, 0x7106, 0x7006, 0x7510, 0x5357, 0x7006, 0x6401, 0x5367, 0x6404, 0x7420, 0x3776, 0x3376, 0x5356, 0x7776, 0x5301, 0x7777}; //BIN LOADER 40/41
  const PROGMEM word Program_14[] = {0x0200, 0x0000, 0x1220, 0x4221, 0x7040, 0x1217, 0x3217, 0x1217, 0x6053, 0x4221, 0x1220, 0x6054, 0x6052, 0x5212, 0x6055, 0x3220, 0x5200, 0x3777, 0x0006, 0x0000
                                    ,0x3236, 0x7404, 0x7041, 0x3237, 0x1236, 0x7100, 0x7510, 0x7020, 0x7010, 0x2237, 0x5227, 0x5621, 0x0000, 0x0000, 0x6144, 0x5240, 0x0200}; // Kaleidoscope
  const PROGMEM word Program_15[] = {0x7756, 0x0000, 0x7771, 0x6014, 0x6011, 0x5360, 0x7106, 0x7106, 0x6012, 0x7420, 0x5357, 0x5756, 0x4356, 0x3373, 0x4356, 0x7770}; //MI8-EE Typeset
  const PROGMEM word Program_16[] = {0x7737, 0x0000, 0x6007, 0x7604, 0x7510, 0x3343, 0x6766, 0x6771, 0x5344, 0x1376, 0x5343, 0x7600, 0x6603, 0x6622, 0x5352, 0x5752, 0x7577, 0x6032, 0x6031, 0x5357, 0x6036, 0x7106, 0x7006, 0x7510, 0x5357, 0x7006, 0x6031, 0x5367, 0x6034, 0x7420, 0x3776, 0x3376, 0x5356, 0x0200, 0x7737}; //MI8-EF Edu sys low
  const PROGMEM word Program_17[] = {0x7737, 0x0000, 0x6007, 0x7604, 0x7510, 0x3343, 0x6766, 0x6771, 0x5344, 0x1376, 0x5343, 0x7600, 0x6603, 0x6622, 0x5352, 0x5752, 0x7577, 0x6014, 0x6011, 0x5357, 0x6016, 0x7106, 0x7006, 0x7510, 0x5374, 0x7006, 0x6011, 0x5367, 0x6016, 0x7420, 0x3776, 0x3376, 0x5357, 0x0220, 0x7737}; //MI8-EG Edu sys high


                                    
  // from here programs are not selectable by the dipswitches. These can be loaded by toggeling the SW switch or pushbutton on the PCB only.
  //group 2x
  const PROGMEM word Program_20[] = {0x0200, 0x0000, 0x7200, 0x7100, 0x7040, 0x7020, 0x7020, 0x7010, 0x7004, 0x7012, 0x7006, 0x7001, 0x7001, 0x7002, 0x7402, 0x0200}; //group 1 microinstructions
  const PROGMEM word Program_21[] = {0x0200, 0x0000, 0x7300, 0x7440, 0x7402, 0x7430, 0x7402, 0x7020, 0x7420, 0x7402, 0x7001, 0x7450, 0x7402, 0x7510, 0x7402, 0x7410, 0x7402, 0x7012, 0x7500, 0x7402, 0x7404, 0x7402, 0x0200}; //group 1 microinstructions
  const PROGMEM word Program_22[] = {0x0200, 0x0000, 0x7240, 0x7001, 0x7640, 0x7402, 0x7120, 0x7010, 0x7510, 0x7410, 0x7402, 0x7001, 0x7002, 0x1202, 0x7420, 0x7402, 0x7402, 0x0200}; // Operate instructions
  const PROGMEM word Program_23[] = {0x0200, 0x0000, 0x7500, 0x3300, 0x7001, 0x2300, 0x5202, 0x7440, 0x7402, 0x7402, 0x0200}; // ISZ instructions
  const PROGMEM word Program_24[] = {0x0200, 0x0000, 0x7300, 0x3300, 0x3204, 0x4204, 0x0000, 0x1204, 0x7041, 0x1215, 0x7440, 0x7402, 0x2300, 0x5202, 0x7402, 0x0204, 0x0200}; // JMS instruction
  const PROGMEM word Program_25[] = {0x0200, 0x0000, 0x5210, 0x7402, 0x5206, 0x7402, 0x5212, 0x7402, 0x5204, 0x7402, 0x5204, 0x7402, 0x2300, 0x5200, 0x7402, 0x0200}; // JMP instruction
  const PROGMEM word Program_26[] = {0x0200, 0x0000, 0x7001, 0x2300, 0x5201, 0x5200, 0x0200}; //AC increment
  const PROGMEM word Program_27[] = {0x7777, 0x0000, 0x0000, 0x7300, 0x1007, 0x7040, 0x3007, 0x1007, 0x3410, 0x5000, 0x0000, 0x0011, 0x7777}; //checker board runs endless MD alters between 0000 and 7777

  // group 3x
  const PROGMEM word Program_30[] = {0x0020, 0x0000, 0x7300, 0x7020, 0x7420, 0x5025, 0x5027, 0x1032, 0x7410, 0x1033, 0x3410, 0x5021, 0x5252, 0x2525, 0x0035, 0x0020}; //checker board runs endless MD alters between 5252 and 2525
  const PROGMEM word Program_31[] = {0x0000, 0x0000, 0x7604, 0x6046, 0x6041, 0x5002, 0x5000, 0x0000}; // print character in switch register
  const PROGMEM word Program_32[] = {0x0000, 0x0000, 0x7001, 0x6046, 0x6041, 0x5002, 0x5000, 0x0000}; // console print test 03/04
  const PROGMEM word Program_33[] = {0x0000, 0x0000, 0x6032, 0x6031, 0x5001, 0x6036, 0x6046, 0x6041, 0x5005, 0x5001, 0x0000}; // Echo test for one terminal at 03/04
  const PROGMEM word Program_34[] = {0x0200, 0x0000, 0x7300, 0x1205, 0x6412, 0x6401, 0x5203, 0x0210, 0x6406, 0x5203, 0x7000, 0x7000, 0x7000, 0x5206, 0x7000, 0x7000, 0x7000, 0x6405, 0x6404, 0x5203, 0x0200}; // echo 1-4 terminals
  const PROGMEM word Program_35[] = {0x0200, 0x0000, 0x7001, 0x6026, 0x6021, 0x5202, 0x5202, 0x0200}; // PC04 Punch alternating 1's and 0's
  const PROGMEM word Program_36[] = {0x0200, 0x0000, 0x7300, 0x6016, 0x6011, 0x5202, 0x5200, 0x0200}; // PC04 read the papertape (doesn't load to core, just for test purposes)
  const PROGMEM word Program_37[] = {0x0000, 0x0000, 0x6032, 0x6031, 0x5001, 0x6036, 0x5001, 0x0000}; // Place received character into AC from terminal at 03/04

  // group 4x
  const PROGMEM word Program_40[] = {0x3000, 0x0000, 0x0016, 0x3015, 0x3601, 0x2201, 0x2200, 0x5202, 0x3203, 0x3204, 0x3205, 0x1201, 0x3014, 0x3212, 0x3414, 0x3000}; // memory wipe field 0 Vince
  const PROGMEM word Program_41[] = {0x0200, 0x0000, 0x7001, 0x2300, 0x5201, 0x5200, 0x0200}; //AC increment. REPLACE THIS CODE WITH YOUR OWN BOOTSTRAP IF YOU WANT TO.
  const PROGMEM word Program_42[] = {0x0200, 0x0000, 0x7001, 0x2300, 0x5201, 0x5200, 0x0200}; //AC increment. REPLACE THIS CODE WITH YOUR OWN BOOTSTRAP IF YOU WANT TO.
  const PROGMEM word Program_43[] = {0x0200, 0x0000, 0x7001, 0x2300, 0x5201, 0x5200, 0x0200}; //AC increment. REPLACE THIS CODE WITH YOUR OWN BOOTSTRAP IF YOU WANT TO.
  const PROGMEM word Program_44[] = {0x0200, 0x0000, 0x7001, 0x2300, 0x5201, 0x5200, 0x0200}; //AC increment. REPLACE THIS CODE WITH YOUR OWN BOOTSTRAP IF YOU WANT TO.
  const PROGMEM word Program_45[] = {0x0200, 0x0000, 0x7001, 0x2300, 0x5201, 0x5200, 0x0200}; //AC increment. REPLACE THIS CODE WITH YOUR OWN BOOTSTRAP IF YOU WANT TO.
  const PROGMEM word Program_46[] = {0x0200, 0x0000, 0x7001, 0x2300, 0x5201, 0x5200, 0x0200}; //AC increment. REPLACE THIS CODE WITH YOUR OWN BOOTSTRAP IF YOU WANT TO.
  const PROGMEM word Program_47[] = {0x0200, 0x0000, 0x7001, 0x2300, 0x5201, 0x5200, 0x0200}; //AC increment. REPLACE THIS CODE WITH YOUR OWN BOOTSTRAP IF YOU WANT TO.

  // group 5x
  const PROGMEM word Program_50[] = {0x0200, 0x0000, 0x7001, 0x2300, 0x5201, 0x5200, 0x0200}; //AC increment. REPLACE THIS CODE WITH YOUR OWN BOOTSTRAP IF YOU WANT TO.
  const PROGMEM word Program_51[] = {0x0200, 0x0000, 0x7001, 0x2300, 0x5201, 0x5200, 0x0200}; //AC increment. REPLACE THIS CODE WITH YOUR OWN BOOTSTRAP IF YOU WANT TO.
  const PROGMEM word Program_52[] = {0x0200, 0x0000, 0x7001, 0x2300, 0x5201, 0x5200, 0x0200}; //AC increment. REPLACE THIS CODE WITH YOUR OWN BOOTSTRAP IF YOU WANT TO.
  const PROGMEM word Program_53[] = {0x0200, 0x0000, 0x7001, 0x2300, 0x5201, 0x5200, 0x0200}; //AC increment. REPLACE THIS CODE WITH YOUR OWN BOOTSTRAP IF YOU WANT TO.
  const PROGMEM word Program_54[] = {0x0200, 0x0000, 0x7001, 0x2300, 0x5201, 0x5200, 0x0200}; //AC increment. REPLACE THIS CODE WITH YOUR OWN BOOTSTRAP IF YOU WANT TO.
  const PROGMEM word Program_55[] = {0x0200, 0x0000, 0x7001, 0x2300, 0x5201, 0x5200, 0x0200}; //AC increment. REPLACE THIS CODE WITH YOUR OWN BOOTSTRAP IF YOU WANT TO.
  const PROGMEM word Program_56[] = {0x0200, 0x0000, 0x7001, 0x2300, 0x5201, 0x5200, 0x0200}; //AC increment. REPLACE THIS CODE WITH YOUR OWN BOOTSTRAP IF YOU WANT TO.
  const PROGMEM word Program_57[] = {0x0200, 0x0000, 0x7001, 0x2300, 0x5201, 0x5200, 0x0200}; //AC increment. REPLACE THIS CODE WITH YOUR OWN BOOTSTRAP IF YOU WANT TO.

  // group 6x
  const PROGMEM word Program_60[] = {0x0200, 0x0000, 0x7001, 0x2300, 0x5201, 0x5200, 0x0200}; //AC increment. REPLACE THIS CODE WITH YOUR OWN BOOTSTRAP IF YOU WANT TO.
  const PROGMEM word Program_61[] = {0x0200, 0x0000, 0x7001, 0x2300, 0x5201, 0x5200, 0x0200}; //AC increment. REPLACE THIS CODE WITH YOUR OWN BOOTSTRAP IF YOU WANT TO.
  const PROGMEM word Program_62[] = {0x0200, 0x0000, 0x7001, 0x2300, 0x5201, 0x5200, 0x0200}; //AC increment. REPLACE THIS CODE WITH YOUR OWN BOOTSTRAP IF YOU WANT TO.
  const PROGMEM word Program_63[] = {0x0200, 0x0000, 0x7001, 0x2300, 0x5201, 0x5200, 0x0200}; //AC increment. REPLACE THIS CODE WITH YOUR OWN BOOTSTRAP IF YOU WANT TO.
  const PROGMEM word Program_64[] = {0x0200, 0x0000, 0x7001, 0x2300, 0x5201, 0x5200, 0x0200}; //AC increment. REPLACE THIS CODE WITH YOUR OWN BOOTSTRAP IF YOU WANT TO.
  const PROGMEM word Program_65[] = {0x0200, 0x0000, 0x7001, 0x2300, 0x5201, 0x5200, 0x0200}; //AC increment. REPLACE THIS CODE WITH YOUR OWN BOOTSTRAP IF YOU WANT TO.
  const PROGMEM word Program_66[] = {0x0200, 0x0000, 0x7001, 0x2300, 0x5201, 0x5200, 0x0200}; //AC increment. REPLACE THIS CODE WITH YOUR OWN BOOTSTRAP IF YOU WANT TO.
  const PROGMEM word Program_67[] = {0x0200, 0x0000, 0x7001, 0x2300, 0x5201, 0x5200, 0x0200}; //AC increment. REPLACE THIS CODE WITH YOUR OWN BOOTSTRAP IF YOU WANT TO.

  // group 7x
  const PROGMEM word Program_70[] = {0x0200, 0x0000, 0x7001, 0x2300, 0x5201, 0x5200, 0x0200}; //AC increment. REPLACE THIS CODE WITH YOUR OWN BOOTSTRAP IF YOU WANT TO.
  const PROGMEM word Program_71[] = {0x0200, 0x0000, 0x7001, 0x2300, 0x5201, 0x5200, 0x0200}; //AC increment. REPLACE THIS CODE WITH YOUR OWN BOOTSTRAP IF YOU WANT TO.
  const PROGMEM word Program_72[] = {0x0200, 0x0000, 0x7001, 0x2300, 0x5201, 0x5200, 0x0200}; //AC increment. REPLACE THIS CODE WITH YOUR OWN BOOTSTRAP IF YOU WANT TO.
  const PROGMEM word Program_73[] = {0x0200, 0x0000, 0x7001, 0x2300, 0x5201, 0x5200, 0x0200}; //AC increment. REPLACE THIS CODE WITH YOUR OWN BOOTSTRAP IF YOU WANT TO.
  const PROGMEM word Program_74[] = {0x0200, 0x0000, 0x7001, 0x2300, 0x5201, 0x5200, 0x0200}; //AC increment. REPLACE THIS CODE WITH YOUR OWN BOOTSTRAP IF YOU WANT TO.
  const PROGMEM word Program_75[] = {0x0200, 0x0000, 0x7001, 0x2300, 0x5201, 0x5200, 0x0200}; //AC increment. REPLACE THIS CODE WITH YOUR OWN BOOTSTRAP IF YOU WANT TO.
  const PROGMEM word Program_76[] = {0x0200, 0x0000, 0x7001, 0x2300, 0x5201, 0x5200, 0x0200}; //AC increment. REPLACE THIS CODE WITH YOUR OWN BOOTSTRAP IF YOU WANT TO.
  const PROGMEM word Program_77[] = {0x0200, 0x0000, 0x7001, 0x2300, 0x5201, 0x5200, 0x0200}; //AC increment. REPLACE THIS CODE WITH YOUR OWN BOOTSTRAP IF YOU WANT TO.
                          
                               
// ====================================== END OF BOOTSTRAPLIST =================================================