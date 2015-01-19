/* Arduino Interface to the PSC05 X10 Receiver.                       BroHogan 3/24/09
 * SETUP: X10 PSC05/TW523 RJ11 to Arduino (timing for 60Hz)
 * - RJ11 pin 1 (BLK) -> Pin 2 (Interrupt 0) = Zero Crossing
 * - RJ11 pin 2 (RED) -> GND
 * - RJ11 pin 3 (GRN) -> Pin 4 = Arduino receive
 * - RJ11 pin 4 (YEL) -> Pin 5 = Arduino transmit (via X10 Lib)
 * NOTES:
 * - Must detach interrup when transmitting with X10 Lib
 */
 
 /*
  SD card datalogger and Arduino TFT for display

  The circuit:
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK -  pin 13
 
 SD CS is pin 3. 
 
 LCD CS pin 10. must be left as an output or the SD library
 functions will not work.
 
 pins for TFT definition for the Uno
 
 cs   10
 rs   9
 rst  8
 bkl  to 3.3v
 MOSI - pin 11
 MISO - pin 12
 CLK -  pin 13
 
 */
 
 
#include "Arduino.h"                  // this is needed to compile with Rel. 0013
#include <TFT.h>  // Arduino LCD library
#include <Adafruit_GFX.h>
#include <SPI.h>
// #include <SD.h>
#include <SdFat.h>
#include <x10.h>                       // X10 lib is used for transmitting X10
#include <x10constants.h>              // X10 Lib constants

#define RPT_SEND 2
#define nRPT_SEND 1
#define ZCROSS_PIN     2               // BLK pin 1 of PSC05
#define RCVE_PIN       4               // GRN pin 3 of PSC05
#define TRANS_PIN      5               // YEL pin 4 of PSC05
#define LED_PIN        13              // for testing


// pin TFT definition for the Uno
#define cs   10
#define rs   9
#define rst  8  


// SD CS is pin 3. 
// LCD CS pin 10. must be left as an output or the SD library
// functions will not work.


const int SdChipSelect = 3;
const int LCDchipSelect = 10;

#define FILE_BASE_NAME "file"



// create an instance of the library
TFT TFTscreen = TFT(cs, rs, rst);

const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
char fileName[13] = FILE_BASE_NAME "00.CSV";

// File system object.
SdFat sd;

// Log file.
SdFile file;

void LCD_Serial_display(String message, int mesgDelay, int xcor, int ycor);


x10 SX10= x10(ZCROSS_PIN,TRANS_PIN,RCVE_PIN,LED_PIN);// set up a x10 library instance:

void setup() {
  
   // Put this line at the beginning of every sketch that uses the GLCD:
  TFTscreen.begin();

  // clear the screen with a black background
  TFTscreen.background(0, 0, 0);
  delay(250);  // pause

  // write the static text to the screen 
  // set the font color
//  TFTscreen.stroke(255,255,255);  
  // set the font size
  TFTscreen.setTextSize(1);
  
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
   }
  
    // char array to print to the screen

String screenSizeHW = "screen height & width = ";
char screenSizeC[40];   


  // Get the Width and Height.
  // set it up for the serial print
  screenSizeHW += String(TFTscreen.height());
  screenSizeHW += " , ";
  screenSizeHW += String(TFTscreen.width());
  
  // Set it up for the TFT LCD disaplay
  screenSizeHW.toCharArray(screenSizeC, sizeof(screenSizeHW));
  
  // Print to to both Serial and TFT
 LCD_Serial_display(screenSizeHW, 2000 ,0,0);

 LCD_Serial_display("Initializing SD card...", 250 ,0,0);
  
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
   pinMode(LCDchipSelect, OUTPUT);   // LCD CS 
   pinMode(SdChipSelect, OUTPUT);   // SD CS
  
  // see if the card is present and can be initialized:
  if (!sd.begin(SdChipSelect, SPI_HALF_SPEED)) {
    
  LCD_Serial_display("Card failed, or not present\n ", 1000 ,0,0); 
 
     // don't do anything more:
    return;
  }

    // Create CSV header 
  if (file.open(fileName, O_CREAT | O_WRITE | O_EXCL)) {
    file.println("X10 SC, HouseCode, UnitCode, FunctionCole");
    file.close();
  }
  else
  {
   // problem opening file 
    LCD_Serial_display("Error opening file ", 250,0,0); 
  // don't do anything more:
    return;  
  }
  
 // float fs = 0.000512*volFree*vol.blocksPerCluster();
  
  LCD_Serial_display("card initialized.\n ", 250,0,0); 
  
  LCD_Serial_display("x10 receive/send test starting", 250,0,0);
  
   // Print only to TFT
  TFTscreen.stroke(255,255,255);
  // write the text to the top left corner of the screen
  TFTscreen.text("Value read: ",0,0);
  // set the font size very large for the loop
  TFTscreen.setTextSize(1);
     
}

