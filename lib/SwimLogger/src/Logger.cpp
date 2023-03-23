#include "Logger.h"
#include "LogEntry.h"

#include <TinyGPS.h>

void Logger::log(LogEntry entry)
{
    // TODO file name should be passed to constructor
    File file = SD.open("/data.txt", FILE_APPEND);
    if (!file)
    {
        Serial.println("Failed to open file for appending");
        return;
    }

    char line[100];

    sprintf(line,
            "%s,%.4f,%.4f,%.2f,%.2f,%.3f,%s,%i,%.2f",
            entry.dateTime.c_str(), entry.latitude, entry.longitude, entry.altitude, entry.speed,
            entry.course, entry.cardinal.c_str(), entry.satellites, entry.temperature);

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
