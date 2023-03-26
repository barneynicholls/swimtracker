#ifndef GPSLOG_H
#define GPSLOG_H

#include <TinyGPS.h>
#include "LogEntry.h"
#include "FS.h"

class SwimLogger
{
public:
    LogEntry createLogEntry(TinyGPS gps, float temperature, bool recording);
    void deleteLog();
    File getLog();
    void begin();
};

#endif