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

// DISPLAY
#include <SwimDisplay.h>
SwimDisplay display;

// DS18B20 temperature sensor
#include <OneWire.h>
#include <DallasTemperature.h>

// Swim Tracker
#include <SwimLogger.h>
SwimLogger swimLog;

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
  // SD.remove("/data.txt");
  swimLog.deleteLog();
  server.send(200, "text/plain", "Data File Deleted");
}

void getData()
{
  File dataFile = swimLog.getLog();

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
  display.begin();

  // Start the log
  swimLog.begin();

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
  toggled = !toggled;

  bool wifiConnected;

  wifiConnected = WiFi.status() == WL_CONNECTED;

  if (wifiConnected)
  {
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    server.handleClient();
  }

  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);

  LogEntry entry = swimLog.createLogEntry(gps, temperatureC);

  display.update(entry, wifiConnected, toggled);

  char test[150];

  sprintf(test, "DATE:%s LAT:%.3f LON:%.3f ALT:%.3f AGE:%lu",
          entry.dateTime.c_str(),
          entry.latitude,
          entry.longitude,
          entry.altitude,
          entry.age);

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