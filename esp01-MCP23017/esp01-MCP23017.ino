// ESP01-MCP23017 EPICS server, V. Ziemann, 200813
const char* ssid     = "DAQnet";
const char* password = "........";
const int port=1137;
#include <ESP8266WiFi.h>
// #include <WiFiClient.h>
// #include <ESP8266WebServer.h>
WiFiServer server(port);
WiFiClient client;
#include <EEPROM.h>

volatile int portA=0,portB=0;
const int MCP23017=0x20;
int SavedState[22];

#define SDA 0
#define SCL 3
#include <Wire.h>

void I2Cwrite(int addr, int reg, int value) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.write(value & 0xFF);
  Wire.endTransmission(true); 
}

int I2Cread(int addr, int reg) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(addr,1);
  return Wire.read();  
}

void restore_state() {
  EEPROM.begin(512);  
  EEPROM.get(0x00,SavedState);
  I2Cwrite(MCP23017,0x00,SavedState[0x00]);  // IODIRA
  I2Cwrite(MCP23017,0x01,SavedState[0x01]);  // IODIRB
  I2Cwrite(MCP23017,0x02,SavedState[0x02]);  // IPOLA
  I2Cwrite(MCP23017,0x03,SavedState[0x03]);  // IPOLB  
  I2Cwrite(MCP23017,0x0C,SavedState[0x0C]);  // GPPUA
  I2Cwrite(MCP23017,0x0D,SavedState[0x0D]);  // GPPUB
  I2Cwrite(MCP23017,0x14,SavedState[0x14]);  // OLATA
  I2Cwrite(MCP23017,0x15,SavedState[0x15]);  // OLATB
  EEPROM.end();
}

