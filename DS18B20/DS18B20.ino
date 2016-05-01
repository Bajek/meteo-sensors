#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 2  // DS18B20 pin
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
float temperature;

const char* ssid = "SSID";
const char* pass = "PASS";
const char* url = "URL";


extern "C" {
  #include "user_interface.h"
  //for input voltage readouts
  ADC_MODE(ADC_VCC);
  uint16 readvdd33(void);
}


void setup() {
  Serial.begin(115200);
  Serial.println();
  DS18B20.begin();
}

void sleep() {
  wifi_set_sleep_type(LIGHT_SLEEP_T);
  Serial.println("Sleeping for 30 sec");
  delay(30000);
}

void initWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while ((WiFi.status() != WL_CONNECTED)) {
     delay(500);
     Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected, IP address: "); Serial.println(WiFi.localIP());
}

void readAndSend() {
  DS18B20.requestTemperatures(); 
  delay(250);
  temperature = DS18B20.getTempCByIndex(0);
  char charBuf[15];
  dtostrf(temperature, 7, 3, charBuf);

  //For time being GET as it's easier.
  //Issue #1
  HTTPClient http;
  http.setUserAgent("DS18B20");
  http.begin(String(url) + "?temp=" + String(charBuf) + "&vcc=" + String(ESP.getVcc()) + "&vdd=" + String(readvdd33())); //HTTP
  int httpCode = http.GET();
  if(httpCode > 0) {
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    if(httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
    }
  } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
}


void loop() {
  initWifi();
  readAndSend();
  sleep();
}
