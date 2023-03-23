#include "SwimDisplay.h"
#include "LogEntry.h"

// 128x32 OLED
#include <U8g2lib.h>
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0);

// GPS
#include <TinyGPS.h>

void SwimDisplay::begin()
{
    // Start the OLED display
    u8g2.begin();
}

void SwimDisplay::update(LogEntry entry, bool wifiConnected, bool toggle)
{
    String lat = entry.latitude == TinyGPS::GPS_INVALID_F_ANGLE ? "-" : String(entry.latitude, 3);
    String lon = entry.longitude == TinyGPS::GPS_INVALID_F_ANGLE ? "-" : String(entry.longitude, 3);
    String alt = entry.altitude == TinyGPS::GPS_INVALID_F_ALTITUDE ? "-" : String(entry.altitude, 1);
    String speed = entry.speed == TinyGPS::GPS_INVALID_F_SPEED ? "-" : String(entry.speed, 1);
    String course = String(entry.cardinal);
    String sats = entry.satellites == TinyGPS::GPS_INVALID_SATELLITES ? "-" : String(entry.satellites);

    bool invalidTemp = entry.temperature < -50 || entry.temperature > 100;

    String temp = invalidTemp ? "--.-" : String(entry.temperature, 1);

    u8g2.firstPage();
    do
    {

        u8g2.setFont(u8g2_font_logisoso34_tf);
        u8g2.drawStr(0, 62, temp.c_str());

        u8g2.setFont(u8g2_font_tom_thumb_4x6_mr);
        u8g2.drawStr(0, 24, "TEMPERATURE C");

        if (toggle)
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
        bool showGPS = toggle || entry.satellites != TinyGPS::GPS_INVALID_SATELLITES;
        u8g2.drawStr(94, 16, showGPS ? "F" : "");
        // WIFI
        u8g2.drawStr(112, 16, wifiConnected ? "Q" : "");

    } while (u8g2.nextPage());
}
