#include <AFArray.h>
#include <AFArrayType.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <WiFiUdp.h>
#include <Adafruit_NeoPixel.h>
#include "FS.h"
#include "SSD1306Wire.h"
#include "OLEDDisplayUi.h"

#include "HTML.h"

#define VERSION "0.8a"
#define HOSTNAME "IOT-ALARM-CLOCK"
#define CONFIG "/conf.txt"
#define WEBSERVER_PORT 80
#define NUMPIXELS 300 //change to however many are used

int timeZone = 0;
long previousMillisDisplay = 0;
long previousMillisLight = 0;
long intervalDisplay = 10000;
boolean displayOn = true;
boolean twelveHour = true;
boolean alarmEnabled = true;
boolean firstOff = true;
const boolean INVERT_DISPLAY = true; // true = pins at top | false = pins at the bottom

const int buttonPin3 = A0;    //for the alarm snooze
const int DATA = D0;          //for the shift register
const int CLK = D1;           //for the shift register
const int SDA_PIN = D2;       //for the screen
const int LATCH = D3;         //for the shift register
const int speaker = D4;       //for the speaker/alarm (figure this out later)
const int SCL_PIN = D5;       //for the screen
const int buttonPin2 = D6;    //for the lights
const int buttonPin1 = D7;    //for the screen
const int neopixelPin =  D8;  //for the lights

const int I2C_DISPLAY_ADDRESS = 0x3c; // I2C Address of your Display (usually 0x3c or 0x3d)

const byte segmentOn[11] = {
  B00111111,  //0
  B00000110,  //1
  B01011011,  //2
  B01001111,  //3
  B01100110,  //4
  B01101101,  //5
  B01111101,  //6
  B00000111,  //7
  B01111111,  //8
  B01101111,   //9
  B00000000   //blank
};

const byte segmentOff[11] = {
  B11000000,  //0
  B11111001,  //1
  B10100100,  //2
  B10110000,  //3
  B10011001,  //4
  B10010010,  //5
  B10000010,  //6
  B11111000,  //7
  B10000000,  //8
  B10010000,   //9
  B11111111   //blank
};

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, neopixelPin, NEO_GRB + NEO_KHZ800);

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
void initializePixels();
uint32_t Wheel(byte WheelPos);
void rainbowCycle();

int rainbowCycleLoop0 = 0;
int lastHour = 0;
int lastMinute = 0;

void setup() {
  Serial.begin(115200);
  SPIFFS.begin();
  delay(10);

  Serial.println();
  pinMode(buttonPin1, INPUT);
  pinMode(buttonPin2, INPUT);
  pinMode(buttonPin3, INPUT);
  pinMode(speaker, OUTPUT);
  pinMode(LATCH, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(DATA, OUTPUT);

  timeClient.setTimeOffset(3600);
  timeClient.setUpdateInterval(60000);

  readSettings();

  //initialize display
  display.init();
  if (INVERT_DISPLAY) {
    display.flipScreenVertically(); // connections at top of OLED display
  }

  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_10);
  display.drawString(64, 10, "Connect to WiFi Network");
  display.setFont(ArialMT_Plain_10);
  display.drawString(64, 24, HOSTNAME);
  display.setFont(ArialMT_Plain_10);
  display.drawString(64, 38, "And log into your home");
  display.drawString(64, 48, "WiFi network to setup");
  display.display();

  //add initialize statements for website and the neopixels
  parseHomePage();
  parseConfigurePage();
  initializePixels();

  WiFiManager wifiManager;

  String hostname(HOSTNAME);
  if (!wifiManager.autoConnect((const char *)hostname.c_str())) {// new addition
    delay(3000);
    WiFi.disconnect(true);
    ESP.reset();
    delay(5000);
  }

  Serial.println("WEBSERVER_ENABLED");
  //add server.on(); statements for webserver
  server.on("/Home", HTTP_GET, handleRoot);
  server.on("/Configure", handleConfigure);
  server.on("/updateConfig", handleUpdateConfigure);
  server.on("/FactoryReset", handleSystemReset);
  server.on("/WifiReset", handleWifiReset);
  server.onNotFound(handleRoot);
  server.begin();
  Serial.println("Server Started");
  String webAddress = "http://" + WiFi.localIP().toString() + ":" + String(WEBSERVER_PORT) + "/";
  Serial.println("Use this URL : " + webAddress);

  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_10);
  display.drawString(64, 10, "Web Interface On");
  display.drawString(64, 20, "You May Connect to IP");
  display.setFont(ArialMT_Plain_16);
  display.drawString(64, 30, WiFi.localIP().toString());
  display.drawString(64, 46, "Port: " + String(WEBSERVER_PORT));
  display.display();

  timeClient.begin();
}

