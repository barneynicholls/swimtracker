#include "secrets.h"

#include <SPI.h>
#include <Wire.h>

// GPS
#include <SoftwareSerial.h>
#include <TinyGPS.h>

TinyGPS gps;
SoftwareSerial ss(25, 26);

// LEDS
const int LED_BUILTIN = 2;
const int LED_RECORDING = 32;

// BUTTON
const int BUTTON_REC = 35;

// DISPLAY
#include <U8g2lib.h>
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0);

// WIFI
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <basicOTA.h>

const char *ssid = SECRET_SSID;
const char *password = SECRET_PASS;
WebServer server(80);

// SD CARD
#include "FS.h"
#include "SD.h"
#include "SPI.h"

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
bool recording = false;
float temperature;

void deleteData()
{
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

void handleNotFound()
{
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

  // // Start the log
  swimLog.begin();

  // LEDS
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(LED_RECORDING, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // BUTTONS
  pinMode(BUTTON_REC, INPUT);

  // DISPLAY
  u8g2.begin();

  // wifi
  WiFi.begin(ssid, password);

  SD.begin();

  // web server
  server.serveStatic("/", SD, "/index.html");
  server.serveStatic("/styles.css", SD, "/styles.css");
  server.serveStatic("/config.js", SD, "/config.js");
  server.serveStatic("/main.js", SD, "/main.js");

  server.on("/data", HTTP_GET, getData);
  server.on("/delete", HTTP_GET, deleteData);

  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");

  // Setup Firmware update over the air (OTA)
  setup_OTA();
}

void loop(void)
{
  sensors.requestTemperatures();
  float tempReading = sensors.getTempCByIndex(0);

  LogEntry entry = swimLog.createLogEntry(gps, tempReading, recording);

  // read the state of the pushbutton value:
  int buttonState = digitalRead(BUTTON_REC);

  if (buttonState == HIGH)
  {
    recording = !recording;
    digitalWrite(LED_RECORDING, recording ? HIGH : LOW);
  }

  bool wifiConnected = WiFi.status() == WL_CONNECTED;

  if (wifiConnected)
  {
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    server.handleClient();
    // Check for OTA updates
    ArduinoOTA.handle();
  }

  u8g2.firstPage();
  do
  {
    u8g2.setFont(u8g2_font_crox4h_tf);
    String status = wifiConnected ? WiFi.localIP().toString() : "- - -.- - -.- - -.- - -";
    u8g2.drawStr(0, 40, status.c_str());
  } while (u8g2.nextPage());

  char test[200];

  sprintf(test, "DATE:%s LAT:%.3f LON:%.3f ALT:%.3f AGE:%lu, REC:%i",
          entry.dateTime.c_str(),
          entry.latitude,
          entry.longitude,
          entry.altitude,
          entry.age,
          entry.recording ? 1 : 0);

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