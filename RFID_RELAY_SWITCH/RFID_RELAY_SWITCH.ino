/*******************************************************************************
* (C) Copyright 2018-2023 ;  Nepal Digital Systems Pvt. Ltd., Kathmandu, Nepal.
* The attached material and the information contained therein is proprietary to
* Nepal Digital Systems Pvt. Ltd. and is issued only under strict confidentiality
* arrangements.It shall not be used, reproduced, copied in whole or in part,
* adapted,modified, or disseminated without a written license of Nepal Digital  
* Systems Pvt. Ltd.It must be returned to Nepal Digital Systems Pvt. Ltd. upon 
* its first request.
*
*  File Name           : RFID_RELAY_SWITCH.ino
*
*  Description         : Read RFID tag and operate the relay.
*
*  Change history      : 
*
*     Author        Date          Ver                 Description
*  ------------    --------       ---   --------------------------------------
*  Riken Maharjan  26 Feb 2023    1.0               Initial Creation
*  
*******************************************************************************/

/*******************************************************************************
*                          Include Files
*******************************************************************************/
#include <SPI.h>
#include <MFRC522.h>   
#include <avr/wdt.h>
/*******************************************************************************
*                          Type & Macro Definitions
*******************************************************************************/
#define relay_pin                     8
#define RST_PIN                       9    //Pin 9 is for the RC522 reset
#define SDA_PIN                       10   //Pin 10 is the SDA (SDA) of RC522 module
#define RFID_CHECK_INTERVAL           1000
#define SERIAL_DEBUG                  Serial
/*******************************************************************************
*                          Static Data Definitions
*******************************************************************************/
MFRC522 mfrc522(SDA_PIN, RST_PIN); //Create new MFRC522 object
boolean System_On = false;
byte ActualUID[4];                          // This will store the ID each time we read RFID tag
byte USER1[4]= {0x83, 0x42, 0x7B, 0xA9} ;   // Authorized User ID code 
//byte USER1[4]= {0x23, 0x8C, 0x0A, 0xA6} ; // Authorized User ID code 

uint32_t time_prev = 0;       
/*******************************************************************************
*                          Static Function Definitions
*******************************************************************************/
//Compare the 4 bytes of the users and the received ID
boolean compareArray(byte array1[],byte array2[])
{
  if(array1[0] != array2[0])return(false);
  if(array1[1] != array2[1])return(false);
  if(array1[2] != array2[2])return(false);
  if(array1[3] != array2[3])return(false);
  return(true);
}
void relay_switch_on() {
  digitalWrite(relay_pin, LOW);
}

void relay_switch_off() {
  digitalWrite(relay_pin, HIGH);
}
//-----------------------------------------------------------------------------------------
void setup() {

  SERIAL_DEBUG.begin(9600);
  SPI.begin();                  //Start a new SPI bus
  mfrc522.PCD_Init();           //Start the MFRC522  
  
  pinMode(relay_pin,OUTPUT);    //Set digital pin D7 to be the buzzer OUTPUT
  relay_switch_off();
  #ifdef SERIAL_DEBUG
  (System_On) ? SERIAL_DEBUG.println(F("System ON")) : SERIAL_DEBUG.println(F("System OFF"));
  SERIAL_DEBUG.println(F("Enter Card"));
  #endif // SERIAL_DEBUG

  wdt_disable(); //Disable WDT
  delay(3000);
  wdt_enable(WDTO_2S); //Enable WDT with a timeout of 2 seconds
}
//-----------------------------------------------------------------------------------------
void loop() {
    
    if((millis() - time_prev) >= RFID_CHECK_INTERVAL) 
    {
      time_prev = millis();
      // Check if there are any new ID card in front of the sensor
      if (mfrc522.PICC_IsNewCardPresent()) 
      {  
        //Select the found card
        if ( mfrc522.PICC_ReadCardSerial()) 
        {     
              // We store the read ID into 4 bytes with a for loop and display them                 
              for (byte i = 0; i < mfrc522.uid.size; i++) {
                ActualUID[i] = mfrc522.uid.uidByte[i];       
              } 
                #ifdef SERIAL_DEBUG
                SERIAL_DEBUG.println(F("\nThe UID tag is:"));
                SERIAL_DEBUG.print(F("In hex: "));
                SERIAL_DEBUG.print(" 0x");SERIAL_DEBUG.print(ActualUID[0],HEX);  
                SERIAL_DEBUG.print(" 0x");SERIAL_DEBUG.print(ActualUID[1],HEX);
                SERIAL_DEBUG.print(" 0x");SERIAL_DEBUG.print(ActualUID[2],HEX);
                SERIAL_DEBUG.print(" 0x");SERIAL_DEBUG.println(ActualUID[3],HEX);
                #endif // SERIAL_DEBUG
              //Compare the UID and default User1
              if(compareArray(ActualUID,USER1))
              {
                  System_On = !(System_On); // oFF -> ON // ON -> OFF
                  (System_On) ? relay_switch_on() : relay_switch_off();
                  #ifdef SERIAL_DEBUG
                  (System_On) ? SERIAL_DEBUG.println(F("System ON")) : SERIAL_DEBUG.println(F("System OFF"));
                  #endif // SERIAL_DEBUG
              }
              else
              {
                #ifdef SERIAL_DEBUG
                SERIAL_DEBUG.println(F("...Invalid User..."));
                #endif // SERIAL_DEBUG
              }
            // Halt PICC
            mfrc522.PICC_HaltA();
            // Stop encryption on PCD
            mfrc522.PCD_StopCrypto1();                          
        }
          
      }
      else
      {
        #ifdef SERIAL_DEBUG
        SERIAL_DEBUG.print(F("."));
        #endif // SERIAL_DEBUG
      }
    }
  
  wdt_reset(); //Reset the watchdog
}
/*******************************************************************************
*                          End of File
*******************************************************************************/