void loop() {
  server.handleClient();

  delay(1);

  unsigned long currentMillis = millis();

  //Display on/off
  if (displayOn == true) {
    displayOn = false;
    previousMillisDisplay = currentMillis;
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_10);
    display.drawString(64, 10, "Web Interface On");
    display.drawString(64, 20, "You May Connect to IP");
    display.setFont(ArialMT_Plain_16);
    display.drawString(64, 30, WiFi.localIP().toString());
    display.drawString(64, 46, "Port: " + String(WEBSERVER_PORT));
    display.display();
  }

  if (currentMillis - previousMillisDisplay > intervalDisplay) {
    display.clear();
    display.display();
  }

  if (digitalRead(buttonPin1) == LOW) {
    displayOn = true;
  }
  //-------------------------

  //update time
  if (lastHour != timeClient.getHours() || lastMinute != timeClient.getMinutes()) {
    updateTime();
  }
  //-------------------------

  //lights
  if (digitalRead(buttonPin2) == HIGH) {
    rainbowCycle();
    firstOff = true;
  } else if (digitalRead(buttonPin2) == LOW) {
    if (firstOff == true) {
      for (int i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, 0, 0, 0);
      }
      pixels.show();
      firstOff = false;
    }
  }
  //-------------------------

  //add alarm clock functionality

  //-------------------------
}

void handleSystemReset() {
  server.requestAuthentication();
  Serial.println("Reset System Configuration");
  if (SPIFFS.remove(CONFIG)) {
    handleRoot();
    ESP.restart();
  }
}

void handleWifiReset() {
  server.requestAuthentication();
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  handleRoot();
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  ESP.restart();
}

void writeSettings() {
  // Save decoded message to SPIFFS file for playback on power up.
  File f = SPIFFS.open(CONFIG, "w");
  if (!f) {
    Serial.println("File open failed!");
  } else {
    Serial.println("Saving settings now...");
    f.println("timeZone=" + String(timeZone));
    f.println("twelveHour=" + String(twelveHour));
    f.println("alarmEnabled=" + String(alarmEnabled));
  }
  f.close();
  readSettings();
}

void readSettings() {
  if (SPIFFS.exists(CONFIG) == false) {
    Serial.println("Settings File does not yet exists.");
    writeSettings();
    return;
  }
  File fr = SPIFFS.open(CONFIG, "r");
  String line;
  while (fr.available()) {
    line = fr.readStringUntil('\n');

    if (line.indexOf("timeZone=") >= 0) {
      timeZone = line.substring(line.lastIndexOf("timeZone=") + 9).toInt();
      Serial.println("timeZone=" + String(timeZone));
    }
    if (line.indexOf("twelveHour=") >= 0) {
      twelveHour = line.substring(line.lastIndexOf("www_password=") + 11).toInt();
      Serial.println("twelveHour=" + String(twelveHour));
    }
    if (line.indexOf("alarmEnabled=") >= 0) {
      alarmEnabled = line.substring(line.lastIndexOf("alarmEnabled=") + 13).toInt();
      Serial.println("alarmEnabled: " + String(alarmEnabled));
    }
  }
  fr.close();
  timeClient.end();
  timeClient.setTimeOffset(timeZone);
  timeClient.begin();
}

