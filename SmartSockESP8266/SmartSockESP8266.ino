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

#define BUFFERLEN 80

//const char* ssid = STASSID;
//const char* password = STAPSK;

char data[BUFFERLEN] = {};
//bool receivedData = false;                           // flag to retry fetching the data if there's no response in the last call


ESP8266WebServer server(80);
ESP8266WiFiMulti wifiMulti;

//const int led = 13;
bool connectionWasAlive = false;

String getContentType(String filename); // convert the file extension to the MIME type
bool handleFileRead(String path);       // send the right file to the client (if it exists)

void handleData() {
  Serial.println("request: /data");
  server.send(200, "text/plain", data);
}

void handleToggle() {
  Serial.print("!");
  server.send(200, "text/plain", "");
}


void setup(void) {
  Serial.begin(115200);
  Serial.print("\n");
  wifiMulti.addAP("#teamChitoge", "truenibba");
  wifiMulti.addAP("Ganda & Cute Boarding House", "llaneta583");
//  WiFi.mode(WIFI_STA);
//  WiFi.begin(ssid, password);
//  Serial.println("");

  SPIFFS.begin();                                       // Start the SPI Flash Files System

  server.on("/data", handleData);
  server.on("/toggle", handleToggle);
  
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
      Serial.print("Lost WiFi Connection ");
      connectionWasAlive = false;
      server.close();
    }
    Serial.print(".");
    delay(500);
  }
  if (connectionWasAlive == false) {
    Serial.print("\nConnected to ");
    Serial.println(WiFi.SSID());
    Serial.println(WiFi.localIP());
    connectionWasAlive = true;
    server.begin();
  }
//  Serial.println("");
//  Serial.print("Connected to ");
//  Serial.println(ssid);
//  Serial.print("IP address: ");
}

void fetchData()
{
//  if (Serial.available())
//  {
//    data = Serial.readString();
//    Serial.print("data updated.\n");
//    Serial.println(data);
//  }


  static unsigned int previousMillis = 0;
  unsigned int currentMillis = millis();

  if (currentMillis - previousMillis > 2000)
  {
    previousMillis = currentMillis;
    
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
      if (i > 0)
        Serial.println("ESP: Data updated.");
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
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";         // If a folder is requested, send the index file
  String contentType = getContentType(path);            // Get the MIME type
  if (SPIFFS.exists(path)) {                            // If the file exists
    File file = SPIFFS.open(path, "r");                 // Open it
    size_t sent = server.streamFile(file, contentType); // And send it to the client
    file.close();                                       // Then close the file again
    return true;
  }
  Serial.println("\tFile Not Found");
  return false;                                         // If the file doesn't exist, return false
}
