/*
LCD st7735
- gnd  braun
- vcc  red
- sck  orang    D27  serial clock 
- sda  gelb     D14  serial data
- res  grün     D3v  lcm reset
-  dc  blau     D12           data command
-  cs  violet   D13	 chip select
-  bl  weis     3v

SCD41 Sensor 
- gnd  schwarz
- vcc  weiss
- scl  grau   D22
- sda  violet D21
*/

#include <Adafruit_GFX.h>     // Core graphics library
#include <Adafruit_ST7735.h>  // Hardware-specific library for ST7735
#include <SPI.h>


#define TFT_DC 12    //A0
#define TFT_CS 13    //CS
#define TFT_MOSI 14  //SDA
#define TFT_CLK 27   //SCK
#define TFT_RST 0

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST);

#include <Arduino.h>
#include <SensirionI2cScd4x.h>
#include <Wire.h>

#ifdef NO_ERROR
#undef NO_ERROR
#endif
#define NO_ERROR 0

SensirionI2cScd4x sensor;

bool lastWasError = true;
uint16_t co2Concentration = 0;
float temperature = 0.0;
float relativeHumidity = 0.0;

uint16_t error;
char errorMessage[256];

const int DISPLAY_WIDTH = 160 - 4;
uint16_t co2Array[DISPLAY_WIDTH];

// -------------------------------------------------------------------------
// Setup
// -------------------------------------------------------------------------
void setup(void) {
  Serial.begin(115200);
  while (!Serial) {
    delay(100);
  }


  turnInto3v(26);
  turnInto3v(25);
  turnInto3v(33);


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
  tft.setSPISpeed(40000000);

  tft.fillScreen(ST77XX_BLACK);
  tft.setRotation(1);


  // Initialise arrays
  for (int i = 0; i < DISPLAY_WIDTH; i++) {
    co2Array[i] = 0;
  }
}

// -------------------------------------------------------------------------
// Main loop
// -------------------------------------------------------------------------
void loop() {
  error = sensor.readMeasurement(co2Concentration, temperature, relativeHumidity);
  if (error != NO_ERROR) {
    Serial.print("Error trying to execute readMeasurementData(): ");
    errorToString(error, errorMessage, sizeof errorMessage);
    Serial.println(errorMessage);

    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(3, 3);
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.setTextSize(1);

    tft.print("Error: ");
    tft.println(errorMessage);

    lastWasError = true;
  } else {

    tft.setTextSize(2);

    if (lastWasError) {
      tft.fillScreen(ST77XX_BLACK);

      // Draw head lines:
      tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);

      tft.setCursor(2, 2);
      tft.print("Temp: ");

      tft.setCursor(2, 32);
      tft.print("Humi: ");

      tft.setCursor(2, 62);
      tft.print("CO2: ");

      lastWasError = false;
    }

    Serial.print("Co2:");
    Serial.print(co2Concentration);
    Serial.print(",");
    Serial.print("Temperature:");
    Serial.print(temperature);
    Serial.print(",");
    Serial.print("Humi:");
    Serial.println(relativeHumidity);




    tft.setCursor(60, 2);
    tft.setTextColor(temperatureTextColor(temperature), ST77XX_BLACK);
    tft.print(temperature);
    tft.print(" *C");

    tft.setCursor(60, 32);
    tft.setTextColor(humidityTextColor(relativeHumidity), ST77XX_BLACK);
    tft.print(relativeHumidity);
    tft.print(" %");

    tft.setCursor(60, 62);
    tft.setTextColor(co2TextColor(co2Concentration), ST77XX_BLACK);
    tft.print(co2Concentration);
    tft.print(" ppm");

    storeCo2(co2Concentration);
    drawCo2Graph();
  }

  delay(5000);
}

void turnInto3v(uint8_t pin) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, 1);
}

