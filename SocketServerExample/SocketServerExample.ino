
#include <ESP8266WiFi.h>

char *ssid = "tieunguunhi"; //  your network SSID (name)
char *pass = "tretrau1235";    // your network password (use for WPA, or use as key for WEP)

int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

WiFiServer server(8080);
WiFiClient client;
char data[1500];
int ind = 0;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // start the server:
  server.begin();
  // you're connected now, so print out the status:
  printWifiStatus();
 
}


void loop() {
  // put your main code here, to run repeatedly:
   // listen for incoming clients
 client = server.available();
  if (client){
    Serial.println("Client connected");
    while (client.connected()){
        // Read the incoming TCP command
        String command = ReadTCPCommand(&client);
        // Debugging display command
        command.trim();
        Serial.println(command);
        // Phrase the command
        PhraseTCPCommand(&client, &command);
    }
  } 
}


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}


