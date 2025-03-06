#include "graph.h"
#include "color.h"
#include <Arduino.h>
#include <Adafruit_ST7735.h>

Graph::Graph(Adafruit_ST7735 &tft, uint16_t w,uint16_t h, uint16_t x ,uint16_t y, int16_t scaleMin, int16_t scaleMax)  : tft(tft)
{
    this->tft = tft;
    this->width = w;
    this->height = h;
    this->paddingLeft = x;
    this->yTop = y;
    this->scaleMin = scaleMin;
    this->scaleMax = scaleMax;

    // Initialise arrays
    for (int i = 0; i < w; i++) {
        values[i] = 0;
    }
}

void Graph::addValue(uint16_t newValue) {
    static int i = 0;
    if (isnan((int)newValue)) {
        if (i < width) {
            values[i] = 0;
            i++;
        } else {
            for (int j = 0; j < width - 1; j++) {
                values[j] = values[j + 1];
                values[width - 1] = 0;
            }
        }
    } else {
        if (i < width) {
            values[i] = newValue;
            i++;
        } else {
            for (int j = 0; j < width - 1; j++) {
                values[j] = values[j + 1];
                values[width - 1] = newValue;
            }
        }
    }
}

void Graph::draw() {
    int16_t yBottom = yTop + height;
    int16_t heigth = yBottom - yTop;
    int16_t padding = 2;
    int16_t valuesPerPixel = (scaleMax - scaleMin) / heigth;
    uint16_t X;
    uint16_t Y;
    uint16_t PriorX = 0;
    uint16_t PriorY = 0;
    uint16_t PriorValue = 0;

    // avoid strange rainbow line at total right
    tft.fillRect(width + padding - 1 , yTop - 1, padding * 3, heigth + 2, TFT_BLACK);

    for (int i = 0; i < width; i++) {
        uint16_t value = values[i];

        if (value == 0) {  // Not jet stored value
            continue;
        }

        if (value < scaleMin) {
            value = scaleMin;
        } else if (value > scaleMax) {
            value = scaleMax;
        }

        if (value == PriorValue && i < (width - 1)) {
            // prefer to draw straigth lines in one
            continue;
        }

        X = i + padding;
        Y = yBottom - (((value + 1) - scaleMin) / valuesPerPixel);

//        Serial.print("value:");
//        Serial.print(value);
//        Serial.print(",X:");
//        Serial.print(X);
//        Serial.print(",Y:");
//        Serial.println(Y);

        if (PriorX > 0) {
            // wipe area
            int16_t lineWidth = X - PriorX;
            tft.fillRect(PriorX, yTop - 1, lineWidth, heigth + padding, TFT_BLACK);


            // draw line
            tft.drawLine(PriorX, PriorY, X, Y, co2TextColor(values[i]));

//            Serial.print("x1=");
//            Serial.print(PriorX);
//            Serial.print(",y1=");
//            Serial.print(PriorY);
//            Serial.print(",x2=");
//            Serial.print(X);
//            Serial.print(",y2=");
//            Serial.println(Y);
        }

        PriorX = X;  // Remember this coordinate so connecting line is draw between the PRIOR point and the NEXT point
        PriorY = Y;
        PriorValue = value;
    }
}