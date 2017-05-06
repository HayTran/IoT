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

  // Variables for sequences data between server and client socket
char resultFromServer[14];
int ind = 0;
byte countOfArduino = 0;
const byte NUMBER_BUFFER_BYTE_RECEIVE = 13;

  // Variable for strength of Wifi
byte strengthWifi = 0;  
WiFiClient client;
void setup() { 
    Serial.begin(115200);
    mySerial.begin(115200);
      // D1 for UART communiation signal
    pinMode(D1,OUTPUT);
      // D2 for Wifi communication signal
    pinMode(D2,OUTPUT);
    WiFi.mode(WIFI_STA);
    wifiSetUp();
}
void loop() {
    comUART();
    runWifi();
    printWifiStatus();
    delay(700);
}

void wifiSetUp(){
    delay(10);
      // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
     digitalWrite(D2,HIGH);
     delay(400);
     digitalWrite(D2,LOW);
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
    digitalWrite(D2,HIGH);
      //Increase counter variable 
    delay(20);
    Serial.println("Connecting to server socket: ");
    Serial.println(host);
    while(!client.connect(host,port)){
       Serial.print(".");
       digitalWrite(D2,HIGH);
    }
    sendToServer();
      // Ready to read data sent from server
    delay(30);
    receiveFromServer();
    client.stop();
    delay(5);
    Serial.println("closing connection");
    digitalWrite(D2,LOW);
}
void sendToServer(){
    // Ready to send data to server
    delay(5);
    client.write(humidity);
    client.write(temperature);
    client.write(flameValue0_0);
    client.write(flameValue0_1);
    client.write(flameValue1_0);
    client.write(flameValue1_1);
    client.write(lightIntensity0);
    client.write(lightIntensity1);
    client.write(mq2Value0);
    client.write(mq2Value1);
    client.write(mq7Value0);
    client.write(mq7Value1);
}
void receiveFromServer(){
  ind = 0;
    if (client.available() == 12){
        // Read bytes send from server
      while(client.available()){
       resultFromServer[ind] = client.read();
       ind++;
       delay(5);
      }
        // Check if true continue or not send again
      if (checkResultFromServer()){
        Serial.println("Receive data don't match with send data, try send again!");
        client.stop();
        runWifi();
      }
    } else {
      Serial.println("Don't receive enough bytes, try send again!");
      client.stop();
      runWifi();
    }
}
bool checkResultFromServer(){
  // Check whether or not receive bytes match send byte
  if (resultFromServer[0]!=humidity
    || resultFromServer[1]!= temperature
    || resultFromServer[2]!= flameValue0_0
    || resultFromServer[3]!= flameValue0_1
    || resultFromServer[4]!= flameValue1_0
    || resultFromServer[5]!= flameValue1_1
    || resultFromServer[6]!= lightIntensity0
    || resultFromServer[7]!= lightIntensity1
    || resultFromServer[8]!= mq2Value0
    || resultFromServer[9]!= mq2Value1
    || resultFromServer[10]!= mq7Value0
    || resultFromServer[11]!= mq7Value1 ){
        return true;
      } else {
        return false;
      }
}
void comUART(){
      // Begin communicate serial
    digitalWrite(D1,HIGH);
    byte receiveBytes = 0;
    if (receiveBytes = mySerial.available()) { 
          // Read bytes is not needed
       for(int i = 0; i < receiveBytes - NUMBER_BUFFER_BYTE_RECEIVE; i++){
          byte ingoreByte = mySerial.read();
          Serial.println("#############CLEARED################");
       }
       temperature = mySerial.read();   
       humidity = mySerial.read();
       flameValue0_0 = mySerial.read();
       flameValue0_1 = mySerial.read();
       flameValue1_0 = mySerial.read();
       flameValue1_1 = mySerial.read();
       lightIntensity0 = mySerial.read();
       lightIntensity1 = mySerial.read();
       mq2Value0 = mySerial.read();
       mq2Value1 = mySerial.read();
       mq7Value0 = mySerial.read();
       mq7Value1 = mySerial.read();
       countOfArduino = mySerial.read();
    }
    if(resultFromServer[0] >=0){
           mySerial.write(resultFromServer[0]);
       }
    Serial.print("========================================Count Of Arduino: ");
    Serial.println(countOfArduino,DEC);
    mySerial.flush(); // This action will refresh buffer in serial communication
    Serial.print("Temperature: ");
    Serial.println(temperature);
    Serial.print("Humidity: ");
    Serial.println(humidity);
    Serial.print("Flame 1 : ");
    int flameValue0 = flameValue0_0 + flameValue0_1*256;
    Serial.println(flameValue0,DEC);
    int flameValue1 = flameValue1_0 + flameValue1_1*256;
    Serial.print("Flame 2: ");
    Serial.println(flameValue1,DEC);
    Serial.print("Light: ");
    int lightIntensity = lightIntensity0+ lightIntensity1*256;
    Serial.println(lightIntensity,DEC);
    Serial.print("MQ2: ");
    int mq2Value = mq2Value0 + mq2Value1*256;
    Serial.println(mq2Value,DEC);
    Serial.print("MQ7: ");
    int mq7Value = mq7Value0 + mq7Value1*256;
    Serial.println(mq7Value,DEC);
    digitalWrite(D1,LOW);
}
void printWifiStatus(){
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


