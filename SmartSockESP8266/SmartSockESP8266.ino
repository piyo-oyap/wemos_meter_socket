#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <EasyDDNS.h>
#include "TinyUPnP.h"
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

#define LISTEN_PORT 80  // http://<IP>:<LISTEN_PORT>/?name=<your string>
#define LEASE_DURATION 36000  // seconds
#define FRIENDLY_NAME "MeterSocket"  // this name will appear in your router port forwarding section
#define DDNS_USERNAME "dehechunac@matra.top"
#define DDNS_PASSWORD "FabLab2.0"
#define DDNS_DOMAIN "metersocket.ddns.net"

TinyUPnP tinyUPnP(20000);  // -1 means blocking, preferably, use a timeout value (ms)

ESP8266WebServer server(LISTEN_PORT);
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

void handleToggleMain() {
  Serial.println('\x1A');
  server.send(200, "text/plain", "");
}

void handleToggle1() {
  Serial.println('\x1C');
  server.send(200, "text/plain", "");
}

void handleToggle2() {
  Serial.println('\x1D');
  server.send(200, "text/plain", "");
}

void handleToggle3() {
  Serial.println('\x1E');
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
  wifiMulti.addAP("anon", "12345678");
  wifiMulti.addAP("Ganda & Cute Boarding House", "llaneta583");
  wifiMulti.addAP("Meter Socket", "FabLab2.0");
//  WiFi.mode(WIFI_STA);
//  WiFi.begin(ssid, password);
//  Serial.println("");

  SPIFFS.begin();                                       // Start the SPI Flash Files System

  server.on("/data", handleData);
  server.on("/toggleMain", handleToggleMain);
  server.on("/toggle1", handleToggle1);
  server.on("/toggle2", handleToggle2);
  server.on("/toggle3", handleToggle3);
  server.on("/reset", handleReset);
  
  server.onNotFound([]() {                              // If the client requests any URI
    if (!handleFileRead(server.uri()))                  // send it if it exists
      server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
  });

  checkConnection();

  boolean portMappingAdded = false;
  tinyUPnP.addPortMappingConfig(WiFi.localIP(), LISTEN_PORT, RULE_PROTOCOL_TCP, LEASE_DURATION, FRIENDLY_NAME);
  while (!portMappingAdded) {
    portMappingAdded = tinyUPnP.commitPortMappings();
  
    if (!portMappingAdded) {
      delay(5000);  // 5 seconds before trying again
    }
  }
}

void loop(void) {
  checkConnection();
  EasyDDNS.update(300000);
  tinyUPnP.updatePortMappings(600000, NULL);  // 10 minutes
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
