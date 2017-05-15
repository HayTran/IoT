#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>

  // Set up software serial for ESP
SoftwareSerial mySerial(D6,D7); // RX, TX

  // Variable for connect to wifi
const char *ssid = "tieunguunhi";
const char *password = "tretrau1235";
int status = WL_IDLE_STATUS;     

  // Variable for connect to Socket Server
const uint16_t port = 8080;         
const char * host = "192.168.1.100"; 

  // Variables for communication protocal regulation between server and client socket
byte receivedArray[8];
const byte BEGIN_SESSION_FLAG = 110;
const byte FIRST_CONFIRM_SESSION_FLAG = 120;      
const byte SECOND_CONFIRM_SESSION_FLAG = 130;
const byte END_CONFIRM_SESSION_FLAG = 140;
const byte RESULT_SESSION_FLAG = 150;
const byte REPLY_ALIVE_FLAG = 101;
const byte REQUEST_ALIVE_FLAG = 201; 
/**
 *  Corresponding with NODE_TYPE, it will send to server 18 bytes
 *  1 byte for flag, 1 byte for its capacity
 */
const byte BEGIN_SESSION_POWDEV_BYTE = 2;      
/**
 * 4 bytes for powdev data, 1 byte for first confirm session flags
 */
const byte FIRST_CONFIRM_SESSION_POWDEV_BYTE = 5;
const byte SECOND_CONFRIM_SESSION_POWDEV_BYTE = 6;
const byte END_CONFRIM_SESSION_POWDEV_BYTE = 1;

int ind = 0;
byte numberTrySendToServer = 0;

  // Variable for strength of Wifi
byte strengthWifi = 0;  

WiFiClient client;
void setup() { 
    Serial.begin(115200);
    mySerial.begin(115200);
      // D0 for UART communiation status
    pinMode(D0,OUTPUT);
      // D1 for alarm
    pinMode(D1,OUTPUT);
    digitalWrite(D1,LOW);
    pinMode(D2,OUTPUT);
    digitalWrite(D2,LOW);
      // D4 for Wifi communication status
    pinMode(D4,OUTPUT);
    WiFi.mode(WIFI_STA);
    wifiSetUp();
    digitalWrite(D0,HIGH);
    digitalWrite(D4,HIGH);
}
void loop() {
    getWifiStatus();
    numberTrySendToServer = 0;
    runWifi();
    delay(500);
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
}
void processSession(){
  if (client.available() == FIRST_CONFIRM_SESSION_POWDEV_BYTE) {
    byte flag = client.read();
    if (flag == FIRST_CONFIRM_SESSION_FLAG) {   // Read first byte and check what confirm is that?
      Serial.print("client.available before: ");
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
    runWifi(); 
  }
}
void secondSession(){
    // begin a second confirm session
  while(!client.connect(host,port)){
    delay(300);
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
  Serial.print("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& Result code1:");
  Serial.println(resultCode1,DEC);
  Serial.print("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& Result code2:");
  Serial.println(resultCode2,DEC);
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
    // print your WiFi shield's IP address:
//  Serial.print("IP Address: ");
//  Serial.println(ip);
//  Serial.print("signal strength (RSSI):");
//  Serial.print(rssi);
//  Serial.print(" dBm");
//  Serial.print(", ");
//  Serial.print(strengthWifi);
//  Serial.println();
}

