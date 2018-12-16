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

#define VERSION "0.1a"
#define HOSTNAME "IOT-ALARM-CLOCK"
#define CONFIG "/conf.txt"
#define WEBSERVER_PORT 80
#define NUMPIXELS 300 //change to however many are used

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

float timeZone = 0.0;
int externalLight = LED_BUILTIN;
long previousMillisDisplay = 0;
long intervalDisplay = 10000;
boolean displayOn = true;
boolean 12Hour = true;
const boolean INVERT_DISPLAY = true; // true = pins at top | false = pins at the bottom

const int externalLight = LED_BUILTIN;
const int buttonPin = D7;
const int I2C_DISPLAY_ADDRESS = 0x3c; // I2C Address of your Display (usually 0x3c or 0x3d)
const int SDA_PIN = D2;
const int SCL_PIN = D5;
const int neopixelPin =  D4;

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
  pinMode(externalLight, OUTPUT);
  pinMode(buttonPin, INPUT);

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

  if (digitalRead(buttonPin) == LOW) {
    displayOn = true;
  }

  if (lastHour != timeClient.getHours() || lastMinute != timeClient.getMinutes()) {
    updateTime();
  }

  rainbowCycle();
}

void handleSystemReset() {
  if (!server.authenticate(www_username, www_password)) {
    return server.requestAuthentication();
  }
  Serial.println("Reset System Configuration");
  if (SPIFFS.remove(CONFIG)) {
    handleRoot();;
    ESP.restart();
  }
}

void handleWifiReset() {
  if (!server.authenticate(www_username, www_password)) {
    return server.requestAuthentication();
  }
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  handleRoot();
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  ESP.restart();
}

void writeSettings() { //editthis
  // Save decoded message to SPIFFS file for playback on power up.
  File f = SPIFFS.open(CONFIG, "w");
  if (!f) {
    Serial.println("File open failed!");
  } else {
    Serial.println("Saving settings now...");
    f.println("www_username=" + String(www_username));
    f.println("www_password=" + String(www_password));
    f.println("weatherKey=" + WeatherApiKey);
    f.println("CityID=" + String(CityID));
    f.println("isMetric=" + String(IS_METRIC));
  }
  f.close();
  readSettings();
}

void readSettings() { //editthis
  if (SPIFFS.exists(CONFIG) == false) {
    Serial.println("Settings File does not yet exists.");
    writeSettings();
    return;
  }
  File fr = SPIFFS.open(CONFIG, "r");
  String line;
  while (fr.available()) {
    line = fr.readStringUntil('\n');

    if (line.indexOf("www_username=") >= 0) {
      String temp = line.substring(line.lastIndexOf("www_username=") + 13);
      temp.trim();
      temp.toCharArray(www_username, sizeof(temp));
      Serial.println("www_username=" + String(www_username));
    }
    if (line.indexOf("www_password=") >= 0) {
      String temp = line.substring(line.lastIndexOf("www_password=") + 13);
      temp.trim();
      temp.toCharArray(www_password, sizeof(temp));
      Serial.println("www_password=" + String(www_password));
    }
    if (line.indexOf("weatherKey=") >= 0) {
      WeatherApiKey = line.substring(line.lastIndexOf("weatherKey=") + 11);
      WeatherApiKey.trim();
      Serial.println("WeatherApiKey=" + WeatherApiKey);
    }
    if (line.indexOf("CityID=") >= 0) {
      CityID = line.substring(line.lastIndexOf("CityID=") + 7).toInt();
      Serial.println("CityID: " + String(CityID));
    }
    if (line.indexOf("isMetric=") >= 0) {
      IS_METRIC = line.substring(line.lastIndexOf("isMetric=") + 9).toInt();
      Serial.println("IS_METRIC=" + String(IS_METRIC));
    }
  }
  fr.close();
  weatherClient.updateWeatherApiKey(WeatherApiKey);
  weatherClient.updateCityId(CityID);
  weatherClient.setMetric(IS_METRIC);
  weatherClient.updateCityId(CityID);
}

void handleUpdateConfigure() { //editthis
  if (!server.authenticate(www_username, www_password)) {
    return server.requestAuthentication();
  }

  String temp = server.arg("userid");
  temp.toCharArray(www_username, sizeof(temp));
  temp = server.arg("stationpassword");
  temp.toCharArray(www_password, sizeof(temp));
  OTA_Password = server.arg("otapassword");
  WeatherApiKey = server.arg("openWeatherMapApiKey");
  CityID = server.arg("city1").toInt();
  IS_METRIC = server.hasArg("metric");

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

void handleConfigure() { //editthis
  String form = parseConfigurePage();
  form.replace("%USERID%", www_username);
  form.replace("%STATIONPASSWORD%", www_password);
  form.replace("%OTAPASSWORD%", OTA_Password);
  form.replace("%WEATHERKEY%", WeatherApiKey);
  form.replace("%CITY%", String(CityID));

  Serial.println(String(CityID));
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
    if (currentMillis - previousMillisArray[0] > 1) {
      previousMillisArray[0] = currentMillis;
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
  if () {
    
  }
  if () {
    
  }
}
