#include "NetworkLayer.h"

#include <WiFi.h>
#include "esp_wifi.h"
#include <PubSubClient.h>
#include <Adafruit_ST7735.h>

NetworkLayer::NetworkLayer(Adafruit_ST7735 &tft)
     : tft(tft), previousMillis(0), err(ESP_OK), espClient(), mqttClient(espClient) {
    this->tft = tft;
}

void NetworkLayer::init(const char* ssid, const char *passphrase, String client_id, const char *mqttServer, int mqttPort, const char *mqtt_username, const char *mqtt_password) {
    this->clientId = client_id;
    this->mqttUsername = mqtt_username;
    this->mqttPassword = mqtt_password;

    // Bring up the WiFi connection
    WiFi.setHostname(clientId.c_str());

    // wifiSsidScan();
    // Give user time to read
    // delay(1500);

    tft.print("Wifi connect: ");

    WiFi.begin(ssid, passphrase);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        tft.print(".");
    }
    tft.println("+");

    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    tft.println("IP address: " + WiFi.localIP().toString());

    //---------As we are connected to WiFi, begin MQTT connection
    mqttClient.setServer(mqttServer, mqttPort);

    startTime = millis();
}

void NetworkLayer::ssidScan() {
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n == 0) {
        tft.print("no networks found");
        Serial.println("no networks found");
    } else {
        Serial.print(n);
        Serial.println(" networks found");
        tft.println("networks found");
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            String logLine = String(i + 1) + ": " + WiFi.SSID(i) + " -- " + WiFi.BSSIDstr(i) + " (" + WiFi.RSSI(i) + ") " + ((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
            Serial.println(logLine);

            String tftLine = WiFi.SSID(i) + " @ " + WiFi.BSSIDstr(i) + " " + WiFi.RSSI(i);
            tft.println(tftLine);
        }
    }
}

void NetworkLayer::loop() {
    // Restart every hour to prevent crazy esp problems
    if (((millis() - startTime) / 1000 / 60 / 60) > 12) {
        Serial.println("Rebooting");

        mqttClient.publish(
          ("esp32/" + clientId + "/restart").c_str(),
          "half-day",
          false);

        ESP.restart();
    }

    if (!mqttClient.connected()) {
        Serial.println("Reconnecting to MQTT");
        reconnect();
    }

    mqttClient.loop();
}

void NetworkLayer::reconnect() {
    int wifiStatus = WiFi.status();
    if (wifiStatus != WL_CONNECTED) {
        Serial.print("Wifi not connected, rebootinng");
        ESP.restart();
    }

    Serial.print("MQTT ");
    while (!mqttClient.connected()) {
        Serial.print("connecting as ");
        Serial.print(clientId.c_str());
        Serial.print(" -> ");
        Serial.println(mqttUsername);

        if (mqttClient.connect(clientId.c_str(), mqttUsername, mqttPassword)) {
            Serial.println("Connected");
        } else {
            Serial.print("failed, rc= ");
            Serial.println(mqttClient.state());
            delay(1000);
        }
    }
}

void NetworkLayer::sendTempViaMqtt(float temperature, float relativeHumidity, uint16_t co2Concentration) {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis < mqttSendInterval) {
        // Too soon, don't send.
        return;
    }

    previousMillis = currentMillis;


    char tempString[8];
    dtostrf(temperature, 1, 2, tempString);

    mqttClient.publish(
      ("esp32/" + clientId + "/temperature").c_str(),
      tempString,
      false);


    char humidityString[8];
    dtostrf(relativeHumidity, 1, 2, humidityString);

    mqttClient.publish(
      ("esp32/" + clientId + "/relativeHumidity").c_str(),
      humidityString,
      false);


    char co2String[8];
    dtostrf(co2Concentration, 1, 2, co2String);

    mqttClient.publish(
      ("esp32/" + clientId + "/co2Concentration").c_str(),
      co2String,
      false);
}