// A simple test program that demonstrates integrated send/receive
// prints X10 input, then get P1  on/off if unit code on input was 1
void loop(){
  
// make a string for assembling the data to log:
  String dataString = "";
  
    // char array to print to the screen
  String startCodeString = "";
  String houseCodeString = "";
  String unitCodeString = ""; 
  String cmndCodeString = "";   

  char TFTPrintoutstartCode[4];
  char TFTPrintouthouseCode[5];  
  char TFTPrintoutunitCode[5];
  char TFTPrintoutcmndCode[12];
  
  byte startCode = SX10.sc();
  byte houseCode = SX10.houseCode();
  byte unitCode = SX10.unitCode();
  byte cmndCode = SX10.cmndCode();  
  
  
  
  // create string for serial transmition 
    dataString += "SC -"; 
    dataString += startCode;
    dataString += " HOUSE-";
    dataString += houseCode;
    dataString += " UNIT-";
    dataString += unitCode;
    dataString += " CMND ";
    dataString += cmndCode;    
    
 // create char array for tft
    startCodeString = String(startCode);
    houseCodeString = String(houseCode);
    unitCodeString = String(unitCode);
    cmndCodeString = String(cmndCode);
    
    
    startCodeString.toCharArray(TFTPrintoutstartCode, 4);
    houseCodeString.toCharArray(TFTPrintouthouseCode, 5);
    unitCodeString.toCharArray(TFTPrintoutunitCode, 5);    
    cmndCodeString.toCharArray(TFTPrintoutcmndCode, 12);    
    
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  // if the file is available, write to it:
  if (file.open(fileName, O_CREAT | O_WRITE | O_EXCL)) {
    file.println(dataString);
    file.close();   

    // Print to to both Serial and TFT
     // set the font color
    TFTscreen.stroke(255,255,255);
    // print the sensor value
    TFTscreen.text(TFTPrintoutstartCode,0, 20);
    TFTscreen.text(TFTPrintouthouseCode,0, 30);
    TFTscreen.text(TFTPrintoutunitCode,0, 40); 
    TFTscreen.text(TFTPrintoutcmndCode,0, 40);  
    Serial.println(dataString);
    
    // wait for 5 Sec
    delay(5000);
    
    // erase the text you just wrote
    TFTscreen.stroke(0,0,0);
    TFTscreen.text(TFTPrintoutstartCode,0, 20);
    TFTscreen.text(TFTPrintouthouseCode,0, 30);
    TFTscreen.text(TFTPrintoutunitCode,0, 40); 
    TFTscreen.text(TFTPrintoutcmndCode,0, 40);     
   // if the file isn't open, pop up an error:
  }
  else {

    LCD_Serial_display("Problem opening file ", 5000,0,0);    

  }  
  
SX10.write(HOUSE_P,UNIT_1,RPT_SEND);
SX10.writeXTBIIR(16,STATUS_REQUEST,RPT_SEND);
SX10.debug();                       // print out the received command
SX10.reset();
 delay(1000);
}



// User Functions 

void LCD_Serial_display(String message, int mesgDelay, int xcor, int ycor) {
  
     char msg[60];
     message.toCharArray(msg, sizeof(message));
  // Print to to both Serial and TFT
   TFTscreen.stroke(255,255,255);
   
    // Ok SD Card initialised
   TFTscreen.text(msg,xcor,ycor);
   
    // wait for a moment
   delay(mesgDelay);
   
   
   
    // erase the text you just wrote
   TFTscreen.stroke(0,0,0);
   TFTscreen.text(msg,xcor,ycor);
   Serial.println(sizeof(message));
   Serial.println(message);
   
}



