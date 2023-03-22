#ifndef GPSLOG_H
#define GPSLOG_H

#include <TinyGPS.h>
#include "LogEntry.h"

class GPSLog
{
public:
    LogEntry createLogEntry(TinyGPS gps);
};

#endif