void handleUpdateConfigure() { //editthis
  timeZone = server.arg("timezone").toInt();
  twelveHour = server.hasArg("twelvehour");
  alarmEnabled = server.hasArg("alarmenabled");

  writeSettings();
  handleConfigure();
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URL in the request
}

void handleRoot() {
  String form = parseHomePage();
  server.send(200, "text/html", form);  // Home webpage for the cloud
}



// converts the dBm to a range between 0 and 100%
int8_t getWifiQuality() {
  int32_t dbm = WiFi.RSSI();
  if (dbm <= -100) {
    return 0;
  } else if (dbm >= -50) {
    return 100;
  } else {
    return 2 * (dbm + 100);
  }
}

void handleConfigure() {
  String isTwelveHourChecked;
  String isAlarmEnabledChecked;

  if (twelveHour == true) {
    isTwelveHourChecked = "checked='checked'";
  }
  if (alarmEnabled == true) {
    isAlarmEnabledChecked = "checked='checked'";
  }
  String form = parseConfigurePage();
  form.replace("%TIMEZONE%", String(timeZone));
  form.replace("%TWELVEHOUR%", isTwelveHourChecked);
  form.replace("%ALARMENABLED%", isAlarmEnabledChecked);

  server.send(200, "text/html", form);  // Configure portal for the cloud
}

void initializePixels() {
  pixels.begin();
  pixels.show();
  //  pixels.setBrightness(50);
}

void rainbowCycle() {
  if (rainbowCycleLoop0 < 256) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillisLight > 1) {
      previousMillisLight = currentMillis;
      for (int i = 0; i < pixels.numPixels(); i++) {
        pixels.setPixelColor(i, Wheel(((i * 256 / pixels.numPixels()) + rainbowCycleLoop0) & 255));
      }
      pixels.show();
    }
    rainbowCycleLoop0++;
  } else if (rainbowCycleLoop0 >= 256) {
    rainbowCycleLoop0 = 0;
  }
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void updateTime () {
  byte currentSegmentOn[4];
  byte currentSegmentOff[4];
  int hour0 = (timeClient.getHours() / 10);
  int hour1 = (timeClient.getHours() % 10);
  int minute0 = (timeClient.getMinutes() / 10);
  int minute1 = (timeClient.getMinutes() % 10);

  digitalWrite(LATCH, LOW);

  if (timeClient.getHours() != lastHour) {
    lastHour = timeClient.getHours();
    shiftOut(DATA, CLK, LSBFIRST, segmentOn[hour1]);
    shiftOut(DATA, CLK, LSBFIRST, segmentOff[hour1]);
    shiftOut(DATA, CLK, LSBFIRST, segmentOn[hour0]);
    shiftOut(DATA, CLK, LSBFIRST, segmentOff[hour0]);
  } else {
    shiftOut(DATA, CLK, LSBFIRST, segmentOn[10]);
    shiftOut(DATA, CLK, LSBFIRST, segmentOn[10]);
    shiftOut(DATA, CLK, LSBFIRST, segmentOn[10]);
    shiftOut(DATA, CLK, LSBFIRST, segmentOn[10]);
  }

  if (timeClient.getMinutes() != lastMinute) {
    lastMinute = timeClient.getMinutes();
    shiftOut(DATA, CLK, LSBFIRST, segmentOn[minute1]);
    shiftOut(DATA, CLK, LSBFIRST, segmentOff[minute1]);
    shiftOut(DATA, CLK, LSBFIRST, segmentOn[minute0]);
    shiftOut(DATA, CLK, LSBFIRST, segmentOff[minute0]);
  } else {
    shiftOut(DATA, CLK, LSBFIRST, segmentOn[10]);
    shiftOut(DATA, CLK, LSBFIRST, segmentOn[10]);
    shiftOut(DATA, CLK, LSBFIRST, segmentOn[10]);
    shiftOut(DATA, CLK, LSBFIRST, segmentOn[10]);
  }

  digitalWrite(LATCH, HIGH);
}

//void alarmFunction 
