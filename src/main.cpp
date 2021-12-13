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
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

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

const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars]; // temporary array for use when parsing

// variables to hold the parsed data
char DeviceID[numChars] = {0};
int CO2_integer = 0;
float PM10_float = 0.0;
float PM25_float = 0.0;
int PM10_integer = 0;
int PM25_integer = 0;
float latitude = 43.29855;
float longitude = 5.38658;
float latitudePlus = 0.0;
float longitudePlus = 0.0;

boolean newData = false;

const char *host = "datasecure.aircarto-asso.fr"; // only google.com not https://google.com
const char *hostATMO = "api.atmosud.org";
const char *hostATMO_geoService = "geoservices.atmosud.org";
const char *polluantPM10 = "pm10";
const char *polluantPM25 = "pm2_5";
const char *polluantNO2 = "no2";
const char *polluantO3 = "o3";
const char *polluantISA = "multi";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

StaticJsonDocument<400> doc;

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

  // Serial.println("");
  // Serial.print("Connecting");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    // Serial.print(".");
  }

  // If connection successful show IP address in serial monitor
  /*
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(nomWIFI);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
  */

  /*
    display.clear();
    display.drawLine(0, 0, "WIFI connect ok");
    display.drawLine(0, 10, nomWIFI);
    display.display();
  */

  timeClient.begin();
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
    // Serial.println(rc);

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

  /*
  Serial.print("DeviceID: ");
  Serial.println(DeviceID);
  Serial.print("CO2: ");
  Serial.println(CO2_integer);
  Serial.print("PM25: ");
  Serial.println(PM25_float);
  Serial.print("PM10: ");
  Serial.println(PM10_float);
  */

  // display.drawLine(0, 10, "C02: " + String(CO2_integer) + "  PM25: " + String(PM25_float));
  // display.display();
}

//===============================================================================================
// SEND DATA AIRCARTO

void sendParseData()
{

  /*
  Serial.println("*** ENVOI SUR AIRCARTO ****");
  Serial.print("connecting to ");
  Serial.println(host);
  */

  WiFiClientSecure client;
  const int httpPort = 443; // 80 is for HTTP / 443 is for HTTPS!

  client.setInsecure(); // this is the magical line that makes everything work

  if (!client.connect(host, httpPort))
  { // works!
    // Serial.println("connection failed");
    return;
  }

  // We now create a URI for the request
  String url = "/data-upload.php";
  url += "?data=";
  url += "{\"id_moduleAir\":";
  url += DeviceID;
  url += ",\"co2\":";
  url += CO2_integer;
  url += ",\"pm10\":";
  url += PM10_float;
  url += ",\"pm25\":";
  url += PM25_float;
  url += "}";

  // Serial.print("Requesting URL: ");
  // Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.0\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");

  // Serial.println("Request sent");

  /*
    display.drawLine(0, 20, "Aircarto -> ");
    display.display();
    */

  while (client.connected())
  {
    String line = client.readStringUntil('\n');
    if (line == "\r")
    {
      // Serial.println("Headers received");
      // display.drawLine(60, 20, "OK");
      // display.display();
      break;
    }
  }
  String line = client.readStringUntil('\n');

  /*
  Serial.println("Reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("Closing connection");
  */
}

//==================================================================================================================
// SEND DATA ATMOSUD

void sendParseDataATMOSUD()
{

  // Serial.println("*** ENVOI SUR ATMOSUD ****");

  // Serial.print("connecting to ");
  // Serial.println(hostATMO);

  WiFiClientSecure client;
  const int httpPort = 443; // 80 is for HTTP / 443 is for HTTPS!

  client.setInsecure(); // this is the magical line that makes everything work

  if (!client.connect(hostATMO, httpPort))
  { // works!
    // Serial.println("connection failed");
    return;
  }

  PM10_integer = PM10_float;
  PM25_integer = PM25_float;

  // We now create a URI for the request
  String urlATMO = "/moduleair/module/730";
  urlATMO += "?pm10=";
  urlATMO += PM10_integer;
  urlATMO += "&pm25=";
  urlATMO += PM25_integer;
  urlATMO += "&hum=";
  urlATMO += CO2_integer;

  // Serial.print("Requesting URL: ");
  // Serial.println(urlATMO);

  // This will send the request to the server
  client.print(String("POST ") + urlATMO + " HTTP/1.1\r\n" +
               "Host: " + hostATMO + "\r\n" +
               "Authorization: 9b748b75fcb8ab2b6a51d7d2cb21bd15\r\n" +
               "Connection: close\r\n\r\n");

  // Serial.println("Request sent");

  // display.drawLine(0, 30, "AtmoSud API -> ");
  // display.display();

  while (client.connected())
  {
    String line = client.readStringUntil('\n');
    if (line == "\r")
    {
      // Serial.println("Headers received");
      // display.drawLine(80, 30, "OK");
      // display.display();
      break;
    }
  }
  String line = client.readStringUntil('\n');
  /*
  Serial.println("Reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("Closing connection");
  */
}

