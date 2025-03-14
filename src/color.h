#ifndef COLOR_H
#define COLOR_H
#include <Arduino.h>

// https://rgbcolorpicker.com/565
#define TFT_BLACK 0x0000
#define TFT_WHITE 0xFFFF
#define TFT_RED 0xF800
#define TFT_DARK_RED 0xc021
#define TFT_DARK_GREEN 0x0560
#define TFT_LIGHT_GREEN 0x5fa0
#define TFT_YELLOW 0xf7a0
#define TFT_YELLOW_ORANGE 0xee49
#define TFT_DARK_ORANGE 0xe3e2
#define TFT_LIGHT_BLUE 0x465f
#define TFT_CYAN 0x47db
#define TFT_DARK_BLUE 0x213e

uint16_t co2TextColor(uint16_t co2Concentration);
uint16_t humidityTextColor(float humidity);
uint16_t temperatureTextColor(float temperatur);


#endif //COLOR_H
