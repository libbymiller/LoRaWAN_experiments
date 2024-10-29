/*

  much stolen from https://github.com/jarkman/WhereOTechno/blob/main/WhereOTechno.ino
  and the rest from https://github.com/jgromes/RadioLib/tree/master/examples/LoRaWAN

  radiolib now at 7.0.2 - see https://github.com/jgromes/RadioLib/blob/master/examples/LoRaWAN/LoRaWAN_Starter/LoRaWAN_Starter.ino

*/

#include "config.h"
#include <CayenneLPP.h>

//242 is max, from 
//https://github.com/HelTecAutomation/CubeCell-Arduino/blob/master/libraries/LoraWan102/src/loramac/Commissioning.h
//in turn from here https://www.thethingsnetwork.org/forum/t/example-ttn-code-for-heltec-htcc-ab02a-with-gps-in-us915/46093

//9 from here https://github.com/myDevicesIoT/CayenneLPP (for GPS)
//looking at https://avbentem.github.io/airtime-calculator/ttn/eu868/9
//if we used it for 1 hour, we could do messages every 30 secs (145 / day, at least 21 secs apart)
//currently at 2 mins (see config.h)

CayenneLPP lpp(9);

uint32_t my_id;
double lat = 0;
double lng = 0;
double alt = 0;

#include <TinyGPS++.h>
#include <Wire.h>

#include <GxEPD.h> // GxEPD by Jean-Marc Zingg
#include <GxDEPG0150BN/GxDEPG0150BN.h>  // 1.54" b/w 

#include GxEPD_BitmapExamples
#include <Fonts/FreeMonoBold12pt7b.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

SPIClass        *dispPort  = nullptr;
GxIO_Class      *io        = nullptr;
GxEPD_Class     *display   = nullptr;

// The TinyGPS++ object
TinyGPSPlus *gps;
uint32_t lastGpsFixMillis = 0;

//lorawan things
uint32_t lastLoRaWANMillis = 0;
uint32_t fixSendInterval = 30000;

void configVDD(void);
void boardInit();
void setupDisplay();
bool setupGPS();
void loopGPS();
void loopLoRAWAN();

void setup() {
  Serial.begin(115200);
  delay(4000);
  Serial.println("starting up");
  boardInit();
  delay(4000);

  printText("Board","initialised","");

//loraWAN startup stuff

  Serial.println(F("Initialise the radio"));
  int16_t state = radio.begin();
  debug(state != RADIOLIB_ERR_NONE, F("Initialise radio failed"), state, true);

  // Setup the OTAA session information
  state = node.beginOTAA(joinEUI, devEUI, nwkKey, appKey);
  debug(state != RADIOLIB_ERR_NONE, F("Initialise node failed"), state, true);
  if(state != RADIOLIB_ERR_NONE){
     printText("Initialise node","failed, state:",String(state));
  }

  Serial.println(F("Join ('login') the LoRaWAN Network"));
  state = node.activateOTAA();
  debug(state != RADIOLIB_LORAWAN_NEW_SESSION, F("Join failed"), state, true);
  if(state != RADIOLIB_LORAWAN_NEW_SESSION){
     Serial.println("failed to join lorawan "+String(state));
     printText("join lorawan","failed, state:",String(state));
  }

  Serial.println("Ready, Mac address is "+String(my_id));
  printText("Ready","Mac address:",String(my_id));
}

void boardInit()
{
    Serial.println("board init");

#ifdef HIGH_VOLTAGE
    configVDD();
#endif
    Serial.print("1\n");

    SerialMon.begin(MONITOR_SPEED);

    SerialMon.println("Start\n");

    uint32_t reset_reason;
    sd_power_reset_reason_get(&reset_reason);
    SerialMon.print("sd_power_reset_reason_get:");
    SerialMon.println(reset_reason, HEX);

    pinMode(Power_Enable_Pin, OUTPUT);
    digitalWrite(Power_Enable_Pin, HIGH);

    pinMode(ePaper_Backlight, OUTPUT);
    //enableBacklight(true); //ON backlight
    enableBacklight(false); //OFF  backlight

    pinMode(GreenLed_Pin, OUTPUT);
    pinMode(RedLed_Pin, OUTPUT);
    pinMode(BlueLed_Pin, OUTPUT);

    pinMode(UserButton_Pin, INPUT_PULLUP);
    pinMode(Touch_Pin, INPUT_PULLUP);

    //pinMode( Adc_Pin, ANALOG_IN );

    int i = 10;
    while (i--) {
        digitalWrite(GreenLed_Pin, !digitalRead(GreenLed_Pin));
        digitalWrite(RedLed_Pin, !digitalRead(RedLed_Pin));
        digitalWrite(BlueLed_Pin, !digitalRead(BlueLed_Pin));
        delay(300);
    }
    digitalWrite(GreenLed_Pin, HIGH);
    digitalWrite(RedLed_Pin, HIGH);
    digitalWrite(BlueLed_Pin, HIGH);

    //set my id
    my_id =  getMacAddress();
    Serial.print("my_id is ");  
    Serial.println(my_id);

    Serial.print("2\n");
    setupDisplay();

    Serial.print("3\n");
    setupGPS();

    Serial.print("4\n");
    setupLoRa();

    display->update();
    delay(500);
    Serial.print("6\n");

}

