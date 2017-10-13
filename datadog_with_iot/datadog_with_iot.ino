#include <ESP8266WiFi.h>
#include <Time.h>
#include <TimeLib.h>
#include <NTP.h>
#include <WiFiClientSecure.h>
#include "setting.h" 

WiFiClientSecure client;
 
void setup() {
  Serial.begin(115200);
  delay(100);
  
  // Connect to WiFi network
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

  // NTP
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
  postMetric("environment:test", "test.metric", tm, 123);
  delay(5000);
}

void postMetric(String tags, String metric, time_t tm, double point) {
  if (client.connect("app.datadoghq.com", 443)) {
    char json[256];
    sprintf(json, "{\"series\":[{\"metric\":\"%s\",\"points\":[[%lu, %s]],\"type\":\"gauge\",\"host\":\"%s\",\"tags\":[\"%s\"]}]}", metric.c_str(), tm, String(point).c_str(), datadog_host, tags.c_str());

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
    client.println(json);
    delay(10);
    client.read
    String response = client.readString();  // レスポンス取れない・・・
    Serial.println(response);
  }
}
