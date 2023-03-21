#ifndef LogEntry_h
#define LogEntry_h

#include "Arduino.h" //needed to include standard arduino library

struct LogEntry
{
  float latitude;
  float longitude;
  float altitude;
  float course;
  unsigned short satellites;
};

#endif