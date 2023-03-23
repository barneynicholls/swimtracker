#include "SwimLogger.h"
#include "Logger.h"
#include "LogEntry.h"
#include "FS.h"

#include <TinyGPS.h>

Logger logger;

void SwimLogger::begin()
{
    logger.begin();
}

void SwimLogger::deleteLog()
{
    logger.deleteLog();
}

File SwimLogger::getLog()
{
    return logger.getLog();
}

LogEntry SwimLogger::createLogEntry(TinyGPS gps, float temperature)
{
    float lat, lon, alt, speed, course;
    unsigned long age;
    unsigned short sats;
    int year;
    byte month, day, hour, minute, second, hundredths;

    sats = gps.satellites();

    gps.f_get_position(&lat, &lon, &age);
    alt = gps.f_altitude();
    speed = gps.f_speed_kmph();
    course = gps.f_course();

    gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);

    String cardinal = course == TinyGPS::GPS_INVALID_F_ANGLE ? "" : TinyGPS::cardinal(course);

    String dateTime, time;

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
            lat,
            lon,
            alt,
            speed,
            course,
            cardinal,
            sats,
            time,
            temperature,
            age};

    logger.log(logEntry);

    return logEntry;
}