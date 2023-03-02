#include "Logger.h"

void Logger::log(float lat, float lon, float alt, float kmh, float course, unsigned short sats, float temp)
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
            "%.4f,%.4f,%.2f,%.2f,%.3f,%s,%i,%.2f",
            lat, lon, alt, kmh, course, TinyGPS::cardinal(course), sats, temp);

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