uint16_t co2TextColor(uint16_t co2Concentration) {
  // https://rgbcolorpicker.com/565
  // https://www.cik-solutions.com/branchen-anwendungen/co2-im-innenraum/
  if (co2Concentration < 600) {
    return 0x0560;  // dark green
  } else if (co2Concentration < 800) {
    return 0x5fa0;  // ligth green
  } else if (co2Concentration < 1000) {
    return 0xf7a0;  // yellow
  } else if (co2Concentration < 1200) {
    return 0xee49;  // yellow to orange
  } else if (co2Concentration < 1400) {
    return 0xe3e2;  // dark orange
  } else {
    return 0xc021;  // dark red
  }
}

uint16_t humidityTextColor(float humidity) {
  // https://www.getair.eu/wissen/optimale-luftfeuchtigkeit-so-schuetzen-sie-gesundheit-und-gebaeude/
  if (humidity < 30) {
    return 0xe3e2;  // dark orange
  } else if (humidity < 40) {
    return 0xf7a0;  // yellow
  } else if (humidity > 70) {
    return 0x213e;  // dark blue
  } else if (humidity > 60) {
    return 0x465f;  // light blue
  } else {
    return 0x0560;  // dark green
  }
}

uint16_t temperatureTextColor(float temperatur) {
  // https://www.getair.eu/wissen/optimale-luftfeuchtigkeit-so-schuetzen-sie-gesundheit-und-gebaeude/
  if (temperatur < 20) {
    return 0x213e;  // dark blue
  } else if (temperatur < 23) {
    return 0x47db;  // light blue
  } else if (temperatur > 28) {
    return 0xc021;  // dark red
  } else if (temperatur > 26) {
    return 0xe3e2;  // dark orange
  } else {
    return 0x0560;  // dark green
  }
}

void storeCo2(uint16_t newValue) {
  static int i = 0;
  if (isnan((int)newValue)) {
    if (i < DISPLAY_WIDTH) {
      co2Array[i] = 0;
      i++;
    } else {
      for (int j = 0; j < DISPLAY_WIDTH - 1; j++) {
        co2Array[j] = co2Array[j + 1];
        co2Array[DISPLAY_WIDTH - 1] = 0;
      }
    }
  } else {
    if (i < DISPLAY_WIDTH) {
      co2Array[i] = newValue;
      i++;
    } else {
      for (int j = 0; j < DISPLAY_WIDTH - 1; j++) {
        co2Array[j] = co2Array[j + 1];
        co2Array[DISPLAY_WIDTH - 1] = newValue;
      }
    }
  }
}

void drawCo2Graph() {
  int16_t yMin = 600;
  int16_t yMax = 1600;

  int16_t yTop = 92;
  int16_t yBottom = 158;
  int16_t heigth = yBottom - yTop;
  int16_t width = DISPLAY_WIDTH;
  int16_t ppmPerPixel = 15;  // (1600 - 600) / (160 - 92)
  tft.fillRect(2, yTop, width, heigth, ST77XX_BLACK);


  uint16_t X;
  uint16_t Y;
  uint16_t PriorX = 0;
  uint16_t PriorY = 0;
  for (int i = 0; i < DISPLAY_WIDTH; i++) {
    uint16_t value = co2Array[i];

    if (value == 0) {  // Not jet stored value
      continue;
    }

    if (value < yMin) {
      value = yMin;
    } else if (value > yMax) {
      value = yMax;
    }

    X = i + 2;
    Y = yBottom - (((value + 1) - yMin) / ppmPerPixel);

    Serial.print("value:");
    Serial.print(value);
    Serial.print(",X:");
    Serial.print(X);
    Serial.print(",Y:");
    Serial.println(Y);



    if (PriorX > 0) {
      tft.drawLine(PriorX, PriorY, X, Y, ST77XX_MAGENTA);
    }

    PriorX = X;  // Remember this coordinate so connecting line is draw between the PRIOR point and the NEXT point
    PriorY = Y;
  }
}