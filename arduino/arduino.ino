
#include "RCSwitch.h"
#include <SPI.h>
#include <Ethernet.h>
#include "PubSubClient.h"
#include <math.h>

#define WIFIPIN 9

#define AMBILIGHTBLUEEPIN 6
#define AMBILIGHTREDPIN 5
#define AMBILIGHTGREENPIN 3
#define THERMISTORPIN A0       
#define THERMISTORNOMINAL 2200  // resistance at 25 degrees C
#define TEMPERATURENOMINAL 25   // temp. for nominal resistance (almost always 25 C)
#define BCOEFFICIENT 4791       // The beta coefficient of the thermistor (usually 3000-4000)
#define SERIESRESISTOR 2200     // the value of the 'other' resistor


// Prototypes

void setWifi(char* state, char* group, int switchNumber);
void setColor(int r, int g, int b);
float getTemp();
void mqttReceive(char* topic, byte* payload, unsigned int length);
void publishRetained(char* topic, char* payload);



// Settings 
byte mac[]    = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFF, 0xFF };
byte ip[]     = { 192, 168, 8, 4 };
byte mqttServer[] = { 192, 168, 8, 2 };
char* mqttClientId = "homeduino";
char* wifiSwitchHomeGroup = "11011";


  int currentr = 255;
int currentg = 0;
int currentb = 0;


// Global variables
RCSwitch wifiTransmitter = RCSwitch();
boolean publishSensorsFlag = false;
boolean fadeStepFlag = false;
boolean fading = false;
EthernetClient ethClient;
PubSubClient client(mqttServer, 1883, mqttReceive, ethClient);




void setWifi(char* state, char* group, int switchNumber) {  
  if (strcmp(state, "1") == 0) {
    wifiTransmitter.switchOn(group, switchNumber);
    delay(500);
    wifiTransmitter.switchOn(group, switchNumber);  
   Serial.println(group); 
   Serial.println(switchNumber);
      Serial.println("here");

  } else if (strcmp(state, "0") == 0)  {  
    wifiTransmitter.switchOff(group, switchNumber); 
    delay(500);
    wifiTransmitter.switchOff(group, switchNumber); 
  } 
}

void setColor(int r, int g, int b) {    
  analogWrite(AMBILIGHTREDPIN, r);
  analogWrite(AMBILIGHTGREENPIN, g);
  analogWrite(AMBILIGHTBLUEEPIN, b);
}

void publishColor(int r, int g, int b) {
  char buffer[6];
  
  sprintf(buffer, "%.2X%.2X%.2X", r, g, b);
  Serial.println("hrx is");
  Serial.println(buffer);
}

void fadeStep () {

  Serial.println(currentr);
  Serial.println(currentg);
  Serial.println(currentb);

  if (currentr == 255 && currentg == 0  && currentb < 255) {
    currentb++;
  } else if (currentb == 255 && currentg == 0 && currentr > 0) {
    currentr--;
  } else if (currentr ==0 && currentb == 255  && currentg < 255){
    currentg++;
  } else if (currentr == 0 && currentg == 255 && currentb > 0){
    currentb--;
  } else if (currentg == 255 && currentb == 0 && currentr < 255){
    currentr++;
  } else if (currentr == 255 && currentb == 0 && currentg > 0){
    currentg--;
  }
  setColor(currentr, currentg, currentb);
}


float getTemp() {
  float temp;
  temp = analogRead(THERMISTORPIN); 
  temp = 1023 / temp - 1;
  temp = SERIESRESISTOR / temp;  
  temp = temp / THERMISTORNOMINAL;                 // (R/Ro)
  temp = log(temp);                                // ln(R/Ro)
  temp /= BCOEFFICIENT;                            // 1/B * ln(R/Ro)
  temp += 1.0 / (TEMPERATURENOMINAL + 273.15);     // + (1/To)
  temp = 1.0 / temp;                             // Invert
  temp -= 273.15;                                  // convert to C
  return temp;
}



//
//
// MQTT 
//
//

