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

    const char *clientId;
    const char *mqttUsername;
    const char *mqttPassword;

    WiFiClient espClient;
    PubSubClient mqttClient;
    esp_err_t err;

    void reconnect();

  public:
  NetworkLayer(Adafruit_ST7735 &tft);
  void init(const char* ssid, const char *passphrase, const char *clientId, const char *mqttServer, int mqttPort, const char *mqttUsername, const char *mqttPassword);
  void ssidScan();
  boolean loop();
  void sendViaMqtt(float temperature, float relativeHumidity, uint16_t co2Concentration);
  void sendMqttConfig();
  void sendOnline();
};



#endif //NETWORKLAYER_H
