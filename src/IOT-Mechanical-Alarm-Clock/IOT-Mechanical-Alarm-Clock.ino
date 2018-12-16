#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include "FS.h"
#include "SSD1306Wire.h"
#include "OLEDDisplayUi.h"

#include "HTML.h"

#define VERSION "0.1a"
#define HOSTNAME "IOT-ALARM-CLOCK"
#define CONFIG "/conf.txt"
#define WEBSERVER_PORT 80

float timeZone = 0.0;
int externalLight = LED_BUILTIN;
long previousMillisDisplay = 0;
long intervalDisplay = 10000;
boolean displayOn = true;
const boolean INVERT_DISPLAY = true; // true = pins at top | false = pins at the bottom

const int externalLight = LED_BUILTIN;
const int buttonPin = D7;
const int I2C_DISPLAY_ADDRESS = 0x3c; // I2C Address of your Display (usually 0x3c or 0x3d)
const int SDA_PIN = D2;
const int SCL_PIN = D5;

SSD1306Wire display(I2C_DISPLAY_ADDRESS, SDA_PIN, SCL_PIN);
OLEDDisplayUi   ui( &display );
const int numberOfFrames = 3;
FrameCallback frames[numberOfFrames];
FrameCallback clockFrame[2];

ESP8266WebServer server(WEBSERVER_PORT);

void handleSystemReset();
int8_t getWifiQuality();
void readSettings();
void writeSettings();
void handleUpdateConfigure();
void handleNotFound();
void handleRoot();
void handleConfigure();
void handleConfigureNoPassword();

void setup() {

}

void loop() {

}
