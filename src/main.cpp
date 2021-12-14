/*
 * MODULE WIFI DU MODULE AIR
  Code pour faire
  1- un GET sur le site de Aircarto en HTTPS
  2- un POST  sur l'API de AtmoSud
  3 - un GET sur l'API Geoservices d'AmtoSud

  Avec un Ecan OLED SSD1306 pour le debuggage

  Board: NodeMCU 1.0
  Paul Vuarambon
  Aircarto
  septembre 2021

  Configuration Arduino IDE:
    1.  Ouvrir l’IDE Arduino. Aller dans "Fichier => Préférences" :
    2.  Dans "URL de gestionnaire de cartes supplémentaires", entrer :
        http://arduino.esp8266.com/stable/package_esp8266com_index.json
    3.  Cliquer sur "OK".
    4. Aller ensuite dans "Outils => Type de carte => Gestionnaire de carte" et installer "esp8266"

  Pas de wifi Manager
  Pas de geoloc
*/

#include <Arduino.h>

#include <NTPClient.h>

#include <ESP8266WiFi.h>
//#include <WiFiClient.h>
//#include <ESP8266WebServer.h>
//#include <ESP8266HTTPClient.h>
#include "ESPDateTime.h"
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include "requests.h"

Adafruit_SSD1306 display(0x3C, D2, D5); // Address set here 0x3C that I found in the scanner, and pins defined as D2 (SDA/Serial Data), and D5 (SCK/Serial Clock).

/* MODIFIER ICI EN REMPLACANT "ModuleAir" par votre nom de réseau WIFI et votre code WIFI */
const char *nomWIFI = "ModuleAir";
const char *codeWIFI = "ModuleAir";

// nom de l'esp sur le réseau
const char *HostName = "ModuleAir730";

String data;
String json;
String Link;
String formattedDate;
String dayStamp;
int epochTime;

const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars]; // temporary array for use when parsing
char date[] = "2021-12-14";

// variables to hold the parsed data
char DeviceID[numChars] = {0};
int CO2_integer = 0;
float PM10_float = 0.0;
float PM25_float = 0.0;
int PM10_integer = 0;
int PM25_integer = 0;
float latitudePlus = 0.0;
float longitudePlus = 0.0;

float latitude = 43.29855;
float longitude = 5.38658;

boolean newData = false;

const char *polluantPM10 = "pm10";
const char *polluantPM25 = "pm2_5";
const char *polluantNO2 = "no2";
const char *polluantO3 = "o3";
const char *polluantISA = "multi";

void setup()
{
  delay(1000);
  Serial.begin(115200);

  display.begin();
  display.setTextSize(1);              // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);
  display.println("MODULE AIR");
  display.display();

  WiFi.mode(WIFI_OFF); // Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA); // This line hides the viewing of ESP as wifi hotspot

  WiFi.hostname(HostName);
  WiFi.begin(nomWIFI, codeWIFI); // Connect to your WiFi router

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
  Serial.println(nomWIFI);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); // IP address assigned to your ESP

  /*
    display.clear();
    display.drawLine(0, 0, "WIFI connect ok");
    display.drawLine(0, 10, nomWIFI);
    display.display();
  */

  DateTime.setServer("pool.ntp.org");
  DateTime.setTimeZone("CST-1");
  DateTime.begin();

  if (!DateTime.isTimeValid())
  {
    Serial.println("Failed to get time from server.");
  }
  else
  {
    Serial.printf("Date Now is %s\n", DateTime.toISOString().c_str());
    Serial.printf("Timestamp is %ld\n", DateTime.now());
  }
}

//======================================================

void recvWithStartEndMarkers()
{
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (Serial.available() > 0)
  {
    rc = Serial.read();

    Serial.println(rc);

    if (recvInProgress == true)
    {
      if (rc != endMarker)
      {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars)
        {
          ndx = numChars - 1;
        }
      }
      else
      {
        receivedChars[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }

    else if (rc == startMarker)
    {
      recvInProgress = true;
    }
  }
}

//============================================================

void parseData()
{ // split the data into its parts

  char *strtokIndx; // this is used by strtok() as an index

  strtokIndx = strtok(tempChars, ","); // get the first part - the string
  strcpy(DeviceID, strtokIndx);        // copy it to DeviceID

  strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
  CO2_integer = atoi(strtokIndx); // convert this part to an integer

  strtokIndx = strtok(NULL, ",");
  PM25_float = atof(strtokIndx); // convert this part to a float

  strtokIndx = strtok(NULL, ",");
  PM10_float = atof(strtokIndx); // convert this part to a float
}

//===============================================================================================
// IMPRESSION ECRAN SERIAL

void showParsedData()
{

  Serial.print("DeviceID: ");
  Serial.println(DeviceID);
  Serial.print("CO2: ");
  Serial.println(CO2_integer);
  Serial.print("PM25: ");
  Serial.println(PM25_float);
  Serial.print("PM10: ");
  Serial.println(PM10_float);

  // display.drawLine(0, 10, "C02: " + String(CO2_integer) + "  PM25: " + String(PM25_float));
  // display.display();
}

void getDate()
{
  // Serial.printf("Local  Time: %s\n", DateTime.format(DateFormatter::SIMPLE).c_str());
  //  formattedDate = DateTime.format(DateFormatter::SIMPLE).c_str();
  snprintf(date, 11, "%s", DateTime.format(DateFormatter::SIMPLE).c_str());
  Serial.printf("%s\n", date);
}

// ============ LOOP ============

void loop()
{

  recvWithStartEndMarkers();
  if (newData == true)
  {
    strcpy(tempChars, receivedChars);
    // this temporary copy is necessary to protect the original data
    //   because strtok() used in parseData() replaces the commas with \0

    Serial.println("----> NEW DATA RECEIVED FROM ARDUINO MEGA");

    // display.clear();
    // display.drawLine(0, 0, "New data received");
    // display.display();

    parseData();
    showParsedData();
    sendAirCarto(); // envoi des datas sur le serveur de Aircarto
    // sendAtmoSud();       //envoi des datas sur le serveur d'AtmoSud
    getDate();         // récupération de la date
    getAtmoSud_PM10(); // récupération PM_10
    newData = false;
  }

  getDate();
  delay(5000);
}