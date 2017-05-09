#include <SoftwareSerial.h>
#include "DHT.h"  
#include <Wire.h>
#include "BH1750.h"

// Configure for Software UART
SoftwareSerial mySerial(2,3);  // RX, TX
byte valueR = 0;
byte valueW = 255;
bool a = 1;

// Configure for DHT11
const int DHTPIN = 4;       // Communicate by one-wire in pin 4 of Arduino
const int DHTTYPE = DHT11;  // Declare type of sensor
DHT dht(DHTPIN, DHTTYPE);

// Configure for Flame Sensor pin
const int FLAME_0_PIN = A0; 
const int FLAME_1_PIN = A1; 
const int FLAME_2_PIN = A2; 
const int FLAME_3_PIN = A3; 

// Configure for BH1750
/*Main address  0x23 
  secondary address 0x5C 
  connect this sensor as following :
  VCC >>> 3.3V
  SDA >>> A4 
  SCL >>> A5
  addr >> GND or NC
  Gnd >>>Gnd */
BH1750 lightMeter;

// Configure for MQ Gas sensor pin
const int MQ2_PIN = A6;
const int MQ7_PIN = A7;

// Variable to store value from sensors
byte temperature = 0;
byte humidity = 0;
uint16_t flameValue0 = 0;
uint16_t flameValue1 = 0; 
uint16_t flameValue2 = 0; 
uint16_t flameValue3 = 0; 
uint16_t lightIntensity = 0;
uint16_t mq2Value = 0;
uint16_t mq7Value = 0;

// Variable for sent to ESP
byte flameValue0_0 = 0; 
byte flameValue0_1 = 0;
byte flameValue1_0 = 0;
byte flameValue1_1 = 0;
byte flameValue2_0 = 0; 
byte flameValue2_1 = 0;
byte flameValue3_0 = 0;
byte flameValue3_1 = 0;
byte lightIntensity0 = 0;
byte lightIntensity1 = 0;
byte mq2Value0 = 0;
byte mq2Value1 = 0;
byte mq7Value0 = 0;
byte mq7Value1 = 0;

void setup() {
    Serial.begin(115200);
    mySerial.begin(115200);
    pinMode(13,OUTPUT);
    Serial.println("Beginning...");   
       // Set up for DTH11
    dht.begin();
       // Set up for Flame Sensor   
    pinMode(FLAME_0_PIN,INPUT);
    pinMode(FLAME_1_PIN,INPUT);
    pinMode(FLAME_2_PIN,INPUT);
    pinMode(FLAME_3_PIN,INPUT);
      // Set up for MQ 2
    pinMode(MQ2_PIN,INPUT);
      // Set up for MQ 7
    pinMode(MQ7_PIN,INPUT);
       // Set up for BH1750
    lightMeter.begin();
}
void loop() {
   readDHT();
   readFlameSensor();
   readBH1750();
   readMQ();
   comUART();
   delay(1000);
}

void comUART(){
  digitalWrite(13,HIGH);
  delay(10);
  mySerial.write(temperature);
  mySerial.write(humidity);
  mySerial.write(flameValue0_0);
  mySerial.write(flameValue0_1);
  mySerial.write(flameValue1_0);
  mySerial.write(flameValue1_1);
  mySerial.write(flameValue2_0);
  mySerial.write(flameValue2_1);
  mySerial.write(flameValue3_0);
  mySerial.write(flameValue3_1);
  mySerial.write(lightIntensity0);
  mySerial.write(lightIntensity1);
  mySerial.write(mq2Value0);
  mySerial.write(mq2Value1);
  mySerial.write(mq7Value0);
  mySerial.write(mq7Value1);
  mySerial.write(valueW);
  delay(5);
  byte numberOfBufferReceiveByte = 0;
  if (numberOfBufferReceiveByte = mySerial.available()) {
    Serial.print("======================================================== Buffer Receive Byte: ");
    Serial.println(numberOfBufferReceiveByte,DEC);  // Buffer byte
    for (int i = 0; i < numberOfBufferReceiveByte - 1; i ++) {
      int ignoreValue = mySerial.read(); 
    }
    valueR = mySerial.read();
  }
  mySerial.flush(); // This action will refresh buffer in serial communication
  digitalWrite(13,LOW);
  valueW--;
  if(valueW <= 0){
    valueW = 255;
  }
  Serial.print("======================================================== Count of Server: ");
  Serial.println(valueR,DEC);   // read it and send it out Serial1 (pins 0 & 1)
  Serial.print("======================================================== Count of Arduino: ");
  Serial.println(valueW);
  delay(10);
}
void readFlameSensor(){
    flameValue0 = analogRead(FLAME_0_PIN);
    flameValue1 = analogRead(FLAME_1_PIN);
    flameValue2 = analogRead(FLAME_2_PIN);
    flameValue3 = analogRead(FLAME_3_PIN);
    flameValue0_0 = flameValue0 % 256;
    flameValue0_1 = flameValue0 / 256;
    flameValue1_0 = flameValue1 % 256;
    flameValue1_1 = flameValue1 / 256; 
    flameValue2_0 = flameValue2 % 256;
    flameValue2_1 = flameValue2 / 256;
    flameValue3_0 = flameValue3 % 256;
    flameValue3_1 = flameValue3 / 256; 
    Serial.print("Flame sensor 0: ");
    Serial.println(flameValue0); 
    Serial.print("Flame sensor 1: ");
    Serial.println(flameValue1);
    Serial.print("Flame sensor 2: ");
    Serial.println(flameValue2); 
    Serial.print("Flame sensor 3: ");
    Serial.println(flameValue3);  
    delay(10);      
}
void readDHT(){
    humidity = dht.readHumidity();   
    temperature = dht.readTemperature(); 
    Serial.print("Temperature: ");
    Serial.println(temperature);               
    Serial.print("Humidity: ");
    Serial.println(humidity);               
    delay(10);
}
void readBH1750(){
  lightIntensity = lightMeter.readLightLevel();
  lightIntensity0 = lightIntensity % 256;
  lightIntensity1 = lightIntensity / 256;
  Serial.print("Light: ");
  Serial.print(lightIntensity);
  Serial.println(" lux");
  delay(10);
}
void readMQ(){
  mq2Value = analogRead(MQ2_PIN);
  mq7Value = analogRead(MQ7_PIN);
  mq2Value0 = mq2Value % 256;
  mq2Value1 = mq2Value / 256;
  mq7Value0 = mq7Value % 256;
  mq7Value1 = mq7Value / 256;
  Serial.print("MQ2: ");
  Serial.println(mq2Value);
  Serial.print("MQ7: ");
  Serial.println(mq7Value);
  delay(10);      
}

