// ESP01-MCP4922 diual for EPICS server, V. Ziemann, 200220
const char* ssid     = "DAQnet";
const char* password = "........";
const int port=1137;
#include <ESP8266WiFi.h>
WiFiServer server(port);
WiFiClient client;

/* ESP-01 pin assignment */
#define CS1  1        // ChipSelect channel 1
#define CS2  3        // ChipSelect channel 2
#define MOSI 2        // MasterOutSlaveIn
#define CLK  0        // Clock
// #define MISO 0     // MasterInSlaveOut, not used here
//..........................................................dac_init_bb
void dac_init_bb() {  
  pinMode(CS1,OUTPUT); 
  pinMode(CS2,OUTPUT); 
  pinMode(MOSI,OUTPUT);
  pinMode(CLK,OUTPUT); 
  digitalWrite(CS1,HIGH); 
  digitalWrite(CS2,HIGH); 
  digitalWrite(MOSI,LOW); 
  digitalWrite(CLK,LOW);  
}
//...........................................................dac_set_bb
void dac_set_bb(uint16_t val,int cspin) {
  digitalWrite(cspin,LOW);  // chip select
  delay(1);
  for (int i=15; i>=0; i--){   // clock bits to device
    digitalWrite(MOSI,(val&(1<<i))>>i);
    digitalWrite(CLK,HIGH);
    digitalWrite(CLK,LOW);    
  } 
  digitalWrite(cspin,HIGH);  // chip select 
}
//................................................................setup
void setup() {
  WiFi.begin(ssid,password);
  while (WiFi.status() != WL_CONNECTED) {delay(500);}
  server.begin();
  dac_init_bb();
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
      client.println("ESP01-MCP4922 V1");
    } else if (strstr(line,"DACA ")==line) {
      uint16_t val=(int)atof(&line[4]);
      val|=(B0011 << 12);  // 0=chA,0=unBuf,1=x1,1=ON
      dac_set_bb(val,CS1);
    } else if (strstr(line,"DACB ")==line) {
      uint16_t val=(int)atof(&line[4]);
      val|=(B1011 << 12);  // 1=chB,0=unBuf,1=x1,1=ON
      dac_set_bb(val,CS1);  
    } else if (strstr(line,"DACC ")==line) {
      uint16_t val=(int)atof(&line[4]);
      val|=(B0011 << 12);  // 0=chA,0=unBuf,1=x1,1=ON
      dac_set_bb(val,CS2);
    } else if (strstr(line,"DACD ")==line) {
      uint16_t val=(int)atof(&line[4]);
      val|=(B1011 << 12);  // 1=chB,0=unBuf,1=x1,1=ON
      dac_set_bb(val,CS2);  
    }
    yield();
  } 
}
