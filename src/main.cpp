// GPS
#include <SoftwareSerial.h>
#include <TinyGPS.h>

TinyGPS gps;
SoftwareSerial ss(26, 25);

// WIFI
const int LED_BUILTIN = 2;
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
const char *ssid = "Seareach";
const char *password = "seareach";
WebServer server(80);

// SD CARD
#include "FS.h"
#include "SD.h"
#include "SPI.h"

// 128x32 OLED
#include <U8g2lib.h>
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0);

// DS18B20 temperature sensor
#include <OneWire.h>
#include <DallasTemperature.h>

// Swim Tracker
#include "Logger.h"
#include "LogEntry.h"
#include "GPSLog.h"
Logger logger;
GPSLog gpsLog;

// GPIO where the DS18B20 is connected to
const int oneWireBus = 27;

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature sensors(&oneWire);

// globals
bool toggled = false;

void deleteData()
{
  SD.remove("/data.txt");
  server.send(200, "text/plain", "Data File Deleted");
}

void getData()
{
  File dataFile = SD.open("/data.txt");

  if (!dataFile)
  {
    return;
  }
  server.sendHeader("Content-Disposition", "attachment;filename=swim_track_exportedData.csv");
  if (server.streamFile(dataFile, "text/plain") != dataFile.size())
  {
    Serial.println("Sent less data than expected!");
  }

  dataFile.close();
}

static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

void handleRoot()
{
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);
  char html[1000];

  sprintf(
      html,
      "<html><head><link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.2.3/dist/css/bootstrap.min.css\" integrity=\"sha384-rbsA2VBKQhggwzxH7pPCaAqO46MgnOM80zW1RWuH61DGLwZJEdK2Kadq2F9CUG65\" crossorigin=\"anonymous\"><title>Swim Tracker</title><meta http-equiv=\"refresh\" content=\"10\"></head><body><h1>Swim Tracker</h1><h2>Current</h2><p>Temperature: %.1fc</p><h2>Archive</h2><p><a href='/data'>Retrieve Data</a></p><p><a href='/delete'>Delete Data</a></p></body></html>",
      temperatureC);

  server.send(200, "text/html", html);
}

void handleNotFound()
{
  digitalWrite(LED_BUILTIN, HIGH);
  String message = "File Not Found";
  message += "URI: ";
  message += server.uri();
  message += "nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "nArguments: ";
  message += server.args();
  message += "n";
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(LED_BUILTIN, LOW);
}

// Setup
void setup(void)
{
  // Start the serial for debug
  Serial.begin(115200);

  // Start GPS serial
  ss.begin(9600);

  // Start the DS18B20 sensor
  sensors.begin();

  // Start the OLED display
  u8g2.begin();

  // Start SD
  SD.begin();

  // wifi
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  WiFi.begin(ssid, password);

  // web server
  server.on("/data", HTTP_GET, getData);
  server.on("/delete", HTTP_GET, deleteData);
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void)
{
  bool showGPS, showWifi, wifiConnected;

  toggled = !toggled;
  wifiConnected = WiFi.status() == WL_CONNECTED;
  showWifi = toggled || wifiConnected;

  if (wifiConnected)
  {
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    server.handleClient();
  }

  LogEntry entry = gpsLog.createLogEntry(gps);

  showGPS = toggled || entry.satellites != TinyGPS::GPS_INVALID_SATELLITES;

  String lat = entry.latitude == TinyGPS::GPS_INVALID_F_ANGLE ? "-" : String(entry.latitude, 3);
  String lon = entry.longitude == TinyGPS::GPS_INVALID_F_ANGLE ? "-" : String(entry.longitude, 3);
  String alt = entry.altitude == TinyGPS::GPS_INVALID_F_ALTITUDE ? "-" : String(entry.altitude, 1);
  String speed = entry.speed == TinyGPS::GPS_INVALID_F_SPEED ? "-" : String(entry.speed, 1);
  String course = String(entry.cardinal);
  String sats = entry.satellites == TinyGPS::GPS_INVALID_SATELLITES ? "-" : String(entry.satellites);

  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);
  bool invalidTemp = temperatureC < -50 || temperatureC > 100;

  String temp = invalidTemp ? "--.-" : String(temperatureC, 1);

  entry.temperature = temperatureC;
  logger.log(entry);

  u8g2.firstPage();
  do
  {

    u8g2.setFont(u8g2_font_logisoso34_tf);
    u8g2.drawStr(0, 62, temp.c_str());

    u8g2.setFont(u8g2_font_tom_thumb_4x6_mr);
    u8g2.drawStr(0, 24, "TEMPERATURE C");

    if (toggled)
    {
      u8g2.drawStr(94, 24, "LAT:");
      u8g2.drawStr(94, 30, lat.c_str());
      u8g2.drawStr(94, 40, "LON:");
      u8g2.drawStr(94, 46, lon.c_str());
      u8g2.drawStr(94, 56, "ALT:");
      u8g2.drawStr(94, 64, alt.c_str());
    }
    else
    {
      u8g2.drawStr(94, 24, "KMH:");
      u8g2.drawStr(94, 30, speed.c_str());
      u8g2.drawStr(94, 40, "COURSE:");
      u8g2.drawStr(94, 46, course.c_str());
      u8g2.drawStr(94, 56, "SATS:");
      u8g2.drawStr(94, 64, sats.c_str());
    }

    // TIME u8g2_font_profont22_mf
    u8g2.setFont(u8g2_font_t0_18_mf);
    u8g2.drawStr(0, 13, entry.time.c_str());

    // ICONS
    u8g2.setFont(u8g2_font_open_iconic_www_2x_t);
    // GPPS-ARROW
    u8g2.drawStr(94, 16, showGPS ? "F" : "");
    // WIFI
    u8g2.drawStr(112, 16, showWifi ? "Q" : "");

  } while (u8g2.nextPage());

  
  char test[150];

  sprintf(test, "DATE:%s LAT:%.3f LON:%.3f ALT:%.3f",
          entry.dateTime.c_str(),
          entry.latitude,
          entry.longitude,
          entry.altitude);

  Serial.println(test);

  sprintf(test, "COURSE:%.3f CARDINAL:%s SATS:%d TIME:%s TEMP:%.2f",
          entry.course,
          entry.cardinal.c_str(),
          entry.satellites,
          entry.time.c_str(),
          entry.temperature);

  Serial.println(test);

  smartdelay(2000);
}