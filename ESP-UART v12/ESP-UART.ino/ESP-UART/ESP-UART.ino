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

  // Variable for storing data sent to Raspberry
byte arrayValue[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

  // Variable for Serial communication with Arduino
byte countOfArduino = 0;
const byte NUMBER_BUFFER_BYTE_RECEIVE_SERIAL = 17;

  // Variables for sequences data between server and client socket
char resultFromServer[17];
const byte NUMBER_BYTE_RECEIVE_SOCKET = 17;
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
    digitalWrite(D1,HIGH);
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
    checkNumberTrySendToServer();
    comUART();
    runWifi();
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
       checkNumberTrySendToServer();
       delay(300);
    }
    sendToServer();
      // Ready to read data sent from server
    delay(30);
    receiveFromServer();
    client.stop();
    delay(5);
    Serial.println("closing connection");
    digitalWrite(D4,HIGH);
}
void sendToServer(){
    delay(5);
      // Send sensor value to server
    for (int i = 0; i < NUMBER_BYTE_RECEIVE_SOCKET - 1; i++){
      client.write(arrayValue[i]);
    }
      // Send strengthWifi to server
    client.write(strengthWifi);
}
void receiveFromServer(){
  ind = 0;
    if (client.available() == NUMBER_BYTE_RECEIVE_SOCKET){
        // Read bytes send from server
      while(client.available()){
       resultFromServer[ind] = client.read();
       ind++;
       delay(5);
      }
        // Check if true continue or not send again
      if (checkResultFromServer()){
        Serial.println("Receive data don't match with send data, try send again!");
        checkNumberTrySendToServer();
        client.stop();
        runWifi();
      }
    } else {
      Serial.println("Don't receive enough bytes, try send again!");
      checkNumberTrySendToServer();
      client.stop();
      runWifi();
    }
}
bool checkResultFromServer(){
  byte numberFailerCounter = 0;
    // Check whether or not receive bytes match send byte
  for (int i = 0 ; i < NUMBER_BYTE_RECEIVE_SOCKET - 1; i++){
     if ( resultFromServer[i] != arrayValue[i]){
        numberFailerCounter ++;
      }
  }
    // Has a greater failer 
  if (numberFailerCounter > 0){
    return true;
  } else {
    return false;
  }
  Serial.print("Number failer:");
  Serial.println(numberFailerCounter,DEC);
}
void checkNumberTrySendToServer(){
  if(numberTrySendToServer <=5){
    numberTrySendToServer ++;
  }
  Serial.print("Number send to server:");
  Serial.println(numberTrySendToServer,DEC);
  if (numberTrySendToServer >= 5) {
    digitalWrite(D1,LOW);
  } else {
    digitalWrite(D1,HIGH);
  }
}
void comUART(){
    // Begin communicate serial
  digitalWrite(D0,LOW);
    // Send request serial communication to Arduino
  mySerial.write(128);
  delay(200);
    // Block this until receive enough NUMBER_BUFFER_BYTE_RECEIVE_SERIAL bytes
  while(mySerial.available() <= NUMBER_BUFFER_BYTE_RECEIVE_SERIAL){
    Serial.println("Waiting Arduino reply.....");
    mySerial.write(128);
    delay(200);
  }
    // Read sensor value
  for (int i = 0; i < NUMBER_BUFFER_BYTE_RECEIVE_SERIAL - 1; i++){
    arrayValue[i] = mySerial.read();
  }
    // Read arduino nano counter
  countOfArduino = mySerial.read();
  
    // Show result in console
  int temperature = 0;
  temperature = arrayValue[0] + arrayValue[1]*256;  
  int humidity = 0;
  humidity = arrayValue[2] + arrayValue[3]*256;
  int flameValue0 = 0;
  flameValue0 = arrayValue[4] + arrayValue[5]*256;  
  int flameValue1 = 0;
  flameValue1 = arrayValue[6] + arrayValue[7]*256;
  int flameValue2 = 0;
  flameValue2 = arrayValue[8] + arrayValue[9]*256;  
  int flameValue3 = 0;
  flameValue3 = arrayValue[10] + arrayValue[11]*256;  
  int lightIntensity = 0;
  lightIntensity = arrayValue[12] + arrayValue[13]*256;
  int MQ2 = 0;
  MQ2 = arrayValue[14] + arrayValue[15]*256;
  int MQ7 = 0;
  MQ7 = arrayValue[16] + arrayValue[17]*256;     
  Serial.print("Temperature: ");
  Serial.println(temperature,DEC);
  Serial.print("Humidity: ");
  Serial.println(humidity,DEC);
  Serial.print("Flame 0: ");
  Serial.println(flameValue0,DEC);
  Serial.print("Flame 1: ");
  Serial.println(flameValue1,DEC);
  Serial.print("Flame 2: ");
  Serial.println(flameValue2,DEC);
  Serial.print("Flame 3: ");
  Serial.println(flameValue3,DEC);
  Serial.print("Light Intensity: ");
  Serial.println(lightIntensity,DEC);
  Serial.print("MQ2: ");
  Serial.println(MQ2,DEC);
  Serial.print("MQ7: ");
  Serial.println(MQ7,DEC);
    // Make sure read out of buffer bytes
  while(mySerial.available()){
    mySerial.read();
  }
  Serial.print("========================================Count Of Arduino: ");
  Serial.println(countOfArduino,DEC);
  digitalWrite(D0,HIGH);
}
void getWifiStatus(){
    // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

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
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.print(" dBm");
  Serial.print(", ");
  Serial.print(strengthWifi);
  Serial.println();
}

