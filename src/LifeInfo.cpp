#include "LifeInfo.h"
#include "color.h"
#include <Arduino.h>
#include <SensirionI2cScd4x.h>
#include <Adafruit_ST7735.h>

LifeInfo::LifeInfo(Adafruit_ST7735 &tft)
     : tft(tft) {
    this->tft = tft;
}

void LifeInfo::drawError(uint16_t error) {
    char errorMessage[256];

    Serial.print("Error trying to execute readMeasurementData(): ");
    errorToString(error, errorMessage, sizeof errorMessage);
    Serial.println(errorMessage);

    tft.fillScreen(TFT_RED);
    tft.setCursor(3, 3);
    tft.setTextColor(TFT_WHITE, TFT_RED);
    tft.setTextSize(1);

    tft.print("Error: ");
    tft.println(errorMessage);

    lastWasError = true;
}

void LifeInfo::draw(float temperature, float relativeHumidity, uint16_t co2Concentration) {
    tft.setTextSize(2);

    if (lastWasError) {
        // Save time, draw header only once.
        Serial.println("lastWasError=true");
        tft.fillScreen(TFT_BLACK);

        // Draw head lines:
        tft.setTextColor(TFT_WHITE, TFT_BLACK);

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
    tft.setTextColor(temperatureTextColor(temperature), TFT_BLACK);
    tft.print(temperature);
    tft.print(" *C  ");

    tft.setCursor(60, 32);
    tft.setTextColor(humidityTextColor(relativeHumidity), TFT_BLACK);
    tft.print(relativeHumidity);
    tft.print(" %  ");

    tft.setCursor(60, 62);
    tft.setTextColor(co2TextColor(co2Concentration), TFT_BLACK);
    tft.print(co2Concentration);
    tft.print(" ppm  ");
}