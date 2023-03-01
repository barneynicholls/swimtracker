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

// GPIO where the DS18B20 is connected to
const int oneWireBus = 27;

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature sensors(&oneWire);

// globals
bool toggled = false;

void logData(float lat, float lon)
{

  File file = SD.open("/data.txt", FILE_APPEND);
  if (!file)
  {
    Serial.println("Failed to open file for appending");
    return;
  }

  String line = String(lat, 3);
  line.concat(",");
  line.concat(String(lon, 3).c_str());

  if (file.println(line))
  {
    Serial.println("Message appended");
  }
  else
  {
    Serial.println("Append failed");
  }
  file.close();
}

void getData()
{
  File dataFile = SD.open("/data.txt");

  if (!dataFile)
  {
    return;
  }

  if (server.streamFile(dataFile, "text/plain") != dataFile.size())
  {
    Serial.println("Sent less data than expected!");
  }

  dataFile.close();
}

static String getTime(bool hasGPS)
{
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);

  if (!hasGPS || age == TinyGPS::GPS_INVALID_AGE)
  {
    return toggled ? "--:--" : "00:00";
  }

  char display[5];

  sprintf(display, "%02d:%02d", hour, minute);

  return String(display);
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
  bool invalidTemp = temperatureC < -50 || temperatureC > 100;

  String temp = invalidTemp ? "--.-" : String(temperatureC, 1);
  temp.concat("c");
  digitalWrite(LED_BUILTIN, HIGH);
  server.send(200, "text/plain", temp);
  digitalWrite(LED_BUILTIN, LOW);
}

void handleNotFound()
{
  digitalWrite(LED_BUILTIN, HIGH);
  String message = "File Not Foundnn";
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
  server.on("/", handleRoot);
  server.on("/inline", []()
            { server.send(200, "text/plain", "this works as well"); });
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void)
{
  float flat, flon, falt, fspeed, fcourse;
  unsigned long posAge;
  unsigned short uSats;
  bool showGPS, showWifi;

  toggled = !toggled;

  uSats = gps.satellites();

  showGPS = toggled || uSats != TinyGPS::GPS_INVALID_SATELLITES;
  showWifi = toggled || WiFi.status() == WL_CONNECTED;

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    server.handleClient();
  }

  gps.f_get_position(&flat, &flon, &posAge);
  falt = gps.f_altitude();
  fspeed = gps.f_speed_kmph();
  fcourse = gps.f_course();

  String lat = flat == TinyGPS::GPS_INVALID_F_ANGLE ? "-" : String(flat, 3);
  String lon = flon == TinyGPS::GPS_INVALID_F_ANGLE ? "-" : String(flon, 3);
  String alt = falt == TinyGPS::GPS_INVALID_F_ALTITUDE ? "-" : String(falt, 1);
  String speed = fspeed == TinyGPS::GPS_INVALID_F_SPEED ? "-" : String(fspeed, 1);
  String course = fcourse == TinyGPS::GPS_INVALID_F_ANGLE ? "-" : TinyGPS::cardinal(fcourse);
  String sats = uSats == TinyGPS::GPS_INVALID_SATELLITES ? "-" : String(uSats);

  String time = getTime(showGPS);

  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);
  bool invalidTemp = temperatureC < -50 || temperatureC > 100;

  String temp = invalidTemp ? "--.-" : String(temperatureC, 1);

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
    u8g2.drawStr(0, 13, time.c_str());

    // ICONS
    u8g2.setFont(u8g2_font_open_iconic_www_2x_t);
    // GPPS-ARROW
    u8g2.drawStr(94, 16, showGPS ? "F" : "");
    // WIFI
    u8g2.drawStr(112, 16, showWifi ? "Q" : "");

  } while (u8g2.nextPage());

  logData(flat, flon);

  smartdelay(2000);
}