// ESP01-bare-trigger EPICS server, V. Ziemann, 200813
const char* ssid     = "DAQnet";
const char* password = "........";
const int port=1137;
#include <ESP8266WiFi.h>
WiFiServer server(port);
WiFiClient client;

volatile int ICACHE_RAM_ATTR pin_has_changed=0;
void ICACHE_RAM_ATTR action() {   //.....................................action
  pin_has_changed=1;
}

//................................................................setup
void setup() {    
  //...................................static IP
  IPAddress ip(192, 168, 20, 55);
  IPAddress gw(192, 168, 20, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.config(ip,gw,subnet);
  //............................................
  WiFi.begin(ssid,password);
  while (WiFi.status() != WL_CONNECTED) {delay(500);}
  server.begin();
  pinMode(1,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(1), action, FALLING);
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
    if (strstr(line,"V?")==line) {
      client.println("ESP01-bare-trigger V1");
    } else if (strstr(line,"STATUS?")==line) {
      client.print("STATUS "); client.println(pin_has_changed);
      pin_has_changed=0;
    } else if (strstr(line,"GO")==line) {  
      while (client.connected()) {
        yield();
        if (pin_has_changed) {
          client.println("Trigger"); 
          delay(50); 
          pin_has_changed=0;
        }
      } 
      client.stop(); break;
    } 
  } 
}
