#ifndef GRAPH_H
#define GRAPH_H

#include <Arduino.h>
#include <Adafruit_ST7735.h>

class Graph {
    private:
        uint16_t width;
        uint16_t height;
        uint16_t paddingLeft;
        uint16_t yTop;
        int16_t scaleMin;
        int16_t scaleMax;
        Adafruit_ST7735 &tft;
        uint16_t values[500];

    public:
        Graph(Adafruit_ST7735 &tft, uint16_t widthPx,uint16_t heightPx, uint16_t posX,uint16_t posY, int16_t scaleMin, int16_t scaleMax);

        void addValue(uint16_t newValue);

        void draw();
};



#endif //GRAPH_H
