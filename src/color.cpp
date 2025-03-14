#include "color.h"
#include <Arduino.h>

uint16_t co2TextColor(uint16_t co2Concentration) {
    // https://www.cik-solutions.com/branchen-anwendungen/co2-im-innenraum/
    if (co2Concentration < 600) {
        return TFT_DARK_GREEN;
    } else if (co2Concentration < 800) {
        return TFT_LIGHT_GREEN;
    } else if (co2Concentration < 1000) {
        return TFT_YELLOW;
    } else if (co2Concentration < 1200) {
        return TFT_YELLOW_ORANGE;
    } else if (co2Concentration < 1400) {
        return TFT_DARK_ORANGE;
    } else {
        return TFT_DARK_RED;
    }
}

uint16_t humidityTextColor(float humidity) {
    // https://www.getair.eu/wissen/optimale-luftfeuchtigkeit-so-schuetzen-sie-gesundheit-und-gebaeude/
    if (humidity < 30) {
        return TFT_DARK_ORANGE;
    } else if (humidity < 40) {
        return TFT_YELLOW;
    } else if (humidity > 70) {
        return TFT_DARK_BLUE;
    } else if (humidity > 60) {
        return TFT_LIGHT_BLUE;
    } else {
        return TFT_DARK_GREEN;
    }
}

uint16_t temperatureTextColor(float temperatur) {
    // https://www.getair.eu/wissen/optimale-luftfeuchtigkeit-so-schuetzen-sie-gesundheit-und-gebaeude/
    if (temperatur < 20) {
        return TFT_DARK_BLUE;
    } else if (temperatur < 23) {
        return TFT_CYAN;
    } else if (temperatur > 28) {
        return TFT_DARK_RED;
    } else if (temperatur > 26) {
        return TFT_DARK_ORANGE;
    } else {
        return TFT_DARK_GREEN;
    }
}