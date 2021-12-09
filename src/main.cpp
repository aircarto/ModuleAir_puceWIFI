#include <Arduino.h>
#include "ESPAsyncWebServer.h"
#include <ESPAsyncTCP.h>
#include "LittleFS.h"

const char *ssid = "Nuage";
const char *password = "2628nuage";

const int led = 2;
const int capteurLuminosite = 34;

AsyncWebServer server(80);

void setup()
{
  Serial.begin(115200);
  Serial.println("Start");

  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
  pinMode(capteurLuminosite, INPUT);

  if (!LittleFS.begin())
  {
    Serial.println("Erreur SPIFFS...");
    return;
  }

  // On ouvre la mémoire de l'ESP8266
  File root = LittleFS.open("/", "r");
  File file = root.openNextFile();

  while (file)
  {
    Serial.print("File: ");
    Serial.println(file.name());
    file.close();
    file = root.openNextFile();
  }

  // ------ WIFI
  WiFi.begin(ssid, password); // Connect to your WiFi router

  Serial.println("");
  Serial.print("Connecting");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  // If connection successful show IP address in serial monitor

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); // IP address assigned to your ESP

  // --------------- SERVEUR
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/index.html", "text/html"); });
  server.on("/w3.css", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/w3.css", "text/css"); });
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/script.js", "text/javascript"); });
  server.on("/lireLuminosite", HTTP_GET, [](AsyncWebServerRequest *request)
            { 
              int val = analogRead(capteurLuminosite);
              String luminosite = String(val);
              request->send(200, "text/plain", luminosite); });
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request)
            { 
              digitalWrite(led, HIGH);
              request->send(200); });
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request)
            { 
              digitalWrite(led, LOW);
              request->send(200); });
  server.begin();
  Serial.println("Serveur actif!");
}

void loop()
{
  // put your main code here, to run repeatedly:
}