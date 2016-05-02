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
const int sensorId = 1;
const String url = "URL" + String(sensorId);




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
  HTTPClient http;
  http.setUserAgent("DS18B20");
  Serial.println("URL" + url);
  http.begin(url);
  float voltage = (float(ESP.getVcc()) + float(readvdd33())) / 2048.0;
  Serial.println("Temperature: " + String(temperature));
  Serial.println("Voltage: " + String(voltage));
  String payload = "{ \"value\" : " + String(temperature) +  " , \"voltage\" : { \"value\": " + String(voltage) +  "  } }";
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(payload);
  if(httpCode > 0) {
    Serial.printf("[HTTP] POST... code: %d\n", httpCode);
    if(httpCode == HTTP_CODE_OK) {
        payload = http.getString();
        Serial.println(payload);
    }
  } else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
}


void loop() {
  initWifi();
  readAndSend();
  sleep();
}
