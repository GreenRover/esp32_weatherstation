#include <Adafruit_GFX.h>     // Core graphics library
#include <Adafruit_ST7735.h>  // Hardware-specific library for ST7735
#include <SPI.h>
#include "color.h"
#include "Graph.h"
#include "NetworkLayer.h"
#include "LifeInfo.h"
#include "secrets.h"

#define TFT_CS  5
#define TFT_RST 16
#define TFT_DC  17
// #define TFT_MOSI  23  // SDA // HW MOSI
// #define TFT_SCLK  18  // SCL // HW SCLK
// #define TFT_MISO  19  // not used
// #define TFT_BL  25  // LED back-light

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

#include <Arduino.h>
#include <SensirionI2cScd4x.h>
#include <Wire.h>

#ifdef NO_ERROR
#undef NO_ERROR
#endif
#define NO_ERROR 0

SensirionI2cScd4x sensor;

// ############### Programm logic start

uint16_t co2Concentration = 0;
float temperature = 0.0;
float relativeHumidity = 0.0;

uint16_t error;
char errorMessage[256];

Graph co2Graph = Graph(tft, 160 - 4, 128 - 88, 2, 88, 600, 1600);
NetworkLayer networkLayer = NetworkLayer(tft);
LifeInfo lifeInfo = LifeInfo(tft);

// -------------------------------------------------------------------------
// Setup
// -------------------------------------------------------------------------
void setup(void) {
  Serial.begin(115200);
  while (!Serial) {
    delay(100);
  }

  // Hack to create multiple permant 3v sources
  // turnInto3v(25); may result in black screen, prefer connect to 3v if possible


  Wire.begin();
  sensor.begin(Wire, SCD41_I2C_ADDR_62);
  delay(30);

  error = sensor.wakeUp();
  if (error != NO_ERROR) {
    Serial.print("Error trying to execute wakeUp(): ");
    errorToString(error, errorMessage, sizeof errorMessage);
    Serial.println(errorMessage);
  }

  sensor.startPeriodicMeasurement();



  tft.initR(INITR_18BLACKTAB);
  // tft.setSPISpeed(40000000);

  tft.fillScreen(TFT_BLACK);
  tft.setRotation(1);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextWrap(false);
  tft.println("Booting ...");


  // Bring up the WiFi connection
  networkLayer.init(wlan_ssid, wlan_password, device_name, mqtt_server, mqtt_port, mqtt_username, mqtt_password);
  Serial.print("Setup finished");
  delay(4000);
}

void turnInto3v(uint8_t pin) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, 1);
}

// -------------------------------------------------------------------------
// Main loop
// -------------------------------------------------------------------------
unsigned long currentMillis;

unsigned long lastSendMqttConfigMillis;
const unsigned long sendMqttConfigPeriod = (60 * 1000) + 1;

unsigned long lastReadValueMillis;
const unsigned long readValuePeriod = 5 * 1000;
const unsigned int mqttSendValueMultiplier = 2;
unsigned int readValueDidtNotSendMqtt = 0;

const unsigned long drawDelay = 500;
unsigned long drawAfterMillis;

void loop() {
  networkLayer.loop();

  currentMillis = millis();

  if (currentMillis - lastSendMqttConfigMillis >= sendMqttConfigPeriod) {
    networkLayer.sendMqttConfig();
    lastSendMqttConfigMillis = currentMillis;
  }

  if (currentMillis - lastReadValueMillis >= readValuePeriod) {
    error = sensor.readMeasurement(co2Concentration, temperature, relativeHumidity);
    if (error != NO_ERROR) {
      lifeInfo.drawError(error);
    } else {
  
      lifeInfo.draw(temperature, relativeHumidity, co2Concentration);
  
      co2Graph.addValue(co2Concentration);
  
      readValueDidtNotSendMqtt++;
      if (readValueDidtNotSendMqtt >= mqttSendValueMultiplier) {
        networkLayer.sendViaMqtt(temperature, relativeHumidity, co2Concentration);
        readValueDidtNotSendMqtt = 0;
      }
    }

    lastReadValueMillis = currentMillis;
    drawAfterMillis = currentMillis + drawDelay;
  }

  if (drawAfterMillis > 0 && drawAfterMillis < currentMillis) {
    // Drawing interferes with network sending.
    co2Graph.draw();
    drawAfterMillis = 0;
  }
}
