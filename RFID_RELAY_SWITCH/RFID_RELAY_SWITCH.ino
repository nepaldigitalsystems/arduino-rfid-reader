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
#include <EEPROM.h>
/*******************************************************************************
*                          Type & Macro Definitions
*******************************************************************************/
#define RFID_RELAY_RELAY_PIN                     8
#define RFID_RELAY_RST_PIN                       9    //Pin 9 is for the RC522 reset
#define RFID_RELAY_SDA_PIN                       10   //Pin 10 is the SDA (SDA) of RC522 module
#define RFID_RELAY_RFID_CHECK_INTERVAL           1000
#define RFID_RELAY_SERIAL_DEBUG                  Serial
#define RFID_RELAY_SIZE_RFID                     4
#define RFID_RELAY_STAT_ADDR                     0                    
// #define RFID_RELAY_DEVICE1
#define RFID_RELAY_DEVICE2
/*******************************************************************************
*                          Static Data Definitions
*******************************************************************************/
typedef enum {
  RELAY_STAT_OFF,
  RELAY_STAT_ON
} RFID_RELAY_STAT;

MFRC522 RFID_RELAY_MRFC522(RFID_RELAY_SDA_PIN, RFID_RELAY_RST_PIN); //Create new MFRC522 object
//boolean RFID_RELAY_RELAY_STAT = false;
byte RFID_RELAY_read_uuid[RFID_RELAY_SIZE_RFID];                        //This will store the ID each time we read RFID tag

#ifdef RFID_RELAY_DEVICE1
byte RFID_RELAY_ID_DEV_CARD[RFID_RELAY_SIZE_RFID] = {0xB3, 0x81, 0xA7, 0xA9};
byte RFID_RELAY_ID_DEV_TAG[RFID_RELAY_SIZE_RFID] = {0xA3, 0x27, 0x77, 0xA6};
#else if RFID_RELAY_DEVICE2
byte RFID_RELAY_ID_DEV_CARD[RFID_RELAY_SIZE_RFID] = {0x83, 0x42, 0x7B, 0xA9};
byte RFID_RELAY_ID_DEV_TAG[RFID_RELAY_SIZE_RFID] = {0x23, 0x8C, 0x0A, 0xA6};

#endif 

uint32_t RFID_RELAY_rfid_read_timekeeper_past_event = 0;       

RFID_RELAY_STAT rfid_relay_status = RELAY_STAT_OFF;

/*******************************************************************************
*                          Static Function Definitions
*******************************************************************************/
/**
 * @brief Compare the 4 bytes of the users and the received ID
 * 
 * @param array1 First array with 4 byte RFID 
 * @param array2 Second array with 4 byte RFID
 * @return boolean True if matched False if not matched 
 */
boolean RFID_RELAY_compare_ids(byte array1[], byte array2[])
{
  if(array1[0] != array2[0])return(false);
  if(array1[1] != array2[1])return(false);
  if(array1[2] != array2[2])return(false);
  if(array1[3] != array2[3])return(false);
  return(true);
}

/**
 * @brief Turn ON the Relay connected to \ref RFID_RELAY_RELAY_PIN
 * 
 */
void RFID_RELAY_relay_switch_on() {
  digitalWrite(RFID_RELAY_RELAY_PIN, LOW);
}

/**
 * @brief Turn OFF the Relay connected to \ref RFID_RELAY_RELAY_PIN
 * 
 */
void RFID_RELAY_relay_switch_off() {
  digitalWrite(RFID_RELAY_RELAY_PIN, HIGH);
}

/**
 * @brief Arduino Setup function
 * Here we write the statement to be executed only once when program starts
 * 
 */

void setup() {

  RFID_RELAY_SERIAL_DEBUG.begin(9600);
  SPI.begin();                              //Start a new SPI bus
  RFID_RELAY_MRFC522.PCD_Init();            //Start the MFRC522  
  
  pinMode(RFID_RELAY_RELAY_PIN, OUTPUT);     //Set digital pin D7 to be the buzzer OUTPU

  rfid_relay_status = (RFID_RELAY_STAT)EEPROM.read(RFID_RELAY_STAT_ADDR);
  
  #ifdef RFID_RELAY_SERIAL_DEBUG
  RFID_RELAY_SERIAL_DEBUG.print("Rfid Relay Status: ");
  if(rfid_relay_status == RELAY_STAT_OFF) RFID_RELAY_SERIAL_DEBUG.println("OFF");
  else RFID_RELAY_SERIAL_DEBUG.println("ON");
  #endif // RFID_RELAY_SERIAL_DEBUG
  
  if(rfid_relay_status == RELAY_STAT_OFF) {
    RFID_RELAY_relay_switch_off();
  } else {
    RFID_RELAY_relay_switch_on();
  }

  wdt_disable(); //Disable WDT
  delay(3000);
  wdt_enable(WDTO_2S); //Enable WDT with a timeout of 2 seconds
}

