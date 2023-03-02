#ifndef LOGGER_H
#define LOGGER_H

// SD CARD
#include "FS.h"
#include "SD.h"
#include "SPI.h"

// GPS
#include <TinyGPS.h>

class Logger
{
public:
    void log(float lat, float lon, float alt, float kmh, float course, unsigned short sats, float temp);
};

#endif