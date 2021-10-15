// ESP01-MCP3302/4 for EPICS server, V. Ziemann, 200217
const char* ssid     = "DAQnet";
const char* password = "........";
const int port=1137;
#include <ESP8266WiFi.h>
WiFiServer server(port);
WiFiClient client;

#define npts 512
#include <Ticker.h>
Ticker SampleFast;
volatile uint16_t isamp=0;
int sample_period=1,sample_buffer_ready=0,nsamp=npts;
int sample_buffer[npts];
uint8_t chan=0,single_diff=0;

/* ESP-01 pin assignement */
/*
#define CS   0        // ChipSelect
#define MOSI 2        // MasterOutSlaveIn
#define MISO 3        // MasterInSlaveOut
#define CLK  1        // Clock
*/
/* ESP-01 alternate assignment */
#define CS   1        // ChipSelect
#define MOSI 2        // MasterOutSlaveIn
#define MISO 0        // MasterInSlaveOut
#define CLK  3        // Clock

void mcp3304_init_bb() {   // also works for mcp3302
  pinMode(CS,OUTPUT); 
  pinMode(MOSI,OUTPUT);
  pinMode(MISO,INPUT); 
  pinMode(CLK,OUTPUT); 
  digitalWrite(CS,HIGH); 
  digitalWrite(MOSI,LOW); 
  digitalWrite(CLK,LOW);  
}
int mcp3304_read_bb(int channel, int singel) {  // bit-bang version mcp3302/4
  int adcvalue=0, sign=0;
  byte commandbits=B10000000;  // defaults to differential
  if (singel) {  //singel-ended
    commandbits = B11000000;   // startbit+diff+D2+D1+D0
    commandbits|=(channel & 0x07) << 3; // 5 config bits, MSB first
  } else {       // differential
    commandbits = B10000000;   // startbit+diff+D2+D1+D0
    commandbits|=(channel & 0x03) << 4; // 5 config bits, MSB first
  }
  digitalWrite(CS,LOW);  // chip select
  for (int i=7; i>0; i--){  // clock bits to device
    digitalWrite(MOSI,commandbits&1<<i);
    digitalWrite(CLK,HIGH); // including two null bits
    digitalWrite(CLK,LOW);    
  } 
  sign=digitalRead(MISO);    // first read the sign bit
  digitalWrite(CLK,HIGH);
  digitalWrite(CLK,LOW);  
  for (int i=11; i>=0; i--){
    adcvalue+=digitalRead(MISO)<<i;
    digitalWrite(CLK,HIGH);
    digitalWrite(CLK,LOW); 
  }
  digitalWrite(CS, HIGH); 
  if (sign) {adcvalue = adcvalue-4096;  } 
  return adcvalue;
}
//...............................................samplefast_action
void samplefast_action() {
  sample_buffer[isamp]=mcp3304_read_bb(chan,single_diff);
  isamp++;
  if (nsamp == isamp) {   
     SampleFast.detach();
     isamp=0;
     sample_buffer_ready=1;
  }
}
//.................................................output_waveform
void output_waveform(char *name) {
  client.print(name); //client.println(nsamp); 
  client.print(sample_buffer[0]);   // first sample, no preceeding comma
  for (int k=1;k<nsamp;k++) {
     client.print(","); client.print(sample_buffer[k]); 
  }
  client.println(); 
}

//..................................................................setup
void setup() {
  mcp3304_init_bb();    // initialize MCP3302/4 ADC
  WiFi.begin(ssid,password);
  while (WiFi.status() != WL_CONNECTED) {delay(500);}
  server.begin();
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
      client.println("ESP01-MCP3302/4 V1");
    } else if (strstr(line,"CHA?")==line) {  // differential channel A
      int val=mcp3304_read_bb(0,0);   // channel,sngl=1/diff=0
      client.print("CHA "); client.println(val);
    } else if (strstr(line,"CHB?")==line) {  // differential channel B
      int val=mcp3304_read_bb(1,0);
      client.print("CHB "); client.println(val);
    } else if (strstr(line,"CH0?")==line) {  // single-ended channel 0
      int val=mcp3304_read_bb(0,1);
      client.print("CH0 "); client.println(val);
    } else if (strstr(line,"CH1?")==line) {  // single-ended channel 1
      int val=mcp3304_read_bb(1,1);
      client.print("CH1 "); client.println(val);
    } else if (strstr(line,"CH2?")==line) {  // single-ended channel 2
      int val=mcp3304_read_bb(2,1);
      client.print("CH2 "); client.println(val);
    } else if (strstr(line,"CH3?")==line) {  // single-ended channel 3
      int val=mcp3304_read_bb(3,1);
      client.print("CH3 "); client.println(val);
    } else if (strstr(line,"WFA?")==line) {          // differential channels
      chan=0; single_diff=0; sample_buffer_ready=0;
      SampleFast.attach_ms(sample_period,samplefast_action); 
      while (sample_buffer_ready==0) {yield();}
      output_waveform("WFA ");
    } else if (strstr(line,"WFB?")==line) {
      chan=1; single_diff=0; sample_buffer_ready=0;
      SampleFast.attach_ms(sample_period,samplefast_action); 
      while (sample_buffer_ready==0) {yield();}
      output_waveform("WFB ");
    } else if (strstr(line,"WF0?")==line) {          // single-ended channels
      chan=0; single_diff=1; sample_buffer_ready=0;
      SampleFast.attach_ms(sample_period,samplefast_action); 
      while (sample_buffer_ready==0) {yield();}
      output_waveform("WF0 ");
    } else if (strstr(line,"WF1?")==line) {
      chan=1; single_diff=1; sample_buffer_ready=0;
      SampleFast.attach_ms(sample_period,samplefast_action); 
      while (sample_buffer_ready==0) {yield();}
      output_waveform("WF1 ");
    } else if (strstr(line,"WF2?")==line) {
      chan=2; single_diff=1; sample_buffer_ready=0;
      SampleFast.attach_ms(sample_period,samplefast_action); 
      while (sample_buffer_ready==0) {yield();}
      output_waveform("WF2 ");
    } else if (strstr(line,"WF3?")==line) {
      chan=3; single_diff=1; sample_buffer_ready=0;
      SampleFast.attach_ms(sample_period,samplefast_action); 
      while (sample_buffer_ready==0) {yield();}
      output_waveform("WF3 ");
    } else if (strstr(line,"PERIOD?")==line) {            // sample_period
      client.print("PERIOD "); client.println(sample_period);
    } else if (strstr(line,"PERIOD ")==line) {
      sample_period=(int)atof(&line[6]);
    } else if (strstr(line,"SAMPLES?")==line) {           // nsamp
      client.print("SAMPLES "); client.println(nsamp);
    } else if (strstr(line,"SAMPLES ")==line) {
      nsamp=(int)atof(&line[7]);
      nsamp=max(1,min(npts,nsamp));  // clamp to range
    }
    yield();
  } 
}
