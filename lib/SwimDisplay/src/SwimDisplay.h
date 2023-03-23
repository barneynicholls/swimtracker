#ifndef SWIMDISPLAY_H
#define SWIMDISPLAY_H

// GPS
#include <TinyGPS.h>

#include "LogEntry.h"

class SwimDisplay
{
public:
    void update(LogEntry entry);
};

#endif