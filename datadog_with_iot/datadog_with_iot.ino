#include <Wire.h>
#include <SSCI_BME280.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <Time.h>
#include <TimeLib.h>
#include <NTP.h>
#include "setting.h" 

WiFiClientSecure client;
SSCI_BME280 bme280;
uint8_t i2c_addr = 0x76;

void setup() {
  Serial.begin(115200);
  delay(100);

  setupBME280();
  setupWifi();
  setupNTP();
}

void setupBME280() {
  uint8_t osrs_t = 1;             //Temperature oversampling x 1
  uint8_t osrs_p = 1;             //Pressure oversampling x 1
  uint8_t osrs_h = 1;             //Humidity oversampling x 1
  uint8_t bme280mode = 3;         //Normal mode
  uint8_t t_sb = 5;               //Tstandby 1000ms
  uint8_t filter = 0;             //Filter off
  uint8_t spi3w_en = 0;           //3-wire SPI Disable

  Wire.begin();
  bme280.setMode(i2c_addr, osrs_t, osrs_p, osrs_h, bme280mode, t_sb, filter, spi3w_en);
  bme280.readTrim();
}

void setupWifi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
   
  WiFi.begin(wifi_ssid, wifi_password);
   
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
    
  // Print the IP address
  Serial.println(WiFi.localIP());  
}

void setupNTP() {
  ntp_begin(2390); 
  while (timeStatus() == timeNotSet) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("NTP time set");    
}

void loop() {
  time_t tm = now();
  double temp_act, press_act, hum_act;
  bme280.readData(&temp_act, &press_act, &hum_act);
  
  postMetric("bme280", tm, temp_act, press_act, hum_act);
  
  delay(15000);
}

void postMetric(String tags, time_t tm, double temp_act, double press_act, double hum_act) {
  if (client.connect("app.datadoghq.com", 443)) {
    char json[1024];
    sprintf(json, "{\"series\":[{\"metric\":\"indoorenv.temperature\",\"points\":[[%lu, %s]],\"type\":\"gauge\",\"tags\":[\"%s\"]},{\"metric\":\"indoorenv.pressure\",\"points\":[[%lu, %s]],\"type\":\"gauge\",\"tags\":[\"%s\"]},{\"metric\":\"indoorenv.humidity\",\"points\":[[%lu, %s]],\"type\":\"gauge\",\"tags\":[\"%s\"]}]}", 
      tm, String(temp_act).c_str(), tags.c_str(),
      tm, String(press_act).c_str(), tags.c_str(),
      tm, String(hum_act).c_str(), tags.c_str());

    Serial.println(json);
    String requestPath = "/api/v1/series?api_key=";
    requestPath += datadog_api_key;
    client.println("POST " + requestPath + " HTTP/1.1");
    client.println("Host: app.datadoghq.com");
    client.println("User-Agent: ESP8266/1.0");
    client.println("Connection: close");
    client.println("Content-Type: application/json;");
    client.print("Content-Length: ");
    client.println(strlen(json));
    client.println();
    client.print(json);
    client.flush();
    String response = client.readString();
    Serial.println(response);
  }
}
