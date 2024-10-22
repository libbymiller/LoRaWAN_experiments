//#ifndef _RADIOLIB_EX_LORAWAN_CONFIG_H
//#define _RADIOLIB_EX_LORAWAN_CONFIG_H

#pragma once

#include <Arduino.h>
#include <RadioLib.h>


// #define VERSION_1
// #define HIGH_VOLTAGE

#ifndef _PINNUM
#define _PINNUM(port, pin)    ((port)*32 + (pin))
#endif

#if defined(VERSION_1)
#define ePaper_Miso         _PINNUM(1,3)
#else
#define ePaper_Miso         _PINNUM(1,6)
#endif
#define ePaper_Mosi         _PINNUM(0,29)
#define ePaper_Sclk         _PINNUM(0,31)
#define ePaper_Cs           _PINNUM(0,30)
#define ePaper_Dc           _PINNUM(0,28)
#define ePaper_Rst          _PINNUM(0,2)
#define ePaper_Busy         _PINNUM(0,3)
#define ePaper_Backlight    _PINNUM(1,11)

#define LoRa_Miso           _PINNUM(0,23)
#define LoRa_Mosi           _PINNUM(0,22)
#define LoRa_Sclk           _PINNUM(0,19)
#define LoRa_Cs             _PINNUM(0,24)
#define LoRa_Rst            _PINNUM(0,25)
#if defined(VERSION_1)
#define LoRa_Dio0           _PINNUM(1,1)
#else
#define LoRa_Dio0           _PINNUM(0,22)
#endif
#define LoRa_Dio1           _PINNUM(0,20)
#define LoRa_Dio2           //_PINNUM(0,3)
#define LoRa_Dio3           _PINNUM(0,21)
#define LoRa_Dio4           //_PINNUM(0,3)
#define LoRa_Dio5           //_PINNUM(0,3)
#define LoRa_Busy           _PINNUM(0,17)


#define Flash_Cs            _PINNUM(1,15)
#define Flash_Miso          _PINNUM(1,13)
#define Flash_Mosi          _PINNUM(1,12)
#define Flash_Sclk          _PINNUM(1,14)

#define Touch_Pin           _PINNUM(0,11)
#define Adc_Pin             _PINNUM(0,4)

#define SDA_Pin             _PINNUM(0,26)
#define SCL_Pin             _PINNUM(0,27)

#define RTC_Int_Pin         _PINNUM(0,16)

#define Gps_Rx_Pin          _PINNUM(1,9)
#define Gps_Tx_Pin          _PINNUM(1,8)

#if defined(VERSION_1)
#define Gps_Wakeup_Pin      _PINNUM(1,2)
#define Gps_pps_Pin         _PINNUM(1,4)
#else
#define Gps_Wakeup_Pin      _PINNUM(1,2)
#define Gps_Reset_Pin       _PINNUM(1,5)
#define Gps_pps_Pin         _PINNUM(1,4)
#endif



#define UserButton_Pin      _PINNUM(1,10)

#if defined(VERSION_1)
#define Power_Enable_Pin    _PINNUM(0,12)
#else
#define Power_Enable_Pin    _PINNUM(0,12)
//#define Power_Enable1_Pin   _PINNUM(0,13)
#endif


#if defined(VERSION_1)
#define GreenLed_Pin        _PINNUM(0,13)
#define RedLed_Pin          _PINNUM(0,14)
#define BlueLed_Pin         _PINNUM(0,15)
#else
#define GreenLed_Pin        _PINNUM(1,1)
#define RedLed_Pin          _PINNUM(1,3)
#define BlueLed_Pin         _PINNUM(0,14)
#endif

#define SerialMon           Serial
#define SerialGPS           Serial2

#define MONITOR_SPEED       115200


SPIClass        *rfPort    = nullptr;
SX1262          radio      = nullptr; 


//SX1276 radio = new Module(18, 26, 14, 33);
//SX1262 radio = new Module(LoRa_Cs, LoRa_Dio1, LoRa_Rst, LoRa_Busy);

// how often to send an uplink - consider legal & FUP constraints - see notes
const uint32_t uplinkIntervalSeconds = 2UL * 60UL;    // minutes x seconds

// joinEUI - previous versions of LoRaWAN called this AppEUI
// for development purposes you can use all zeros - see wiki for details
#define RADIOLIB_LORAWAN_JOIN_EUI  xxxx

// the Device EUI & two keys can be generated on the TTN console 
#ifndef RADIOLIB_LORAWAN_DEV_EUI   // Replace with your Device EUI with 0x first
#define RADIOLIB_LORAWAN_DEV_EUI   xxx
#endif

#ifndef RADIOLIB_LORAWAN_APP_KEY   // Replace with your App Key in MSB format
#define RADIOLIB_LORAWAN_APP_KEY   xxx
#ifndef RADIOLIB_LORAWAN_NWK_KEY   // Put your Nwk Key here in MSB format
#define RADIOLIB_LORAWAN_NWK_KEY   xxx
#endif

// for the curious, the #ifndef blocks allow for automated testing &/or you can
// put your EUI & keys in to your platformio.ini - see wiki for more tips

// regional choices: EU868, US915, AU915, AS923, IN865, KR920, CN780, CN500
const LoRaWANBand_t Region = EU868;
const uint8_t subBand = 0;  // For US915, change this to 2, otherwise leave on 0


// copy over the EUI's & keys in to the something that will not compile if incorrectly formatted
uint64_t joinEUI =   RADIOLIB_LORAWAN_JOIN_EUI;
uint64_t devEUI  =   RADIOLIB_LORAWAN_DEV_EUI;
uint8_t appKey[] = { RADIOLIB_LORAWAN_APP_KEY };
uint8_t nwkKey[] = { RADIOLIB_LORAWAN_NWK_KEY };

// create the LoRaWAN node
LoRaWANNode node(&radio, &Region, subBand);

// helper function to display any issues
void debug(bool isFail, const __FlashStringHelper* message, int state, bool Freeze) {
  if (isFail) {
    Serial.print(message);
    Serial.print("(");
    Serial.print(state);
    Serial.println(")");
    while (Freeze);
  }
}

// helper function to display a byte array
void arrayDump(uint8_t *buffer, uint16_t len) {
  for(uint16_t c = 0; c < len; c++) {
    char b = buffer[c];
    if(b < 0x10) { Serial.print('0'); }
    Serial.print(b, HEX);
  }
  Serial.println();
}

#endif


