#include "NetworkLayer.h"

#include <WiFi.h>
#include "esp_wifi.h"
#include <PubSubClient.h>
#include <Adafruit_ST7735.h>
#include <ArduinoJson.h>

NetworkLayer::NetworkLayer(Adafruit_ST7735 &tft)
     : tft(tft), err(ESP_OK), espClient(), mqttClient(espClient) {
    this->tft = tft;
}

void NetworkLayer::init(const char* ssid, const char *passphrase, const char * client_id, const char *mqttServer, int mqttPort, const char *mqtt_username, const char *mqtt_password) {
    this->clientId = client_id;
    this->mqttUsername = mqtt_username;
    this->mqttPassword = mqtt_password;

    // Bring up the WiFi connection
    WiFi.setHostname(clientId);

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
    mqttClient.setBufferSize(1024); // Config that is usaly ~ 850 bytes +header must fit into buffer

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

boolean NetworkLayer::loop() {
    boolean reconnected = false;
    // Restart every hour to prevent crazy esp problems
    if (((millis() - startTime) / 1000 / 60 / 60) > 12) {
        Serial.println("Rebooting");

        mqttClient.publish(
          ("esp32/" + String(clientId) + "/restart").c_str(),
          "half-day",
          false);

        ESP.restart();
        reconnected = true;
    }

    if (!mqttClient.connected()) {
        Serial.println("Reconnecting to MQTT");
        reconnect();
        reconnected = true;
    }

    mqttClient.loop();

    if (reconnected) {
      sendMqttConfig();
      mqttClient.loop();

      sendOnline();
      mqttClient.loop();
    }

    return reconnected;
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
        Serial.print(clientId);
        Serial.print(" -> ");
        Serial.println(mqttUsername);

        if (mqttClient.connect(
          clientId,
          mqttUsername,
          mqttPassword,
          ("homeassistant/sensor/" + String(clientId) + "/availability").c_str(),
          MQTTQOS1,
          true,
          "offline"
        )) {
            Serial.println("Connected");
        } else {
            Serial.print("failed, rc= ");
            Serial.println(mqttClient.state());
            delay(1000);
        }
    }
}

void NetworkLayer::sendViaMqtt(float temperature, float relativeHumidity, uint16_t co2Concentration) {
    JsonDocument doc;
    doc.clear();
    doc["temperature"] = (int)(temperature * 100 + 0.5) / 100.0;
    doc["humidity"] = (int)(relativeHumidity * 100 + 0.5) / 100.0;
    doc["co2"] = co2Concentration;

    // printf(payload, "{\"temperature\": %.2f, \"humidity\": %.2f, \"co2\": %d}", temperature, relativeHumidity, co2Concentration);
    char buffer[120];
    serializeJson(doc, buffer);

    boolean published = mqttClient.publish(
      ("homeassistant/sensor/" + String(clientId) + "/state").c_str(),
      buffer,
      false);

    if (!published) {
      Serial.println("sendViaMqtt: send failed");
    }
}

void NetworkLayer::sendMqttConfig() {
  // https://www.home-assistant.io/integrations/mqtt/#supported-abbreviations-in-mqtt-discovery-messages
  String macAddress = WiFi.macAddress();
  JsonDocument doc;
  boolean published;

  doc.clear();
  doc["name"] = String(clientId) + " Temperature";
  doc["uniq_id"] = macAddress + "_temp";
  doc["stat_t"] = "homeassistant/sensor/" + String(clientId) + "/state";
  doc["avty_t"] = "homeassistant/sensor/" + String(clientId) + "/availability";
  doc["val_tpl"] = "{{ value_json.temperature}}";
  doc["dev_cla"] = "temperature";
  doc["unit_of_meas"] = "°C";
  JsonObject deviceTemp = doc["device"].to<JsonObject>();
  deviceTemp["name"] = String(clientId);
  deviceTemp["ids"] = macAddress;
  deviceTemp["mf"] = "Home made";
  deviceTemp["mdl"] = "ESP32";

  char bufferTemp[512];
  serializeJson(doc, bufferTemp);
  published = mqttClient.publish(
    ("homeassistant/sensor/" + String(clientId) + "_temp/config").c_str(),
    bufferTemp,
    false
  );
  if (!published) {
    Serial.println("sendViaMqtt: send temperatue failed");
  }


  doc.clear();
  doc["name"] = String(clientId) + " Humidity";
  doc["uniq_id"] = macAddress + "_humi";
  doc["stat_t"] = "homeassistant/sensor/" + String(clientId) + "/state";
  doc["avty_t"] = "homeassistant/sensor/" + String(clientId) + "/availability";
  doc["val_tpl"] = "{{ value_json.humidity}}";
  doc["dev_cla"] = "humidity";
  doc["unit_of_meas"] = "%";
  JsonObject deviceHumi = doc["device"].to<JsonObject>();
  deviceHumi["name"] = String(clientId);
  deviceHumi["ids"] = macAddress;

  char bufferHumi[512];
  serializeJson(doc, bufferHumi);
  mqttClient.publish(
    ("homeassistant/sensor/" + String(clientId) + "_humi/config").c_str(),
    bufferHumi,
    false
  );
  if (!published) {
    Serial.println("sendViaMqtt: send humidity failed");
  }



  doc.clear();
  doc["name"] = String(clientId) + " CO2";
  doc["uniq_id"] = macAddress + "_co2";
  doc["stat_t"] = "homeassistant/sensor/" + String(clientId) + "/state";
  doc["avty_t"] = "homeassistant/sensor/" + String(clientId) + "/availability";
  doc["val_tpl"] = "{{ value_json.co2}}";
  doc["dev_cla"] = "carbon_dioxide";
  doc["unit_of_meas"] = "ppm";
  JsonObject deviceCo2 = doc["device"].to<JsonObject>();
  deviceCo2["name"] = String(clientId);
  deviceCo2["ids"] = macAddress;

  char bufferCo2[512];
  serializeJson(doc, bufferCo2);
  mqttClient.publish(
    ("homeassistant/sensor/" + String(clientId) + "__co2/config").c_str(),
    bufferCo2,
    false
  );
  if (!published) {
    Serial.println("sendViaMqtt: send co2 failed");
  }
}

void NetworkLayer::sendOnline() {
    Serial.println("homeassistant/sensor/" + String(clientId) + "/availability");
    Serial.println("online");

    boolean published = mqttClient.publish(
      ("homeassistant/sensor/" + String(clientId) + "/availability").c_str(),
      "online",
      true);

    if (!published) {
      Serial.println("sendOnline: send failed");
    }
}