/**
 * @brief Arduino Loop function
 * This function is a forever (infinite loop), here we write the 
 * statements to be executed for all the time.
 */
void loop() {
    
    if((millis() - RFID_RELAY_rfid_read_timekeeper_past_event) >= RFID_RELAY_RFID_CHECK_INTERVAL) 
    {
      RFID_RELAY_rfid_read_timekeeper_past_event = millis();
      // Check if there are any new ID card in front of the sensor
      if (RFID_RELAY_MRFC522.PICC_IsNewCardPresent()) 
      {  
        //Select the found card
        if ( RFID_RELAY_MRFC522.PICC_ReadCardSerial()) 
        {     
              // We store the read ID into 4 bytes with a for loop and display them                 
              for (uint8_t i = 0; i < RFID_RELAY_MRFC522.uid.size; i++) {
                RFID_RELAY_read_uuid[i] = RFID_RELAY_MRFC522.uid.uidByte[i];       
              } 
                #ifdef RFID_RELAY_SERIAL_DEBUG
                RFID_RELAY_SERIAL_DEBUG.println(F("\nThe UID tag is:"));
                RFID_RELAY_SERIAL_DEBUG.print(F("In hex: "));
                RFID_RELAY_SERIAL_DEBUG.print(" 0x");RFID_RELAY_SERIAL_DEBUG.print(RFID_RELAY_read_uuid[0],HEX);  
                RFID_RELAY_SERIAL_DEBUG.print(" 0x");RFID_RELAY_SERIAL_DEBUG.print(RFID_RELAY_read_uuid[1],HEX);
                RFID_RELAY_SERIAL_DEBUG.print(" 0x");RFID_RELAY_SERIAL_DEBUG.print(RFID_RELAY_read_uuid[2],HEX);
                RFID_RELAY_SERIAL_DEBUG.print(" 0x");RFID_RELAY_SERIAL_DEBUG.println(RFID_RELAY_read_uuid[3],HEX);
                #endif // RFID_RELAY_SERIAL_DEBUG
              //Compare the UID and default User1
              if(RFID_RELAY_compare_ids(RFID_RELAY_read_uuid,RFID_RELAY_ID_DEV_TAG) 
                                        || RFID_RELAY_compare_ids(RFID_RELAY_read_uuid,RFID_RELAY_ID_DEV_CARD))
              {              
                  if(rfid_relay_status == RELAY_STAT_ON) {
                     RFID_RELAY_relay_switch_off();
                     rfid_relay_status = RELAY_STAT_OFF;                  
                   } else  {
                     RFID_RELAY_relay_switch_on();
                     rfid_relay_status = RELAY_STAT_ON;
                   }

                  EEPROM.update(RFID_RELAY_STAT_ADDR, (uint8_t)rfid_relay_status);

                  #ifdef RFID_RELAY_SERIAL_DEBUG      
                  RFID_RELAY_SERIAL_DEBUG.print("Rfid Relay Status: ");            
                  if(rfid_relay_status == RELAY_STAT_OFF) RFID_RELAY_SERIAL_DEBUG.println("OFF");
                  else RFID_RELAY_SERIAL_DEBUG.println("ON");
                  #endif // RFID_RELAY_SERIAL_DEBUG
              }
              else
              {
                #ifdef RFID_RELAY_SERIAL_DEBUG
                RFID_RELAY_SERIAL_DEBUG.println(F("...Invalid User..."));
                #endif // RFID_RELAY_SERIAL_DEBUG
              }
            // Halt PICC
            RFID_RELAY_MRFC522.PICC_HaltA();
            // Stop encryption on PCD
            RFID_RELAY_MRFC522.PCD_StopCrypto1();                          
        }
          
      }
      else
      {
        #ifdef RFID_RELAY_SERIAL_DEBUG
        RFID_RELAY_SERIAL_DEBUG.print(F("."));
        #endif // RFID_RELAY_SERIAL_DEBUG
      }
    }  
  wdt_reset(); //Reset the watchdog
}
/*******************************************************************************
*                          End of File
*******************************************************************************/
