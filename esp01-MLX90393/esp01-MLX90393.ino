// ESP01-MLX90393 EPICS server, V. Ziemann, 200221
const char* ssid     = "DAQnet";
const char* password = "........";
const int port=1137;
#include <ESP8266WiFi.h>
WiFiServer server(port);
WiFiClient client;

#define SDA 3
#define SCL 1
#include <Wire.h>
#include <MLX90393.h>
//#include "MLX90393_local.h"
//#include "MLX90393_local.cpp"

MLX90393 mlx;
MLX90393::txyz mlxdata;
//................................................................setup
void setup() {
  WiFi.begin(ssid,password);
  while (WiFi.status() != WL_CONNECTED) {delay(500);}
  server.begin();
  Wire.begin(SDA,SCL);
  uint8_t status = mlx.begin(0, 1);   // A1=0,A0=1 (I soldered it)
}
//.................................................................loop
void loop() { 
  char line[30];
  client = server.available();
  while (client) {
    while(!client.available()) {
      delay(5);
      if (!client.connected()) {client.stop(); break;}
    }
    if (!client.connected()) {client.stop(); break;}
    client.readStringUntil('\n').toCharArray(line,30);
    mlx.readData(mlxdata);
    if (strstr(line,"V?")==line) {
      client.println("ESP01-MLX90393 V1");
    } else if (strstr(line,"ALL?")==line) {
      client.print("ALL "); client.print(mlxdata.x); client.print("\t");
      client.print(mlxdata.y); client.print("\t"); client.print(mlxdata.z);
      client.print("\t"); client.println(mlxdata.t);
    } else if (strstr(line,"BX?")==line) {
      client.print("BX "); client.println(mlxdata.x);
    } else if (strstr(line,"BY?")==line) {
      client.print("BY "); client.println(mlxdata.y);
    } else if (strstr(line,"BZ?")==line) {
      client.print("BZ "); client.println(mlxdata.z);
    } else if (strstr(line,"T?")==line) {
      client.print("T "); client.println(mlxdata.t);
    }
    yield();
  } 
}
