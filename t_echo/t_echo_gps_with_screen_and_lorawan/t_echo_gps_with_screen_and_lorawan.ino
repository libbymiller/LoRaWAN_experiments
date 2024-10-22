/*

  much stolen from https://github.com/jarkman/WhereOTechno/blob/main/WhereOTechno.ino
  and the rest from https://github.com/jgromes/RadioLib/tree/master/examples/LoRaWAN

  had to downgrade radiolib to 6.6.0

*/


#include "config.h"
#include <CayenneLPP.h>

//242 is from 
//https://github.com/HelTecAutomation/CubeCell-Arduino/blob/master/libraries/LoraWan102/src/loramac/Commissioning.h
//in turn from here https://www.thethingsnetwork.org/forum/t/example-ttn-code-for-heltec-htcc-ab02a-with-gps-in-us915/46093

CayenneLPP lpp(242);

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

//loraWAN startup stuff

  Serial.println(F("Initialise the radio"));
  int16_t state = radio.begin();
  debug(state != RADIOLIB_ERR_NONE, F("Initialise radio failed"), state, true);

  // Setup the OTAA session information
  node.beginOTAA(joinEUI, devEUI, nwkKey, appKey);

  Serial.println(F("Join ('login') the LoRaWAN Network"));
  state = node.activateOTAA();

  debug(state != RADIOLIB_LORAWAN_NEW_SESSION, F("Join failed"), state, true);

  if(state != RADIOLIB_LORAWAN_NEW_SESSION){
    Serial.println(F("Waiting 5 secs and trying again\n"));
    delay(5000); 
    state = radio.begin();
    Serial.println(F("Join ('login') the LoRaWAN Network"));
    node.beginOTAA(joinEUI, devEUI, nwkKey, appKey);
    state = node.activateOTAA();
    debug(state != RADIOLIB_LORAWAN_NEW_SESSION, F("2nd join failed, giving up"), state, true);
  }

  Serial.println(F("Ready\n"));
  printText("Ready");
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

void printText(const char*label){
  display->setCursor(10,20);
  display->fillScreen(GxEPD_WHITE);
  display->print(label);
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
          printText("got a GPS fix");
          lat = gps->location.lat();
          lng = gps->location.lng();
          alt = gps->altitude.meters();
        }else{
          Serial.println("first fix");
          printText("got a first fix");
          firstFix = false;
        }
          
        lastGpsFixMillis = millis();

      }               
      if (gps->charsProcessed() < 10) {
            Serial.println(F("WARNING: No GPS data.  Check wiring."));
      }
    }
}


void loopLoRaWAN(){

 if (millis() - lastLoRaWANMillis > (uplinkIntervalSeconds * 1000UL)){

  Serial.println(F("Sending uplink"));

  lpp.reset();
  //int batt_lvl = random(0,3.3);
  //Serial.print(F(",batt_lvl="));
  //Serial.print(batt_lvl, 2);
  //lpp.addAnalogInput(8, batt_lvl);

  lpp.addGPS(1, lat, lng, alt);
  Serial.print("my_id is ");  
  Serial.println(my_id);

  // Perform an uplink
  int state = node.sendReceive(lpp.getBuffer(), lpp.getSize());   
 
  debug((state != RADIOLIB_LORAWAN_NO_DOWNLINK) && (state != RADIOLIB_ERR_NONE), F("Error in sendReceive"), state, false);

  Serial.print(F("Uplink complete, next in "));
  Serial.print(uplinkIntervalSeconds);
  Serial.println(F(" seconds"));
  lastLoRaWANMillis = millis();
  //use millis for this or it blocks GPS
  // Wait until next uplink - observing legal & TTN FUP constraints
  //delay(uplinkIntervalSeconds * 1000UL);  // delay needs milli-seconds
 }
}
