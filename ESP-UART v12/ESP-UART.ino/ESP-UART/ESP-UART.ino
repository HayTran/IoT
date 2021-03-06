#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>

  // Set up software serial for ESP
SoftwareSerial mySerial(D6,D7); // RX, TX

  // Variable for connect to wifi
const char *ssid = "Hay Tran";
const char *password = "tranvanhay";
int status = WL_IDLE_STATUS;     

  // Variable for connect to Socket Server
const uint16_t port = 4567;         
const char * host = "192.168.0.100"; 

  // Variable for storing data sent to Raspberry
byte arrayValue[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

  // Variable for Serial communication with Arduino
byte countOfArduino = 0;
const byte NUMBER_BUFFER_BYTES_RECEIVE_SERIAL = 17;

  // Variables for communication protocal regulation between server and client socket
char resultFromServer[20];
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
 *  16 bytes for sensor data, 1 byte for strengthWifi, 1 byte for flags, 1 byte for its capacity
 */
const byte BEGIN_SESSION_SENSOR_BYTE = 19;      
/**
 * 16 bytes for sensor data, 1 byte strengthWifi and 1 byte for first confirm session flag
 */
const byte FIRST_CONFIRM_SESSION_SENSOR_BYTE = 18;
const byte SECOND_CONFRIM_SESSION_SENSOR_BYTE = 1;

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
    delay(1000);
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
    client.write(BEGIN_SESSION_SENSOR_BYTE);   //  BEGIN_SESSION_BYTE is relevant NodeType
      // Send sensor value to server
    for (int i = 0; i < BEGIN_SESSION_SENSOR_BYTE - 3; i++){  
      client.write(arrayValue[i]);
    }
      // Send strengthWifi to server
    client.write(strengthWifi);
}
void processSession(){
  if (client.available() == FIRST_CONFIRM_SESSION_SENSOR_BYTE) {
    byte flag = client.read();
    if (flag == FIRST_CONFIRM_SESSION_FLAG) {   // Read first byte and check what confirm is that?
      ind = 0;
      while(client.available()){
         resultFromServer[ind] = client.read();
         ind++;
         delay(5);
      }
      client.stop();
        // Check if true is continue or not begin again
      if (checkResultFromServer()){
          Serial.println("Receive data don't match with send data, try send again!");
          checkNumberTrySendToServer();
      } else {
          secondSession(true);
      }
    }
  } else {
    Serial.println("Don't receive enough bytes, try send again!");
    checkNumberTrySendToServer();
    secondSession(false); 
  }
}
void secondSession(boolean isSuccess){
        // Confirm to server
   Serial.println("Confirm to server !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
   while(!client.connect(host,port)){
     checkNumberTrySendToServer();
     delay(300);
   }
   client.write(SECOND_CONFIRM_SESSION_FLAG);
   client.write(SECOND_CONFRIM_SESSION_SENSOR_BYTE);
   if (isSuccess == true) {
      client.write(SUCCESS_SESSION_FLAG);
   } else {
      client.write(FAILED_SESSION_FLAG);
   }
   client.stop();
}
bool checkResultFromServer(){
  byte numberFailedByteCounter = 0;
    // Check whether or not receive bytes match send byte
  for (int i = 0 ; i < FIRST_CONFIRM_SESSION_SENSOR_BYTE - 2; i++){    // <-- Just read 16 first bytes, 1 byte remain is of strengthWifi,                                                      
     if ( resultFromServer[i] != arrayValue[i]){                       // don't need comparing them. 18 - (1 flag  + 1 strengthWifi)
        numberFailedByteCounter ++;
      }
  }
  Serial.print("Number failed byte counter:");
  Serial.println(numberFailedByteCounter,DEC);
    // Has a greater failer 
  if (numberFailedByteCounter > 0){
    return true;
  } else {
    return false;
  }
}
void checkNumberTrySendToServer(){
  if(numberTrySendToServer <=5){
    numberTrySendToServer ++;
  }
  Serial.print("Number send to server:");
  Serial.println(numberTrySendToServer,DEC);
  if (numberTrySendToServer >= 10) {
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
  while(mySerial.available() < NUMBER_BUFFER_BYTES_RECEIVE_SERIAL){
    Serial.println("Waiting Arduino reply.....");
    delay(200);
    mySerial.write(128);
  }
    // Read sensor value
  for (int i = 0; i < NUMBER_BUFFER_BYTES_RECEIVE_SERIAL - 1; i++){
    arrayValue[i] = mySerial.read();
  }
    // Read arduino nano counter
  countOfArduino = mySerial.read();
  
    // Show result in console
  int temperature = 0;
  temperature = arrayValue[0];  
  int humidity = 0;
  humidity = arrayValue[1];
  int flameValue0 = 0;
  flameValue0 = arrayValue[2] + arrayValue[3]*256;  
  int flameValue1 = 0;
  flameValue1 = arrayValue[4] + arrayValue[5]*256;
  int flameValue2 = 0;
  flameValue2 = arrayValue[6] + arrayValue[7]*256;  
  int flameValue3 = 0;
  flameValue3 = arrayValue[8] + arrayValue[9]*256;  
  int lightIntensity = 0;
  lightIntensity = arrayValue[10] + arrayValue[11]*256;
  int MQ2 = 0;
  MQ2 = arrayValue[12] + arrayValue[13]*256;
  int MQ7 = 0;
  MQ7 = arrayValue[14] + arrayValue[15]*256;     
      // Make sure read out of buffer bytes
  while(mySerial.available()){
    mySerial.read();
  }
  if (temperature < 0 || temperature > 200
    || humidity < 0 || humidity > 200
    || flameValue0 < 0 || flameValue0 > 1023
    || flameValue1 < 0 || flameValue1 > 1023
    || flameValue2 < 0 || flameValue2 > 1023
    || flameValue3 < 0 || flameValue3 > 1023
    || lightIntensity < 0 || lightIntensity > 65535
    || MQ2 < 0 || MQ2 > 1023
    || MQ7 < 0 || MQ7 > 1023){
//      digitalWrite(D1,LOW);
      delay(500);
      comUART();
    } else {
       digitalWrite(D1,HIGH);
    }
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
  Serial.print("========================================Count Of Arduino: ");
  Serial.println(countOfArduino,DEC);
  digitalWrite(D0,HIGH);
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

