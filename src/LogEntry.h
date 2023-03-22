#ifndef LogEntry_h
#define LogEntry_h

#include "Arduino.h" //needed to include standard arduino library

struct LogEntry
{
  String dateTime;
  float latitude;
  float longitude;
  float altitude;
  float speed;
  float course;
  String cardinal;
  unsigned short satellites;
  String time;
  float temperature;
};

#endif