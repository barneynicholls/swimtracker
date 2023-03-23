#ifndef GPSLOG_H
#define GPSLOG_H

#include <TinyGPS.h>
#include "LogEntry.h"

class SwimLogger
{
public:
    LogEntry createLogEntry(TinyGPS gps, float temperature);
};

#endif