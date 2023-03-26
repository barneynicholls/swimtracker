#include "Logger.h"
#include "LogEntry.h"

// SD CARD
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include <TinyGPS.h>

const char logFile[] = "/data.txt";

void Logger::begin()
{
    // Start SD
    SD.begin();
}

void Logger::deleteLog()
{
    SD.remove(logFile);
}

File Logger::getLog()
{
    return SD.open(logFile);
}

void Logger::log(LogEntry entry)
{
    File file = SD.open(logFile, FILE_APPEND);
    if (!file)
    {
        Serial.println("Failed to open file for appending");
        return;
    }

    char line[100];

    sprintf(line,
            "%s,%.4f,%.4f,%.2f,%.2f,%.3f,%s,%i,%.2f,%i",
            entry.dateTime.c_str(), entry.latitude, entry.longitude, entry.altitude, entry.speed,
            entry.course, entry.cardinal.c_str(), entry.satellites, entry.temperature,
            entry.recording ? 1 : 0);

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
