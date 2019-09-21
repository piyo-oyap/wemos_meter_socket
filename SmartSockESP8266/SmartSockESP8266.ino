#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
//#include <pgmspace.h>
#include "htmlpages.h"

#ifndef STASSID
#define STASSID "#teamChitoge"
#define STAPSK  "truenibba"
#endif

#define BUFFERLEN 50

const char* ssid = STASSID;
const char* password = STAPSK;

char data[50] = {};

ESP8266WebServer server(80);

const int led = 13;

void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/html", root);
  digitalWrite(led, 0);
}

void handleData() {
  server.send(200, "text/plain", data);
}

void handleToggle() {
  Serial.print("!");
  server.send(200, "text/plain", "");
}

void handleCSS() {
  server.send(200, "text/css", purecss);
}

void handleCSSResp() {
  server.send(200, "text/css", purecssresp);
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup(void) {
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
//  Serial.println("");

  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
//  Serial.println("");
//  Serial.print("Connected to ");
//  Serial.println(ssid);
//  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);

  server.on("/data", handleData);

  server.on("/toggle", handleToggle);

  server.on("/pure-min.css", handleCSS);

  server.on("/grids-responsive-min.css", handleCSSResp);

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
//  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  fetchData();
  delay(500);
}

void fetchData()
{
//  if (Serial.available())
//  {
//    data = Serial.readString();
//    Serial.print("data updated.\n");
//    Serial.println(data);
//  }
    
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
    while (Serial.available())
      Serial.read();
  }
}
