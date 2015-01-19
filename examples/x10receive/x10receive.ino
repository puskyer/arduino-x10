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
 bkl  to gnd
 MOSI - pin 11
 MISO - pin 12
 CLK -  pin 13
 
 */
 
 
#include "Arduino.h"                  // this is needed to compile with Rel. 0013
#include <TFT.h>  // Arduino LCD library
#include <SPI.h>
#include <SdFat.h>
#include <x10.h>                       // X10 lib is used for transmitting X10
#include <x10constants.h>              // X10 Lib constants

#define RPT_SEND       2
#define nRPT_SEND      1
#define ZCROSS_PIN     2               // BLK pin 1 of PSC05
#define RCVE_PIN       4               // GRN pin 3 of PSC05
#define TRANS_PIN      5               // YEL pin 4 of PSC05
#define LED_PIN        13              // for testing


// pin TFT definition for the Uno
#define cs   10     //LCD Chipselect
#define dc   9       // LCD Data or Command (rs)
#define rst  8  
#define bkl  7      //LCD Black Light


// SD CS is pin 3. 
// LCD CS pin 10. must be left as an output or the SD library
// functions will not work.


const int SdChipSelect = 6;
const int LCDchipSelect = 10;

String message = " ";
int mesgDelay = 0;
int xcor = 0;
int ycor = 0;

#define FILE_BASE_NAME "file"

const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
char fileName[13] = FILE_BASE_NAME "00.CSV";

// File system object.
SdFat sd;

// Log file.
SdFile file;

x10 SX10= x10(ZCROSS_PIN,TRANS_PIN,RCVE_PIN,LED_PIN);   // set up a x10 library instance:

TFT TFTscreen = TFT(cs, dc, rst);                       // set up a TFTscreen library instance:

// User Functions 
void LCD_Serial_display(String message, int mesgDelay, int xcor, int ycor) {
  
   char msg[sizeof(message)];
   message.toCharArray(msg, sizeof(message));

   Serial.print(sizeof(message));
   Serial.print(" ");
   Serial.println(message);

   // Print to to both Serial and TFT
   TFTscreen.stroke(255,255,255);
   TFTscreen.text(msg,xcor,ycor);
   
   // wait for a moment
   delay(mesgDelay);
   
   // erase the text first
   TFTscreen.stroke(0,0,0);
   TFTscreen.text(msg,xcor,ycor);
      
}

void setup() {
  
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
 pinMode(LCDchipSelect, OUTPUT);  // LCD CS 
 pinMode(SdChipSelect, OUTPUT);   // SD CS
 pinMode(bkl, OUTPUT);            // backlight select
  
   // Put this line at the beginning of every sketch that uses the GLCD:
  TFTscreen.begin();

  // turn on the backlight
  digitalWrite(bkl, LOW);
 
  // clear the screen with a black background
  TFTscreen.background(0, 0, 0);
  // set the font color
  TFTscreen.stroke(255,255,255);  
  // set the font size
  TFTscreen.setTextSize(1);
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
   }
 
  String screenSizeHW = "screen height & width = ";
      
  // Get the Width and Height.
  // set it up for the serial print
  screenSizeHW += String(TFTscreen.height());
  screenSizeHW += " , ";
  screenSizeHW += String(TFTscreen.width());

  // Print to to both Serial and TFT
  LCD_Serial_display(screenSizeHW,500,0,0);

  LCD_Serial_display("Initializing SD card...",250,0,0);
 
  // see if the card is present and can be initialized:
  if (!sd.begin(SdChipSelect, SPI_HALF_SPEED)) {
    
  LCD_Serial_display("Card Init failed or not present\n ",5000,0,0); 
 
     // don't do anything more:
    return;
  }

    // Open File and Create CSV header 
  if (file.open(fileName, O_RDWR | O_CREAT | O_AT_END)) {
    file.println("X10 SC, HouseCode, UnitCode, FunctionCole");
  }
  else
  {
   // problem opening file 
    LCD_Serial_display("Error opening file ",5000,0,0); 
  // don't do anything more:
    return;  
  }
  
 // float fs = 0.000512*volFree*vol.blocksPerCluster();
  
  LCD_Serial_display("SD card initialized.\n ",500,0,0); 
  
  LCD_Serial_display("x10 receive/send test starting",500,0,0);

  // clear the screen with a black background
  TFTscreen.background(0, 0, 0);
  // write the static text to the screen 
  // set the font color
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

  char TFTPrintoutstartCode[10];
  memset(TFTPrintoutstartCode,0,sizeof(TFTPrintoutstartCode));
  char TFTPrintouthouseCode[10];
  memset(TFTPrintouthouseCode,0,sizeof(TFTPrintouthouseCode));
  char TFTPrintoutunitCode[10];
  memset(TFTPrintoutunitCode,0,sizeof(TFTPrintoutunitCode));
  char TFTPrintoutcmndCode[20];
  memset(TFTPrintoutcmndCode,0,sizeof(TFTPrintoutcmndCode));

//   get the codes from X10

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

// display it to serial port

    Serial.println(dataString);

// create char array for tft

    startCodeString = "SC = ";
    startCodeString += String(startCode);
    houseCodeString = "HC = ";
    houseCodeString += String(houseCode);
    unitCodeString = "UC = ";
    unitCodeString += String(unitCode);
    cmndCodeString = "CC = ";
    cmndCodeString += String(cmndCode);
    
    Serial.println(startCodeString);
    Serial.println(houseCodeString);
    Serial.println(unitCodeString);
    Serial.println(cmndCodeString);
        
    startCodeString.toCharArray(TFTPrintoutstartCode, sizeof(startCodeString));
    houseCodeString.toCharArray(TFTPrintouthouseCode, sizeof(houseCodeString));
    unitCodeString.toCharArray(TFTPrintoutunitCode, sizeof(unitCodeString));    
    cmndCodeString.toCharArray(TFTPrintoutcmndCode, sizeof(cmndCodeString)); 
    // Print to TFT
    
     // erase the old text 
    TFTscreen.stroke(0,0,0);
    TFTscreen.text(TFTPrintoutstartCode,0, 20);
    TFTscreen.text(TFTPrintouthouseCode,0, 30);
    TFTscreen.text(TFTPrintoutunitCode,0, 40); 
    TFTscreen.text(TFTPrintoutcmndCode,0, 50); 
    
     // set the font color
    
    TFTscreen.stroke(255,255,255);
    
    // print the sensor value
    
    TFTscreen.text(TFTPrintoutstartCode,0, 20);
    TFTscreen.text(TFTPrintouthouseCode,0, 30);
    TFTscreen.text(TFTPrintoutunitCode,0, 40); 
    TFTscreen.text(TFTPrintoutcmndCode,0, 50);  

// write to SD
    
   file.println(dataString);
    // Force data to SD and update the directory entry to avoid data loss.
    if (!file.sync() || file.getWriteError()) 
    {

    String prob_string = "Sync or write problem with file ";
    prob_string += fileName;
    
    LCD_Serial_display(prob_string, 5000,0,120);
    digitalWrite(bkl, HIGH); 
    return;
    }

    
    // wait for 5 Sec
    delay(5000);
    
     
    //SX10.write(HOUSE_P,UNIT_1,RPT_SEND);
    //SX10.writeXTBIIR(16,STATUS_REQUEST,RPT_SEND);
    SX10.debug();                       // print out the received command
    SX10.reset();
    
}