//========================================================================================================
// GET DATA API ATMOSUD
// attention: marche plus facilement avec une requete HTTP 1.0 et non 1.1
// necessite la date pour fonctionner

void getDataAPI_ATMOSUD()
{

  // Serial.println("*** REQUEST DATA sur API GEOSERVICES  ****");

  timeClient.update();
  formattedDate = timeClient.getFormattedDate();
  // Serial.println(formattedDate);

  // Extract date
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  // Serial.print("DATE: ");
  // Serial.println(dayStamp);

  // Serial.print("connecting to ");
  // Serial.println(hostATMO_geoService);

  WiFiClientSecure client;
  const int httpPort = 443; // 80 is for HTTP / 443 is for HTTPS!

  client.setInsecure(); // this is the magical line that makes everything work

  if (!client.connect(hostATMO_geoService, httpPort))
  { // works!
    // Serial.println("connection failed");
    return;
  }

  latitudePlus = latitude + 0.001;
  longitudePlus = longitude + 0.001;

  // We now create a URI for the request
  String urlATMO_geoService = "/geoserver/azurjour/wms?&INFO_FORMAT=application/json&REQUEST=GetFeatureInfo&SERVICE=WMS&VERSION=1.1.1&WIDTH=1&HEIGHT=1&X=1&Y=1";
  urlATMO_geoService += "&BBOX=";
  urlATMO_geoService += String(longitude, 5);
  urlATMO_geoService += ",";
  urlATMO_geoService += String(latitude, 5);
  urlATMO_geoService += ",";
  urlATMO_geoService += String(longitudePlus, 5);
  urlATMO_geoService += ",";
  urlATMO_geoService += String(latitudePlus, 5);
  urlATMO_geoService += "&LAYERS=azurjour:paca-multi-";
  urlATMO_geoService += dayStamp;
  urlATMO_geoService += "&QUERY_LAYERS=azurjour:paca-multi-";
  urlATMO_geoService += dayStamp;
  urlATMO_geoService += "&TYPENAME=azurjour:paca-multi-";
  urlATMO_geoService += dayStamp;
  urlATMO_geoService += "&srs=EPSG:4326";

  // Serial.print("Requesting URL: ");
  // Serial.println(urlATMO_geoService);

  // This will send the request to the server
  client.print(String("GET ") + urlATMO_geoService + " HTTP/1.0\r\n" +
               "Host: " + hostATMO_geoService + "\r\n" +
               "Connection: close\r\n\r\n");

  // Serial.println("Request sent");

  // get the headers
  while (client.connected())
  {
    String line = client.readStringUntil('\n');
    if (line == "\r")
    {
      // Serial.println("Headers received");

      // display.drawLine(0, 40, "AtmoSud GEO -> ");
      // display.display();
      break;
    }
  }
  // String line = client.readStringUntil('\n');

  // get the response
  // get the response ???
  while (client.available())
  {
    char c = client.read();
    // Serial.write(c);
    data += c;
  }
  // String line = client.readStringUntil('\n');

  /*
    Serial.println("Reply was:");
    Serial.println("==========");
    Serial.println(data);
    Serial.println("==========");
    Serial.println("Closing connection");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    */
  // la réponse prend cette
  // Serial.println("DeserializeJSON:");

  DeserializationError error = deserializeJson(doc, data);
  if (error)
  {
    // Serial.print(F("deserializeJson() failed: "));
    // Serial.println(error.f_str());
    return;
  }

  JsonObject data = doc["features"][0];

  double data_valeur = data["properties"]["GRAY_INDEX"]; // "46"

  // Serial.print("Valeur: ");
  // Serial.println(data_valeur);

  // display.drawLine(0, 50, "Valeur -> " + String(data_valeur));
  // display.display();

  // on envoie au MEGA la valeur du polluant
  Serial.println(data_valeur);
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

    // Serial.println("----> NEW DATA RECEIVED FROM ARDUINO MEGA");

    // display.clear();
    // display.drawLine(0, 0, "New data received");
    // display.display();

    parseData();
    showParsedData();
    sendParseData(); // envoi des datas sur le serveur de Aircarto
    // sendParseDataATMOSUD(); //envoi des datas sur le serveur d'AtmoSud
    // getDataAPI_ATMOSUD();   //get pollution from geoserver
    newData = false;
  }
}