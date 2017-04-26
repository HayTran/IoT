#include <ESP8266WiFi.h>

// Variable for connect to wifi
const char *ssid = "tieunguunhi";
const char *password = "tretrau1235";
int status = WL_IDLE_STATUS;     

// Variable for connect to Socket Server
const uint16_t port = 8080;         
const char * host = "192.168.1.200"; 

// Variable for storing data sent to Raspberry
byte humidity = 0;
byte temperature = 0;
byte flameValue0_0 = 0; 
byte flameValue0_1 = 0;
byte flameValue1_0 = 0;
byte flameValue1_1 = 0;
byte lightIntensity0 = 0;
byte lightIntensity1 = 0;
byte mq2Value0 = 0;
byte mq2Value1 = 0;
byte mq7Value0 = 0;
byte mq7Value1 = 0;

int countOfServer = -1;
int countOfArduino = 0;

// Variable sleep time for ESP8266
const int sleepTimeS = 100;

void setup() {   
    Serial.begin(115200);
    pinMode(2,OUTPUT);
    wifiSetUp();
    runSerial();
    runWifi();
}
void loop() {
    digitalWrite(2,HIGH);
    for(int i = 0; i<5; i++){
      runSerial();
      delay(200);
    }
    runWifi();
    digitalWrite(2,LOW);
}

void wifiSetUp(){
    delay(10);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
     delay(500);
   }
}
void runWifi(){
    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    //Increase counter variable 
    delay(30);
    while(!client.connect(host,port)){
       delay(300);
    }
    // Ready to send data to server
    delay(10);
    client.flush();
    client.write(humidity);
    client.flush();
    client.write(temperature);
    client.flush();
    client.write(flameValue0_0);
    client.flush();
    client.write(flameValue0_1);
    client.flush();
    client.write(flameValue1_0);
    client.flush();
    client.write(flameValue1_1);
    client.flush();
    client.write(lightIntensity0);
    client.flush();
    client.write(lightIntensity1);
    client.flush();
    client.write(mq2Value0);
    client.flush();
    client.write(mq2Value1);
    client.flush();
    client.write(mq7Value0);
    client.flush();
    client.write(mq7Value1);
    client.flush();
    delay(30);
    // Ready to read data sent from server
    while(client.available()){
      countOfServer = client.read();
    }
    delay(5);
    client.stop();
}

void runSerial(){
  // Begin communicate serial
    if (Serial.available()) { 
       temperature = Serial.read();   
       humidity = Serial.read();
       flameValue0_0 = Serial.read();
       flameValue0_1 = Serial.read();
       flameValue1_0 = Serial.read();
       flameValue1_1 = Serial.read();
       lightIntensity0 = Serial.read();
       lightIntensity1 = Serial.read();
       mq2Value0 = Serial.read();
       mq2Value1 = Serial.read();
       mq7Value0 = Serial.read();
       mq7Value1 = Serial.read();
       countOfArduino = Serial.read();
    }
//    if(countOfServer >=0){
//           Serial.write(countOfServer);
//       }
    Serial.flush();
    delay(10);
}


