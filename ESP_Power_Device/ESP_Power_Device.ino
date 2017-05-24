#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>

  // Set up software serial for ESP
SoftwareSerial mySerial(D6,D7); // RX, TX

  // Variable for connect to wifi
const char *ssid = "tieunguunhi";
const char *password = "tretrau1235";
int status = WL_IDLE_STATUS;     

  // Variable for connect to Socket Server
const uint16_t port = 8080;         
const char * host = "192.168.1.200"; 

  // Variables for communication protocal regulation between server and client socket
byte receivedArray[8];
const byte BEGIN_SESSION_FLAG = 110;
const byte FIRST_CONFIRM_SESSION_FLAG = 120;      
const byte SECOND_CONFIRM_SESSION_FLAG = 130;
const byte END_CONFIRM_SESSION_FLAG = 140;
const byte SUCCESS_SESSION_FLAG = 150;
const byte FAILED_SESSION_FLAG = 160;
const byte REPLY_ALIVE_FLAG = 101;
const byte REQUEST_ALIVE_FLAG = 201; 
/**
 *  Corresponding with NODE_TYPE, it will send to server 18 bytes
 *  1 byte for flag, 1 byte for its capacity, 6 byte data
 */
const byte BEGIN_SESSION_POWDEV_BYTE = 8;      
/**
 * 4 bytes for powdev data, 1 byte for first confirm session flags
 */
const byte FIRST_CONFIRM_SESSION_POWDEV_BYTE = 6;
const byte SECOND_CONFRIM_SESSION_POWDEV_BYTE = 7;
const byte END_CONFRIM_SESSION_POWDEV_BYTE = 1;

int ind = 0;
byte numberTrySendToServer = 0;

  // Variable for strength of Wifi
byte strengthWifi = 0;  
  // Variable for communication with Module sim800L
SoftwareSerial sim808(D6,D7);    // RX, TX
String SDT="0973832930";
  // Declare for controller
byte dev0 = 1, dev1 = 1, buzzer = 1, sim0 = 1, sim1 = 1; 
WiFiClient client;
void setup() { 
      // get resume from EEPROM
    getResume();
      // D0 for UART communiation status
    pinMode(D0,OUTPUT);
    digitalWrite(D0,LOW);
      // D4 for Wifi communication status
    pinMode(D4,OUTPUT);
    digitalWrite(D4,HIGH);
      // D5, D8 for controller
    pinMode(D5,INPUT);
    pinMode(D8,INPUT);
      // Set up for Serial communication
    Serial.begin(115200);
    mySerial.begin(115200);
      // Set up for wifi
    WiFi.mode(WIFI_STA);
    wifiSetUp();
    Serial.println("############################################################## Set up");
    digitalWrite(D0,HIGH);
}
void loop() {
    getWifiStatus();
    numberTrySendToServer = 0;
    runWifi();
    delay(500);
}
void getResume(){
      // D1, D2 for relay
    pinMode(D1,OUTPUT);
    pinMode(D2,OUTPUT);
      // D3 for buzzer 
    pinMode(D3,OUTPUT);
    EEPROM.begin(4);
    dev0 = EEPROM.read(0);
    dev1 = EEPROM.read(1);
    buzzer = EEPROM.read(2);
    sim0 = EEPROM.read(3);
    sim1 = EEPROM.read(4);
    receivedArray[0] = dev0;
    receivedArray[1] = dev1;
    receivedArray[2] = buzzer;
    receivedArray[3] = sim0;
    receivedArray[4] = sim1;
    digitalWrite(D1,dev0);
    digitalWrite(D2,dev1);
    digitalWrite(D3,buzzer);
}
void wifiSetUp(){
    delay(10);
      // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
       digitalWrite(D4,HIGH);
       delay(400);
       digitalWrite(D4,LOW);
       delay(100);
       Serial.print(".");
   }
    Serial.println("");
    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}
