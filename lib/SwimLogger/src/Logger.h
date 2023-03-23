#ifndef LOGGER_H
#define LOGGER_H

// SD CARD
#include "FS.h"
#include "SD.h"
#include "SPI.h"

// GPS
#include <TinyGPS.h>

#include "LogEntry.h"

class Logger
{
public:
    void log(LogEntry entry);
};

#endif