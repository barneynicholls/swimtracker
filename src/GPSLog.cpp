#include "GPSLog.h"
#include "Logger.h"
#include "LogEntry.h"

#include <TinyGPS.h>

LogEntry GPSLog::createLogEntry(TinyGPS gps)
{
    float flat, flon, falt, fspeed, fcourse;
    unsigned long posAge;
    unsigned short uSats;
    int year;
    byte month, day, hour, minute, second, hundredths;
    unsigned long age;

    uSats = gps.satellites();

    gps.f_get_position(&flat, &flon, &posAge);
    falt = gps.f_altitude();
    fspeed = gps.f_speed_kmph();
    fcourse = gps.f_course();

    gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);

    String cardinal = fcourse == TinyGPS::GPS_INVALID_F_ANGLE ? "" : TinyGPS::cardinal(fcourse);

    String dateTime,time;

    if (age == TinyGPS::GPS_INVALID_AGE)
    {
        dateTime = "0000-00-00T00:00:00";
        time = "00:00";
    }
    else
    {
        char dateTimeBuf[20];
        sprintf(dateTimeBuf, "%04d-%02d-%02dT%02d:%02d:%02d", year, month, day, hour, minute, second);
        dateTime = String(dateTimeBuf);
        char timeBuf[10];
        sprintf(timeBuf, "%02d:%02d", hour, minute);
        time = String(timeBuf);
    }

    LogEntry logEntry =
        {
            dateTime,
            flat,
            flon,
            falt,
            fspeed,
            fcourse,
            cardinal,
            uSats,
            time,
            0};

    return logEntry;
}