#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
//#include <pgmspace.h>
//#include "htmlpages.h"
#include <FS.h>

//#ifndef STASSID
//#define STASSID "#teamChitoge"
//#define STAPSK  "truenibba"
//#endif

//#define DEBUG
#define BUFFERLEN 80

//const char* ssid = STASSID;
//const char* password = STAPSK;

char data[BUFFERLEN] = {};
char dataTemp[BUFFERLEN] = {};
//bool receivedData = false;                           // flag to retry fetching the data if there's no response in the last call


ESP8266WebServer server(80);
ESP8266WiFiMulti wifiMulti;

//const int led = 13;
bool connectionWasAlive = false;

String getContentType(String filename); // convert the file extension to the MIME type
bool handleFileRead(String path);       // send the right file to the client (if it exists)

void handleData() {
  #ifdef DEBUG
  Serial.println("request: /data");
  #endif
  strcpy(dataTemp, data);
  server.send(200, "text/plain", dataTemp);
}

void handleToggle() {
  Serial.println('\x1A');
  server.send(200, "text/plain", "");
}

void handleReset() {
  Serial.println('\x19');
  server.send(200, "text/plain", "");
}

void setup(void) {
  Serial.begin(115200);
  Serial.print("\n");
  wifiMulti.addAP("#teamChitoge", "truenibba");
  wifiMulti.addAP("Ganda & Cute Boarding House", "llaneta583");
  wifiMulti.addAP("Meter Socket", "FabLab2.0");
//  WiFi.mode(WIFI_STA);
//  WiFi.begin(ssid, password);
//  Serial.println("");

  SPIFFS.begin();                                       // Start the SPI Flash Files System

  server.on("/data", handleData);
  server.on("/toggle", handleToggle);
  server.on("/reset", handleReset);
  
  server.onNotFound([]() {                              // If the client requests any URI
    if (!handleFileRead(server.uri()))                  // send it if it exists
      server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
  });

  server.begin();
}

void loop(void) {
  checkConnection();
  server.handleClient();
  fetchData();
  delay(20);
}

void checkConnection() {
  // Wait for connection
  while (wifiMulti.run() != WL_CONNECTED) {
    if (connectionWasAlive == true) {
      #ifdef DEBUG
      Serial.println("Lost WiFi Connection ");
      #endif
      Serial.println('\x12');
      connectionWasAlive = false;
      server.close();
    }
    Serial.print(".");
    delay(500);
  }
  if (connectionWasAlive == false) {
    #ifdef DEBUG
    Serial.println("\nConnected to ");
    Serial.println(WiFi.SSID());
    #endif
    Serial.print('\x11');
    Serial.print(WiFi.localIP());
    Serial.print('\n');
    connectionWasAlive = true;
    server.begin();
  }
}

void fetchData()
{ 
  static unsigned long lastMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - lastMillis > 200) 
  {
    int i = 0;
    if (Serial.available())
    {
      while (Serial.available() && i < BUFFERLEN - 1)
      {
        char temp = (char)Serial.read();
        if (temp == '!')
          i = 0;
        else
        {
          data[i] = temp;
          ++i;
        } 
        
      }
      data[i] = '\0';
      #ifdef DEBUG
      if (i > 0)
        Serial.println("ESP: Data updated.");
      #endif
    } 
  }
  
}

String getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  return "text/plain";
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
  #ifdef DEBUG
  Serial.println("handleFileRead: " + path);
  #endif
  if (path.endsWith("/")) path += "index.html";         // If a folder is requested, send the index file
  String contentType = getContentType(path);            // Get the MIME type
  if (SPIFFS.exists(path)) {                            // If the file exists
    File file = SPIFFS.open(path, "r");                 // Open it
    size_t sent = server.streamFile(file, contentType); // And send it to the client
    file.close();                                       // Then close the file again
    return true;
  }
  #ifdef DEBUG
  Serial.println("\tFile Not Found");
  #endif
  return false;                                         // If the file doesn't exist, return false
}
