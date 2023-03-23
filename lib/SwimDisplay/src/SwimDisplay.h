#ifndef SWIMDISPLAY_H
#define SWIMDISPLAY_H

// GPS
#include <TinyGPS.h>

#include "LogEntry.h"

class SwimDisplay
{
public:
    void begin();
    void update(LogEntry entry, bool wifiConnected, bool toggle);
};

#endif