void mqttReceive(char* topic, byte* rawPayload, unsigned int length) {  
  char  payload[length+1];
  memcpy(payload, rawPayload, length);
  payload[length] = '\0';
  
//  Serial.println("Callback");
//  Serial.print("Topic:");
//  Serial.println(topic);
//  Serial.print("Length:");
//  Serial.println(length);
//  Serial.print("Payload:");
//  Serial.print(payload);
//  Serial.println();
  
  if(strcmp(topic, "/devices/321995-ambilight/controls/power") == 0) {
    setWifi((char*)payload, wifiSwitchHomeGroup, 1);
  } else if(strcmp(topic, "/devices/321995-ambilight/controls/fading")  == 0 ) {
    fading  = !(*payload == '0');
  } else if(strcmp(topic, "/devices/862671-wirelessSwitch/controls/power")  == 0) {
    setWifi((char*)payload, wifiSwitchHomeGroup, 2);
  }else if(strcmp(topic, "/devices/558426-wirelessSwitch/controls/power")  == 0) {
    setWifi((char*)payload, wifiSwitchHomeGroup, 3);
  }
}

void publishRetained(char* topic, char* payload) {
//  Serial.print("tx t: ");
//  Serial.print(topic);
//  Serial.print(", tx p: ");
//  Serial.println((char*)payload);
//  Serial.println("");

  client.publish(topic, (uint8_t*)payload, strlen((const char*)payload), true);
}

void publishDeviceMetaInformation() {
 publishRetained("/devices/321995-ambilight/controls/power/type", "switch");
 publishRetained("/devices/321995-ambilight/controls/fading/type", "switch");
 publishRetained("/devices/321995-ambilight/controls/color/type", "range");

 publishRetained("/devices/862671-wirelessSwitch/controls/power/type", "switch");
 publishRetained("/devices/558426-wirelessSwitch/controls/power/type", "switch");

}

void subscribe() {
 client.subscribe("/devices/321995-ambilight/controls/#");
 client.subscribe("/devices/862671-wirelessSwitch/controls/#");
 client.subscribe("/devices/558426-wirelessSwitch/controls/#");  
}

void publishSensors () {
    char tempStr[5];
    dtostrf(getTemp(),2,2,tempStr);
    publishRetained("/sensors/579761-temperature", tempStr);
}


//
//
// Init
//
//



void setup() {
  Serial.begin(9600);
  Serial.println("starting");

  pinMode(AMBILIGHTREDPIN, OUTPUT);
  pinMode(AMBILIGHTGREENPIN, OUTPUT);
  pinMode(AMBILIGHTBLUEEPIN, OUTPUT);
  pinMode(WIFIPIN, OUTPUT);
      analogWrite(AMBILIGHTREDPIN, 255);

    analogWrite(AMBILIGHTBLUEEPIN , 255);

    analogWrite(AMBILIGHTGREENPIN, 255);

  Ethernet.begin(mac, ip);
  if (client.connect(mqttClientId)) {
    subscribe();
    publishDeviceMetaInformation();
  }

  wifiTransmitter.enableTransmit(WIFIPIN);
  setupInterrupts();
}


void setupInterrupts() {
      // initialize Timer1
      cli();             // disable global interrupts
      TCCR1A = 0;        // set entire TCCR1A register to 0
      TCCR1B = 0;
   
      // Overflow interrupt with CLK/1024 Prescaler (~4s) for Sensor readings
      TIMSK1 |= (1 << TOIE1); 
   //   TCCR1B |= (1 << CS10);
   //   TCCR1B |= (1 << CS12);
      
      
      

  TCNT1  = 0;

  OCR1A = 2000;            
  TCCR1B |= (1 << WGM12);   // CTC mode
    TCCR1B |= (1 << CS10); // 64 prescaler for 8hz

  TCCR1B |= (1 << CS11); 

  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
      
      // enable global interrupts:
      sei();
}

ISR(TIMER1_OVF_vect)
{

  publishSensorsFlag = true;
  
}

ISR(TIMER1_COMPA_vect)          
{
  fadeStepFlag = true;
}



void loop() {
  client.loop();

  if (publishSensorsFlag) {
    publishSensors();
    publishSensorsFlag = false;
  }
    
  if (fading) {
    fadeStep(); 
    fadeStepFlag = false;
  }
}