void mcp23017_init(int addr) {
  Wire.begin(SDA,SCL);
  I2Cwrite(addr,0x00,0x00);    // IODIRA
  I2Cwrite(addr,0x0D,0xFF);    // GPPUB

  I2Cwrite(addr,0x04,0xFF);    // GPINTENA
  I2Cwrite(addr,0x06,0xFF);    // DEFVALA
  I2Cwrite(addr,0x08,0x00);    // INTCONA 0=change, 1=!defval
  
  I2Cwrite(addr,0x05,0xFF);    // GPINTENB
  I2Cwrite(addr,0x07,0xFF);    // DEFVALB
  I2Cwrite(addr,0x09,0x00);    // INTCONB 0=change, 1=!defval

  I2Cwrite(addr,0x0A,0x02);    // IOCON, INTPOL=1    
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
  pinMode(2,INPUT_PULLUP);
  mcp23017_init(MCP23017); 
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
      client.println("ESP01-MCP23017 V1");
    } else if (strstr(line,"SAVE")==line) {
      EEPROM.begin(512);  
      for (int k=0;k<22;k++) {SavedState[k]=I2Cread(MCP23017,k);}
      EEPROM.put(0x00,SavedState);
      EEPROM.commit();
      EEPROM.end();
    } else if (strstr(line,"RESTORE")==line) { 
      restore_state();
    } else if (strstr(line,"SHOW")==line) { 
      for (int k=0;k<22;k++) {
        client.print(k); client.print(": "); 
        client.println(I2Cread(MCP23017,k)); 
      }
    } else if (strstr(line,"IODIRA?")==line) {
      int tmp=I2Cread(MCP23017,0x00); 
      client.print("IODIRA "); client.println(tmp);
    } else if (strstr(line,"IODIRA ")==line) {
      int tmp=(int)atof(&line[7]);
      I2Cwrite(MCP23017,0x00,tmp & 0xFF); 
    } else if (strstr(line,"IODIRB?")==line) {
      int tmp=I2Cread(MCP23017,0x01); 
      client.print("IODIRB "); client.println(tmp);
    } else if (strstr(line,"IODIRB ")==line) {
      int tmp=(int)atof(&line[7]);
      I2Cwrite(MCP23017,0x01,tmp & 0xFF);   
    } else if (strstr(line,"GPPUA?")==line) {
      int tmp=I2Cread(MCP23017,0x0C); 
      client.print("GPPUA "); client.println(tmp);
    } else if (strstr(line,"GPPUA ")==line) {
      int tmp=(int)atof(&line[6]);
      I2Cwrite(MCP23017,0x0C,tmp & 0xFF); 
    } else if (strstr(line,"GPPUB?")==line) {
      int tmp=I2Cread(MCP23017,0x0D); 
      client.print("GPPUB "); client.println(tmp);
    } else if (strstr(line,"GPPUB ")==line) {
      int tmp=(int)atof(&line[6]);
      I2Cwrite(MCP23017,0x0D,tmp & 0xFF); 
    } else if (strstr(line,"A?")==line) {  
      portA=I2Cread(MCP23017,0x12); 
      client.print("A "); client.println(portA);
    } else if (strstr(line,"A ")==line) {
      portA=(int)atof(&line[2]);
      I2Cwrite(MCP23017,0x14,portA & 0xFF);  
    } else if (strstr(line,"A")==line) {
      int tmp=line[1]-'0';
      uint8_t mask=0x01 << tmp; // (line[1]-'0');     
      if (line[2] == '?') {
        portA=I2Cread(MCP23017,0x12);
        client.print(line[0]); client.print(line[1]); client.print(" ");
        client.println((portA & mask) >> tmp);  
      } else {
        int val=(int)atof(&line[3]);
        if (val==0) {portA &= ~mask;} else {portA |= mask;}
        I2Cwrite(MCP23017,0x014,portA);  // write OLATA
      }       
    } else if (strstr(line,"B?")==line) {
      portB=I2Cread(MCP23017,0x13);   // read GPIOB
      client.print("B "); client.println(portB);
    } else if (strstr(line,"B ")==line) {
      portB=(int)atof(&line[2]);
      I2Cwrite(MCP23017,0x15,portB & 0xFF); // OLATB
    } else if (strstr(line,"B")==line) {
      int tmp=line[1]-'0';
      uint8_t mask=0x01 << tmp; // (line[1]-'0');  
      if (line[2] == '?') {
        portB=I2Cread(MCP23017,0x13);
        client.print(line[0]); client.print(line[1]); client.print(" ");
        client.println((portB & mask) >> tmp);  
      } else {
        int val=(int)atof(&line[3]);
        if (val==0) {portB &= ~mask;} else {portB |= mask;}
        I2Cwrite(MCP23017,0x015,portB);  // write OLATB
      } 
    } else if (strstr(line,"OLATA?")==line) {
      portA=I2Cread(MCP23017,0x14); 
      client.print("OLATA "); client.println(portA);
    } else if (strstr(line,"OLATA ")==line) {
      portA=(int)atof(&line[6]);
      I2Cwrite(MCP23017,0x14,portA & 0xFF); 
    } else if (strstr(line,"OLATB?")==line) {
      portB=I2Cread(MCP23017,0x15); 
      client.print("OLATB "); client.println(portB);
    } else if (strstr(line,"OLATB ")==line) {
      portB=(int)atof(&line[6]);
      I2Cwrite(MCP23017,0x15,portB & 0xFF); 
    } else if (strstr(line,"IPOLA?")==line) {
      portA=I2Cread(MCP23017,0x02); 
      client.print("IPOLA "); client.println(portA);
    } else if (strstr(line,"IPOLA ")==line) {
      portA=(int)atof(&line[6]);
      I2Cwrite(MCP23017,0x02,portA & 0xFF); 
    } else if (strstr(line,"IPOLB?")==line) {
      portB=I2Cread(MCP23017,0x03); 
      client.print("IPOLB "); client.println(portB);
    } else if (strstr(line,"IPOLB ")==line) {
      portB=(int)atof(&line[6]);
      I2Cwrite(MCP23017,0x03,portB & 0xFF); 
    } else if (strstr(line,"INTA?")==line) {
      client.print("INTA "); client.println(digitalRead(1));
    } else if (strstr(line,"INTB?")==line) {
      client.print("INTB "); client.println(digitalRead(2));
    } else if (strstr(line,"INTFA?")==line) {
      portA=I2Cread(MCP23017,0x0E); 
      client.print("INTFA "); client.println(portA);
    } else if (strstr(line,"INTCAPA?")==line) {
      portA=I2Cread(MCP23017,0x10); 
      client.print("INTCAPA "); client.println(portA);
    } else if (strstr(line,"INTCONA?")==line) {
      portA=I2Cread(MCP23017,0x08); 
      client.print("INTCONA "); client.println(portA);
    } else if (strstr(line,"INTFB?")==line) {
      portB=I2Cread(MCP23017,0x0F); 
      client.print("INTFB "); client.println(portB);
    } else if (strstr(line,"INTCAPB?")==line) {
      portB=I2Cread(MCP23017,0x11); 
      client.print("INTCAPB "); client.println(portB);
    } else if (strstr(line,"INTCONB?")==line) {
      portB=I2Cread(MCP23017,0x09); 
      client.print("INTCONB "); client.println(portB);
    } else if (strstr(line,"IOCON?")==line) {
      portA=I2Cread(MCP23017,0x0A); 
      client.print("IOCON "); client.println(portA);
    } else if (strstr(line,"IOCON ")==line) {
      portA=(int)atof(&line[6]);
      I2Cwrite(MCP23017,0x0A,portA & 0xFF);   
    } 
    yield();
  } 
}
