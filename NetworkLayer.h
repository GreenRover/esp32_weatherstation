#ifndef NETWORKLAYER_H
#define NETWORKLAYER_H

#include <WiFi.h>
#include "esp_wifi.h"
#include <PubSubClient.h>
#include <Adafruit_ST7735.h>

class NetworkLayer {
  private:
    Adafruit_ST7735 &tft;

    unsigned long startTime;
    unsigned long previousMillis;
    unsigned long mqttSendInterval = 10 * 1000;  // 10 sec

    String clientId;
    const char *mqttUsername;
    const char *mqttPassword;

    WiFiClient espClient;
    PubSubClient mqttClient;
    esp_err_t err;

    void reconnect();

  public:
  NetworkLayer(Adafruit_ST7735 &tft);
  void init(const char* ssid, const char *passphrase, String clientId, const char *mqttServer, int mqttPort, const char *mqttUsername, const char *mqttPassword);
  void ssidScan();
  void loop();
  void sendTempViaMqtt(float temperature, float relativeHumidity, uint16_t co2Concentration);
};



#endif //NETWORKLAYER_H
