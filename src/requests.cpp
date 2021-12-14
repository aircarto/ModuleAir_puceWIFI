#include "Arduino.h"
#include "requests.h"

const char *host = "datasecure.aircarto-asso.fr";
const char *hostATMO = "api.atmosud.org";
const char *hostATMO_geoService = "geoservices.atmosud.org";

StaticJsonDocument<400> doc;

void sendAirCarto()
{

    Serial.println("*** ENVOI SUR AIRCARTO ****");
    Serial.print("connecting to ");
    Serial.println(host);

    WiFiClientSecure client;
    const int httpPort = 443; // 80 is for HTTP / 443 is for HTTPS!

    client.setInsecure(); // this is the magical line that makes everything work

    if (!client.connect(host, httpPort))
    { // works!

        Serial.println("connection failed");
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

    Serial.print("Requesting URL: ");
    Serial.println(url);

    // This will send the request to the server
    client.print(String("GET ") + url + " HTTP/1.0\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");

    Serial.println("Request sent");

    /*
      display.drawLine(0, 20, "Aircarto -> ");
      display.display();
      */

    while (client.connected())
    {
        String line = client.readStringUntil('\n');
        if (line == "\r")
        {
            Serial.println("Headers received");
            // display.drawLine(60, 20, "OK");
            // display.display();
            break;
        }
    }
    String line = client.readStringUntil('\n');

    Serial.println("Reply was:");
    Serial.println("==========");
    Serial.println(line);
    Serial.println("==========");
    Serial.println("Closing connection");
}

void sendAtmoSud()
{

    Serial.println("*** ENVOI SUR ATMOSUD ****");

    Serial.print("connecting to ");
    Serial.println(hostATMO);

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

    Serial.print("Requesting URL: ");
    Serial.println(urlATMO);

    // This will send the request to the server
    client.print(String("POST ") + urlATMO + " HTTP/1.1\r\n" +
                 "Host: " + hostATMO + "\r\n" +
                 "Authorization: 9b748b75fcb8ab2b6a51d7d2cb21bd15\r\n" +
                 "Connection: close\r\n\r\n");

    Serial.println("Request sent");

    // display.drawLine(0, 30, "AtmoSud API -> ");
    // display.display();

    while (client.connected())
    {
        String line = client.readStringUntil('\n');
        if (line == "\r")
        {
            Serial.println("Headers received");
            // display.drawLine(80, 30, "OK");
            // display.display();
            break;
        }
    }
    String line = client.readStringUntil('\n');

    Serial.println("Reply was:");
    Serial.println("==========");
    Serial.println(line);
    Serial.println("==========");
    Serial.println("Closing connection");
}

void getAtmoSud_PM10()
{
    // GET DATA API ATMOSUD
    // attention: marche plus facilement avec une requete HTTP 1.0 et non 1.1
    // necessite la date pour fonctionner au format AAAA-MM-JJ

    Serial.println("*** REQUEST DATA sur API GEOSERVICES  ****");

    Serial.print("connecting to ");
    Serial.println(hostATMO_geoService);

    WiFiClientSecure client;
    const int httpPort = 443; // 80 is for HTTP / 443 is for HTTPS!

    client.setInsecure(); // this is the magical line that makes everything work

    if (!client.connect(hostATMO_geoService, httpPort))
    { // works!
        Serial.println("connection failed");
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
    urlATMO_geoService += date;
    urlATMO_geoService += "&QUERY_LAYERS=azurjour:paca-multi-";
    urlATMO_geoService += date;
    urlATMO_geoService += "&TYPENAME=azurjour:paca-multi-";
    urlATMO_geoService += date;
    urlATMO_geoService += "&srs=EPSG:4326";

    Serial.print("Requesting URL: ");
    Serial.println(urlATMO_geoService);

    // This will send the request to the server
    client.print(String("GET ") + urlATMO_geoService + " HTTP/1.0\r\n" +
                 "Host: " + hostATMO_geoService + "\r\n" +
                 "Connection: close\r\n\r\n");

    Serial.println("Request sent");

    // get the headers
    while (client.connected())
    {
        String line = client.readStringUntil('\n');
        if (line == "\r")
        {
            Serial.println("Headers received");

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

    Serial.println("Reply was:");
    Serial.println("==========");
    Serial.println(data);
    Serial.println("==========");
    Serial.println("Closing connection");
    Serial.println("");
    Serial.println("");
    Serial.println("");

    // la réponse prend cette
    Serial.println("DeserializeJSON:");

    DeserializationError error = deserializeJson(doc, data);
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    JsonObject data = doc["features"][0];

    double data_valeur = data["properties"]["GRAY_INDEX"]; // "46"

    Serial.print("Valeur: ");
    Serial.println(data_valeur);

    // display.drawLine(0, 50, "Valeur -> " + String(data_valeur));
    // display.display();

    // on envoie au MEGA la valeur du polluant
    Serial.println(data_valeur);
}
