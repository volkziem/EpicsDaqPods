// ESP01-MPU9250 EPICS server, V. Ziemann, 200222
const char* ssid     = "DAQnet";
const char* password = "........";
const int port=1137;
#include <ESP8266WiFi.h>
// #include <WiFiClient.h>
// #include <ESP8266WebServer.h>
WiFiServer server(port);
WiFiClient client;

const int MPU9250=0x68;
const int AK8963=0x0C;
char status[30]=" status unknown";

#define SDA 0
#define SCL 3
#include <Wire.h>

void I2Cwrite(int addr, int reg, int value) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.write(value & 0xFF);
  Wire.endTransmission(true); 
}
uint8_t I2Cread(int addr, int reg) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(addr,1);
  delayMicroseconds(5);
  return Wire.read();  
}

void mpu9250_init(int addr) {
  uint8_t b;
  Wire.begin(SDA,SCL);
  b=I2Cread(addr,0x75);
  //client.print("MPU-9250 WHO_AM_I_byte (should be 0x71) = 0x"); Serial.println(b,HEX);
  if (0x71==b) {strcpy(status," status OK");}
  I2Cwrite(addr,0x37,0x02); // enable bypass mode to access magnetometer at address 0x0C
  b=I2Cread(0x0C,0x00); // check WHOAMI of magnetometer
  //client.print("AK8963   WHO_AM_I_byte (should be 0x48) = 0x"); Serial.println(b,HEX);
//I2Cwrite(0x0C,0x0A,0x12);  // request continuous measurements, 16 bit
  Wire.beginTransmission(addr);   // wake up device
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true); 
}
void mpu9250_read_int(int addr, int16_t data[7]) {
  int16_t intval;
  float fval;
  Wire.beginTransmission(addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(addr,14);
  intval=Wire.read()<<8 | Wire.read(); fval=intval/16.384; data[0]=(int)fval;
  intval=Wire.read()<<8 | Wire.read(); fval=intval/16.384; data[1]=(int)fval;
  intval=Wire.read()<<8 | Wire.read(); fval=intval/16.384; data[2]=(int)fval;
  intval=Wire.read()<<8 | Wire.read(); fval=intval/333.87+21.0; data[3]=(int)fval; 
  intval=Wire.read()<<8 | Wire.read(); fval=intval/131.0; data[4]=(int)fval;
  intval=Wire.read()<<8 | Wire.read(); fval=intval/131.0; data[5]=(int)fval;
  intval=Wire.read()<<8 | Wire.read(); fval=intval/131.0; data[6]=(int)fval;
  Wire.endTransmission();
}
void getBfield(float B[3]) {
  const float raw2muTesla=4912./32760.0;
  uint8_t data[7];
  int16_t intval;
  uint8_t i=0;
  B[0]=0; B[1]=0; B[2]=0;
  I2Cwrite(0x0C,0x0A,0x12);  // request continuous measurements, 16 bit
//I2Cwrite(0x0C,0x0A,0x11);  // request single measurement, 16 bit
  while (!(I2Cread(0x0C,0x02) & 0x01)) {;}  // wait for acqusition to complete
  Wire.beginTransmission(0x0C);
  Wire.write(0x03);   // request 6 or 7 bytes from address 3 onwards 
  Wire.endTransmission(false);
  Wire.requestFrom(0x0C,6);
  for (i=0;i<6;i++) {data[i]=Wire.read();}
  intval=((data[3]<<8) | data[2]); B[0]=intval*raw2muTesla;
  intval=((data[1]<<8) | data[0]); B[1]=intval*raw2muTesla; 
  intval=((data[5]<<8) | data[4]); B[2]=intval*raw2muTesla;
//  I2Cwrite(0x0C,0x0A,0x10); // turn off while not used
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
  mpu9250_init(MPU9250);
}
//.................................................................loop
void loop() { 
  char line[30];
  int16_t data[7];
  float B[3]; 
  client = server.available();
  while (client) {
    while(!client.available()) {
      delay(5);
      if (!client.connected()) {client.stop(); break;}
    }
    if (!client.connected()) {client.stop(); break;}
    client.readStringUntil('\n').toCharArray(line,30);
    if (strstr(line,"V?")==line) {
      client.print("ESP01-MPU9250 V1");
      client.println(status);
    } else if (strstr(line,"T?")==line) {
      mpu9250_read_int(MPU9250,data);
      client.print("T "); client.println(data[3]);
    } else if (strstr(line,"B?")==line) {
      getBfield(B);
      client.print("B "); client.print(B[0]); client.print(",");
      client.print(B[1]); client.print(","); client.println(B[2]);
    } else if (strstr(line,"BX?")==line) {
      getBfield(B); client.print("BX "); client.println(B[0]); 
    } else if (strstr(line,"BY?")==line) {
      getBfield(B); client.print("BY "); client.println(B[1]); 
    } else if (strstr(line,"BZ?")==line) {
      getBfield(B); client.print("BZ "); client.println(B[2]);
    } else if (strstr(line,"ACC?")==line) {
      mpu9250_read_int(MPU9250,data);
      client.print("ACC "); client.print(data[0]); client.print(",");
      client.print(data[1]); client.print(","); client.println(data[2]);   
    } else if (strstr(line,"GYRO?")==line) {
      mpu9250_read_int(MPU9250,data);
      client.print("GYRO "); client.print(data[4]); client.print(",");
      client.print(data[5]); client.print(","); client.println(data[6]);  
    } else if (strstr(line,"ACCRANGE ")==line) {
      int val=(int)atof(&line[8]);
      uint8_t controlbyte=val<<3;
      I2Cwrite(MPU9250,0x1C,controlbyte);
    } else if (strstr(line,"GYRORANGE ")==line) {
      int val=(int)atof(&line[9]);
      uint8_t controlbyte=val<<3;
      I2Cwrite(MPU9250,0x1B,controlbyte);
    } else if (strstr(line,"DLPF ")==line) {
      int val=(int)atof(&line[4]);
      uint8_t controlbyte=val;
      I2Cwrite(MPU9250,0x1A,controlbyte);
    } 
    yield();
  } 
}
