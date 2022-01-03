/************************************************************************************************************
 *
 *  Copyright (C) 2018 Dirkjan Kaper
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, If not, see <http://www.gnu.org/licenses/>.
 *
 ************************************************************************************************************

DESCRIPTION:
This is a script for a Loconet enabled MP3 Player

It plays a number of MP3 tracks, depending on the Loconet Sensor message 
Loconet:
- TX pin D7
- RX pin D8
You MUST connect the RX input to the AVR ICP pin which on an Arduino UNO is digital pin 8.
The TX output can be any Arduino pin, but the LocoNet library defaults to digital pin 6 for TX

LNCV programming gives some options
LNCV[0] : default lncv module address 0
LNCV[1] : Article number
LNCV[2] : Start Address on which to respond to
LNCV[3] : Onumber of tracks to distinguish (LNCV(2) + LNCV(3) -1 = max address to process

/************************************************************************************************************/
#include <LocoNet.h>
#include <EEPROM.h>

// DFPlayer mini fast
#include <DFPlayerMini_Fast.h>
#include <SoftwareSerial.h>

//Verbindungspins zum DFPlayer mini
#define DFPlayer_RX A3
#define DFPlayer_TX A2

#define volume_PIN A6                // Anschluss fuer den Poti zur Lautstärkeregelung
int sensorValue = 0;  //value read from volume_PIN from 0 to 1023
uint8_t volumeValue = 0;  //value for volume from 0 to 30

SoftwareSerial mySoftwareSerial(DFPlayer_RX, DFPlayer_TX); // RX, TX
#define Console Serial           // command processor input/output stream

//DFRobotDFPlayerMini myDFPlayer;
DFPlayerMini_Fast myDFPlayer;

bool bUseCallback = false; // use callbacks?
bool bUseSynch = false;   // use synchronous? 

// uncomment this to debug
//#define DEBUG

#define VERSION 1
#define ARTNR 1531
#define LNCV_COUNT 4

// Loconet
#define LOCONET_TX_PIN 7

uint16_t lncv[LNCV_COUNT];
uint16_t lnaddr = 0;
uint16_t numTracks = 0;

static lnMsg *LnPacket;

uint8_t track = 0;

LocoNetCVClass lnCV;

boolean modeProgramming = false;

#include "lncv.h"
/*************************************************************************/
/*          Setup                                                        */
/*************************************************************************/ 

void setup() {

  #ifdef DEBUG
    Serial.print(" setup... ");
  #endif
  // Initialize Loconet Interface
  LocoNet.init(LOCONET_TX_PIN);
 
 // Verbindung zum DFPlayer mini
  mySoftwareSerial.begin(9600);
  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with DFPlayer
   while(true){
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
 
  //read in volume poti and set DFPlayer volume
  sensorValue = analogRead(volume_PIN);
  volumeValue = map(sensorValue, 0, 1023, 0, 30); // Map analog input to volume of DFPlayer

  myDFPlayer.volume(volumeValue);  //Set volume value. From 0 to 30
  
  /*
   * load settings from eeprom
   */
  loadSettings();
  lnaddr = lncv[2];
  numTracks = lncv[3];

  myDFPlayer.stop();
} 

/*************************************************************************/
/*          Program Loop                                                 */
/*************************************************************************/ 
void loop() {
//read in volume poti and set DFPlayer volume
  sensorValue = analogRead(volume_PIN);
  if (!(volumeValue == map(sensorValue, 0, 1023, 0, 30))) { //only on change
    volumeValue = map(sensorValue, 0, 1023, 0, 30);
    //DEBUG_PRINT("Snd: volume: ");
    //DEBUG_PRINTLN(volumeValue);
    myDFPlayer.volume(volumeValue);  //Set volume value. From 0 to 30
  }
  //receive LnPacket
  LnPacket = LocoNet.receive() ; // Check for any received Loconet packets

   // process events
   if (LnPacket) {
   //LocoNet.processSwitchSensorMessage(LnPacket);
    uint8_t packetConsumed(LocoNet.processSwitchSensorMessage(LnPacket));
    if (packetConsumed == 0) { //it's probably a LNCV message
      packetConsumed = lnCV.processLNCVMessage(LnPacket);
    }
  }  
 
  //check to ring wecker
  if (track > 0) {
    //start sound
      Serial.print(F(", "));
      Serial.println(track);
      myDFPlayer.loop(track);
      //myDFPlayer.startRepeatPlay();    
  }
}

void notifySensor( uint16_t Address, uint8_t State )
{
  if ((Address>=lnaddr) & (Address <= (lnaddr+numTracks -1))) {
    if (State==0x10) {
        track = Address-lnaddr; //calculate index of MP3 file (from 1)
      }
      else if (State==0x00) {
        track = 0;
      }
  }  
}


/*****************************************************************************/
/* This call-back function is called from LocoNet.processSwitchSensorMessage */
/* for all Switch Report messages                                            */
/*****************************************************************************/
void notifySwitchReport( uint16_t Address, uint8_t Output, uint8_t Direction ){
}
/*****************************************************************************/
/* This call-back function is called from LocoNet.processSwitchSensorMessage */
/* for all Switch State messages                                             */
/*****************************************************************************/
void notifySwitchState( uint16_t Address, uint8_t Output, uint8_t Direction ){
}
/*****************************************************************************/
/* This call-back function is called from LocoNet.processSwitchSensorMessage */
/* for all Switch Request messages                                           */
/*****************************************************************************/
void notifySwitchRequest( uint16_t Address, uint8_t Output, uint8_t Direction ){ 
}
