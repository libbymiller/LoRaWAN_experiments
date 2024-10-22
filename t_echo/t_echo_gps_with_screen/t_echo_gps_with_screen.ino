/*

  Just GPS
  much stolen from https://github.com/jarkman/WhereOTechno/blob/main/WhereOTechno.ino

*/


#include "config.h"

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
uint32_t fixSendInterval = 30000;

void configVDD(void);
void boardInit();
void setupDisplay();
bool setupGPS();
void loopGPS();

void setup() {
  Serial.begin(115200);
  delay(4000);
  Serial.println("starting up");
  boardInit();
  delay(4000);
  printText("Ready");
}

void boardInit()
{
    Serial.println("board init");

    uint8_t rlst = 0;

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

    Serial.print("2\n");
    setupDisplay();

    Serial.print("3\n");
    setupGPS();

    Serial.print("4\n");
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


void loop() {
    // put your main code here, to run repeatedly:
    loopGPS();
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

