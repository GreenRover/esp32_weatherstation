#ifndef LIFEINFO_H
#define LIFEINFO_H

#include <Adafruit_ST7735.h>


class LifeInfo {
    private:
      Adafruit_ST7735 &tft;
      bool lastWasError = true;

    public:
      LifeInfo(Adafruit_ST7735 &tft);
      void draw(float temperature, float relativeHumidity, uint16_t co2Concentration);
      void drawError(uint16_t error);
};



#endif //LIFEINFO_H
