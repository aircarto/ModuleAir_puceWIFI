#ifndef REQUESTS_H /* include guards */
#define REQUESTS_H

#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <ArduinoJson.h>

extern WiFiClientSecure client;
extern const byte numChars;
extern char DeviceID[32];
extern int CO2_integer;
extern float PM10_float;
extern float PM25_float;
extern int PM10_integer;
extern int PM25_integer;
extern float latitude;
extern float longitude;
extern float latitudePlus;
extern float longitudePlus;
extern String dayStamp;
extern String data;
extern char date[11];

void sendAirCarto();
void sendAtmoSud();
void getAtmoSud_PM10();
void getAtmoSud_PM2_5();
void getAtmoSud_O3();
void getAtmoSud_NO2();

#endif