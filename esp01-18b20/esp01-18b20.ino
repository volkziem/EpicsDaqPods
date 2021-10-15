// ESP01-18b20 for EPICS server, V. Ziemann, 200217
const char* ssid     = "DAQnet";
const char* password = "........";
const int port=1137;
#include <ESP8266WiFi.h>
WiFiServer server(port);
#include <OneWire.h>
#include <DallasTemperature.h>
OneWire oneWire(2);  // GPIO2
DallasTemperature sensors(&oneWire);
//..................................................................setup
void setup() {
  sensors.begin();
  //...................................static IP
  IPAddress ip(192, 168, 20, 54);
  IPAddress gw(192, 168, 20, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.config(ip,gw,subnet);
  //............................................
  WiFi.begin(ssid,password);
  while (WiFi.status() != WL_CONNECTED) {delay(500);}
  server.begin();
}
//.................................................................loop
void loop() { 
  char line[30];
  WiFiClient client = server.available();
  while (client) {
    while(!client.available()) {
      delay(5);
      if (!client.connected()) {client.stop(); break;}
    }
    if (!client.connected()) {client.stop(); break;}
    client.readStringUntil('\n').toCharArray(line,30);
    if (strstr(line,"V?")==line) {
      client.println("ESP01-18b20 V1");
    } else if (strstr(line,"T?")==line) {
      sensors.requestTemperatures();
      float temp=sensors.getTempCByIndex(0);
      client.print("T "); client.println(temp);
    }
    yield();
  } 
}