void runWifi(){
      // Use WiFiClient class to create TCP connections
    digitalWrite(D4,LOW);
      //Increase counter variable 
    delay(20);
    Serial.println("Connecting to server socket: ");
    Serial.println(host);
    while(!client.connect(host,port)){
       delay(300);
       Serial.println("======================================================= Connecting to server");
    }
    beginSession();
      // Ready to read data sent from server
    delay(30);
    processSession();
    delay(5);
    Serial.println("closing connection");
    digitalWrite(D4,HIGH);
}
void beginSession(){
    delay(5);
    client.write(BEGIN_SESSION_FLAG);
    client.write(BEGIN_SESSION_POWDEV_BYTE);   //  BEGIN_SESSION_POWDEV_BYTE is relevant NodeType
    client.write(strengthWifi);
    client.write(dev0);
    client.write(dev1);
    client.write(buzzer);
    client.write(sim0);
    client.write(sim1);
}
void processSession(){
  if (client.available() == FIRST_CONFIRM_SESSION_POWDEV_BYTE) {
    byte flag = client.read();
    if (flag == FIRST_CONFIRM_SESSION_FLAG) {   // Read first byte and check what confirm is that?
        ind = 0;
        while(client.available()){
           receivedArray[ind] = client.read();
           ind++;
           delay(5);
        }
        // stop a first confirm session
        client.stop();
        // begin second confirm session
        secondSession();
    }
  } else {
    Serial.println("Don't receive enough bytes, try send again!");
    client.stop();
  }
}
void secondSession(){
    // begin a second confirm session
  while(!client.connect(host,port)){
    delay(300);
    Serial.println("======================================================= Connecting to server");
  }
  client.write(SECOND_CONFIRM_SESSION_FLAG);
  client.write(SECOND_CONFRIM_SESSION_POWDEV_BYTE);
  for (int i = 0 ; i < SECOND_CONFRIM_SESSION_POWDEV_BYTE - 2; i++){
     client.write(receivedArray[i]);
  }
  delay(20);
  int resultCode1 = client.read();
  int resultCode2 = client.read();
  client.stop();
  if (resultCode1 == END_CONFIRM_SESSION_FLAG) {
    if (resultCode2 == SUCCESS_SESSION_FLAG) {
        Serial.println("Success, using result to IO...");
        checkStatus();
    } else if (resultCode2 == FAILED_SESSION_FLAG){
        Serial.println("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Session failed!");
    } else {
        Serial.println("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Failed with unknow error");
    }
  } 
  else {
      Serial.println("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Server reply wrong!!! R1,R2");
      Serial.println(resultCode1,DEC);
      Serial.println(resultCode1,DEC);
      Serial.println("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%");
  }
}
void checkStatus(){
  if (receivedArray[0] != dev0 
      || receivedArray[1] != dev1
      || receivedArray[2] != buzzer 
      || receivedArray[3] != sim0
      || receivedArray[4] != sim1){
            // Update for variable when has change
          dev0 = receivedArray[0]; 
          dev1 = receivedArray[1];
          buzzer = receivedArray[2];
          sim0 = receivedArray[3];
          sim1 = receivedArray[4];
            //  Update into EEPROM
          for (byte i = 0; i < 5; i ++){
            EEPROM.write(i,receivedArray[i]);
            delay(5);
          }
          EEPROM.commit();
          Serial.println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Write into EEPROM");
          digitalWrite(D1,dev0);
          digitalWrite(D2,dev1);
          digitalWrite(D3,buzzer);
      }
}
void SIM(){
  Serial.println("Sending SMS");
//  delay(500);
//  at("AT",100);
//  at("AT+CMGF=1",50);
//  at("AT+CSCS=\"GSM\"",50);
//  at("AT+CMGS=\"" + SDT+"\"",50);
//  at("Dev1 and Dev2 is on: " + String(number),300);
//  sim808.write(26);     // ctlr+Z
//  Serial.println("Send SMS finished");
// delay(1000);
  at("ATD"+SDT+";",10);
 
}
void at(String _atcm,unsigned long _dl){
  sim808.print(_atcm+"\r\n");
  delay(_dl);
}
void getWifiStatus(){
  IPAddress ip = WiFi.localIP();
    // print the received signal strength:
  long rssi = WiFi.RSSI();
  if (rssi < -30 && rssi > -65) {
    strengthWifi = 5;  
  } else if (rssi <= -65 && rssi > -70) {
    strengthWifi = 4;
  } else if (rssi <= -70 && rssi > -80) {
    strengthWifi = 3;
  } else if (rssi <= -70 && rssi > -80) {
    strengthWifi = 2;
  } else if (rssi > -90) {
    strengthWifi = 1;
  }
}