void setupDisplay()
{
    dispPort = new SPIClass(
        /*SPIPORT*/NRF_SPIM2,
        /*MISO*/ ePaper_Miso,
        /*SCLK*/ePaper_Sclk,
        /*MOSI*/ePaper_Mosi);

    io = new GxIO_Class(
        *dispPort,
        /*CS*/ ePaper_Cs,
        /*DC*/ ePaper_Dc,
        /*RST*/ePaper_Rst);

    display = new GxEPD_Class(
        *io,
        /*RST*/ ePaper_Rst,
        /*BUSY*/ ePaper_Busy);

    dispPort->begin();
    display->init(/*115200*/);
    display->setRotation(3);
    display->fillScreen(GxEPD_WHITE);
    display->setTextColor(GxEPD_BLACK);
    display->setFont(&FreeMonoBold12pt7b);
}

bool setupLoRa()
{
    rfPort = new SPIClass(
        /*SPIPORT*/NRF_SPIM3,
        /*MISO*/ LoRa_Miso,
        /*SCLK*/LoRa_Sclk,
        /*MOSI*/LoRa_Mosi);
    rfPort->begin();

    SPISettings spiSettings;

    radio = new Module(LoRa_Cs, LoRa_Dio1, LoRa_Rst, LoRa_Busy, *rfPort, spiSettings);
    return true;
}



bool setupGPS()
{
    Serial.println("GPS initialising");

    SerialMon.println("[GPS] Initializing ... ");
    SerialMon.flush();
#ifndef PCA10056
    SerialGPS.setPins(Gps_Rx_Pin, Gps_Tx_Pin);
#endif
    SerialGPS.begin(9600);
    SerialGPS.flush();
    pinMode(Gps_pps_Pin, INPUT);
    pinMode(Gps_Wakeup_Pin, OUTPUT);
    digitalWrite(Gps_Wakeup_Pin, HIGH);
    delay(10);
    pinMode(Gps_Reset_Pin, OUTPUT);
    digitalWrite(Gps_Reset_Pin, HIGH); delay(10);
    digitalWrite(Gps_Reset_Pin, LOW); delay(10);
    digitalWrite(Gps_Reset_Pin, HIGH);
    gps = new TinyGPSPlus();
    return true;
}

void printText(String label, String data1, String data2){
  display->setCursor(10,20);
  display->fillScreen(GxEPD_WHITE);
  display->print(label);
  display->setCursor(10,50);
  display->print(data1);
  display->setCursor(10,80);
  display->print(data2);
  display->update();
}

void enableBacklight(bool en)
{
    digitalWrite(ePaper_Backlight, en);
}

uint32_t getMacAddress()
{
  // read the mac address

  uint32_t id = NRF_FICR->DEVICEADDR[1] & 0xffff;
  id = id | (NRF_FICR->DEVICEADDR[0] << 4);

  return id;
}


void loop() {
    loopGPS();
    loopLoRaWAN();    
}



void loopGPS()
{
    static bool firstFix = true;
    //Serial.println("in loopGPS");

    while (SerialGPS.available() > 0)
    {
        gps->encode(SerialGPS.read());
    }

    if (gps->location.isUpdated())
    {
      //Serial.println("location updated");
      if (millis() - lastGpsFixMillis > fixSendInterval) 
      {
        //  Serial.println("got a GPS fix");
        //  lastGpsFixMillis = millis();

        if( ! firstFix ) // always ignore first fix
        {
          Serial.println("got a GPS fix");
          
          lat = gps->location.lat();
          lng = gps->location.lng();
          alt = gps->altitude.meters();
          printText("got a GPS fix",String(lat),String(lng));
        }else{
          Serial.println("first fix");
          printText("got a first fix","ignoring","");
          firstFix = false;
        }
          
        lastGpsFixMillis = millis();

      }               
      if (gps->charsProcessed() < 10) {
            Serial.println(F("WARNING: No GPS data.  Check wiring."));
            printText("WARNING: No GPS data","","");
      }
    }
}


void loopLoRaWAN(){

 if (millis() - lastLoRaWANMillis > (uplinkIntervalSeconds * 1000UL)){

    Serial.println(F("Sending uplink"));
    printText("Sending","uplink","");

    lpp.reset();

    lpp.addGPS(1, lat, lng, alt);

    // Perform an uplink
    int16_t state =  node.sendReceive(lpp.getBuffer(), lpp.getSize());   
    debug(state < RADIOLIB_ERR_NONE, F("Error in sendReceive"), state, false);
    if(state < RADIOLIB_ERR_NONE){
      printText("Error in sendReceive","state",String(state));
    }

    // Check if a downlink was received 
    // (state 0 = no downlink, state 1/2 = downlink in window Rx1/Rx2)
    if(state > 0) {
      Serial.println(F("Received a downlink"));
      printText("Received downlink","State:",String(state));
    } else {
      Serial.println(F("No downlink received"));
      printText("No downlink.","State:",String(state));
    }


    Serial.print(F("Next uplink in "));
    Serial.print(uplinkIntervalSeconds);
    Serial.println(F(" seconds\n"));
    printText("Next uplink:",String(uplinkIntervalSeconds),"seconds");

    lastLoRaWANMillis = millis(); //delay blocks GPS

 }
}
